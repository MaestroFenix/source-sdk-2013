//==========================================================================//
//
// Purpose: New playermodel panel created by Maestro Fenix 2014
//
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include "NewPlayermodelPanel.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/Frame.h>


#include <vgui_controls/Button.h>
#include <vgui_controls\TextEntry.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>

#include <KeyValues.h>

#include "Filesystem.h"

CUtlVector<char*> playervector;


class CMyPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CMyPanel, vgui::Frame);

	CMyPanel(vgui::VPANEL parent); 	// Constructor
	~CMyPanel(){};				// Destructor

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
	virtual void OnTextChanged(KeyValues *data);
	virtual void RecursiveFindFiles(const char *pWildcard, char const *current);

private:
	//Other used VGUI control Elements:


	Button *m_pCloseButton;
	Button *m_pSelectButton;
	TextEntry *m_pPlayerImageSpace;
	ComboBox *m_pPlayerSelect;
	ImagePanel *m_pPlayerImage;
};

// Constuctor: Initializes the Panel
CMyPanel::CMyPanel(vgui::VPANEL parent)
	: BaseClass(NULL, "MyPanel")
{
	SetParent(parent);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	SetProportional(false);
	SetTitleBarVisible(true);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(true);
	SetVisible(true);


	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("resource/UI/playermodelsel.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

	DevMsg("MyPanel has been constructed\n");


	//Button done
	m_pCloseButton = new Button(this, "Button", "Close", this, "turnoff");
	m_pCloseButton->SetPos(433, 472);
	m_pCloseButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pCloseButton->SetReleasedSound("ui/buttonclick.wav");

	//Button select
	m_pSelectButton = new Button(this, "Button", "Select", this, "select");
	m_pSelectButton->SetPos(350, 472);
	m_pSelectButton->SetDepressedSound("common/bugreporter_succeeded.wav");
	m_pSelectButton->SetReleasedSound("ui/buttonclick.wav");

	//Playermodel preview
	m_pPlayerImageSpace = new TextEntry(this, "playermodeliconspace");
	m_pPlayerImageSpace->SetPos(139, 49);
	m_pPlayerImageSpace->SetWide(256);
	m_pPlayerImageSpace->SetTall(256);
	m_pPlayerImageSpace->SetEditable(0);
	
	//Playermodel icon
	m_pPlayerImage = new ImagePanel(this,"playermodelicon");
	m_pPlayerImage->SetPos(139, 49);
	m_pPlayerImage->SetWide(256);
	m_pPlayerImage->SetTall(256);
	m_pPlayerImage->SetImage(scheme()->GetImage("logos/spray_lambda", false));

	//Combobox for select playermodel
	m_pPlayerSelect = new ComboBox(this, "ComboBox1", 10, false);
	m_pPlayerSelect->SetPos(146, 348);
	m_pPlayerSelect->SetWide(256);
	m_pPlayerSelect->SetTall(24);


	//Go find our precious playermodels
	RecursiveFindFiles("add-ons","add-ons");
}

//Class: CMyPanelInterface Class. Used for construction.
class CMyPanelInterface : public NewPlayermodelPanel
{
private:
	CMyPanel *MyPanel;
public:
	CMyPanelInterface()
	{
		MyPanel = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		MyPanel = new CMyPanel(parent);
	}
	void Destroy()
	{
		if (MyPanel)
		{
			MyPanel->SetParent((vgui::Panel *)NULL);
			delete MyPanel;
		}
	}
	void Activate(void)
	{
		if (MyPanel)
		{
			MyPanel->Activate();
		}
	}
};
static CMyPanelInterface g_MyPanel;
NewPlayermodelPanel* mypanel = (NewPlayermodelPanel*)&g_MyPanel;

ConVar cl_showmypanel("cl_showmypanel", "0", FCVAR_CLIENTDLL, "Sets the state of myPanel <state>");


void CMyPanel::OnTick()
{
	BaseClass::OnTick();
	SetVisible(cl_showmypanel.GetBool());
}


CON_COMMAND(OpenNewPlayermodelPanel, "Toggles NewPlayermodelPanel on or off")
{
	cl_showmypanel.SetValue(!cl_showmypanel.GetBool());
	mypanel->Activate();
};

void CMyPanel::OnCommand(const char* pcCommand)
{
	BaseClass::OnCommand(pcCommand);

	//Closes panel
	if (!Q_stricmp(pcCommand, "turnoff"))
		cl_showmypanel.SetValue(0);

	//Sets playermodel
	if (!Q_stricmp(pcCommand, "select"))
	{
		//Gets selected item at the combobox
		const char *playermodeldir2 = m_pPlayerSelect->GetActiveItemUserData()->GetName(); //Get filename

		const char *playermodeldirN = m_pPlayerSelect->GetActiveItemUserData()->GetString(playermodeldir2); //Get path of the file

		char playermodeldir1[512] = "";
		char *playermodeldir3 = ".txt";

		//Puts all together the playermodel txt path  
		V_strcpy(playermodeldir1, playermodeldirN);

		V_strcat(playermodeldir1, "/", sizeof(playermodeldir1)); //We have to add an slash to the path since it doesnt include it
		V_strcat(playermodeldir1, playermodeldir2, sizeof(playermodeldir1));
		V_strcat(playermodeldir1, playermodeldir3, sizeof(playermodeldir1));

		//Opens and reads the txt file
		FileHandle_t fh = filesystem->Open(playermodeldir1, "r", "GAME"); //GAME

		if (fh)
		{
			int file_len = filesystem->Size(fh);
			char* TextInfo = new char[file_len + 1];

			filesystem->Read((void*)TextInfo, file_len, fh);

			//Ends reading it (null terminator)
			TextInfo[file_len] = 0;  
			
			//Close the file after reading it
			filesystem->Close(fh);

			//Splits the text every time an " is found after being got from the text
			V_SplitString(TextInfo, "\"", playervector);
			
			//Fixes slashes so the icon can be found
			V_FixSlashes(playervector[1]);

			//Set selected icon
			m_pPlayerImage->SetImage(scheme()->GetImage(playervector[1], false));

			//Put all together the command name and the path to the playermodel
			char szReturnString[512];
			Q_snprintf(szReturnString, sizeof(szReturnString), "cl_playermodel %s%s\n", playervector[3], ".mdl");

			//Send command
			engine->ClientCmd(szReturnString);

			//Purge all the elements after being used
			playervector.PurgeAndDeleteElements();
			delete[] TextInfo;
		}
	}
}

//ToDo: switch to display the playermodel icon on changed option instead of pressing Select
void CMyPanel::OnTextChanged(KeyValues *data)
{
	
}

//-----------------------------------------------------------------------------
// Purpose: Moves from an indicated path, and keeps searching files
// Input  : origin folder, current folder
//-----------------------------------------------------------------------------
void CMyPanel::RecursiveFindFiles(const char *pWildcard, char const *current)
{
	
	char path[512];
	if (current[0])
	{
		Q_snprintf(path, sizeof(path), "%s/*.*", current);
	}
	else
	{
		Q_snprintf(path, sizeof(path), "*.*");
	}

	Q_FixSlashes(path);


	FileFindHandle_t findHandle;
	const char *pFilename = filesystem->FindFirstEx(path, "GAME", &findHandle);

	char pFilenameNoExt[32];


	while (pFilename)
	{
		//Loads VPK addons if it finds one     MOVED to cdll_client_int.cpp and gameinterface.cpp. Delete if is not possible to load the files while the mod is working
		/*if (V_stristr(pFilename, ".vpk"))
		{
			
			char playermodeldir1[512] = "";

			V_strcpy(playermodeldir1, current);
			V_strcat(playermodeldir1, "/", sizeof(playermodeldir1)); //We have to add an slash to the path since it doesnt include it
			V_strcat(playermodeldir1, pFilename, sizeof(playermodeldir1));

			Msg("Found '%s'\n", playermodeldir1);


			filesystem->AddSearchPath(VarArgs("%s/%s", engine->GetGameDirectory(), playermodeldir1), "MOD");

			Msg("Mounted '%s/%s'\n", engine->GetGameDirectory(), playermodeldir1);
			char buf2[1024];
			filesystem->GetSearchPath("MOD", true, buf2, sizeof(buf2) - 1);
			DevMsg("New Search Paths: %s\n", buf2);

		}*/

		if (filesystem->FindIsDirectory(findHandle)){

			//Get out . and .. directories
			if ((Q_stricmp(pFilename, ".")) && (Q_stricmp(pFilename, "..")) )
			{
				//Adds the current dir, so it adds to the path
				char nextdir[512];
				if (current[0])
				{
					Q_snprintf(nextdir, sizeof(nextdir), "%s/%s", current, pFilename);
				}
				else
				{
					Q_snprintf(nextdir, sizeof(nextdir), "%s", pFilename);
				}

				char playermodeldir1[32] = "add-ons";
	
				//Go again search the file, with the new path
				RecursiveFindFiles(playermodeldir1, nextdir);
			}

		}

		//If is NOT . or .. directories...
		if ((Q_stricmp(pFilename, ".")) && (Q_stricmp(pFilename, "..")))
		{

			if (filesystem->FindIsDirectory(findHandle))
			{

				pFilename = filesystem->FindNext(findHandle); //keep searching
			}
			else{

				//Msg("Found '%s/%s'\n", current,pFilename);

				//Checks if the file is a txt file
				char ext[512];
				Q_ExtractFileExtension(pFilename, ext, sizeof(ext));

				if ( !Q_stricmp(ext, "txt") )
				{
					//Either puts the filename together with the root path, or with all the path
					char relative[512];
					if (current[0])
					{
						Q_snprintf(relative, sizeof(relative), "%s/%s", current, pFilename);
					}
					else
					{
						Q_snprintf(relative, sizeof(relative), "%s", pFilename);
					}

					//Msg("Found '%s/%s'\n", current, pFilename);
					//Msg("File is %s \n", pFilename);

					V_StripExtension(pFilename, pFilenameNoExt, sizeof(pFilenameNoExt)); //Deletes extension from filename
					KeyValues *kv = new KeyValues(pFilenameNoExt); //Names the keyvalue as the filename
					kv->SetString(pFilenameNoExt, current); //Adds the path to the keyvalue
					m_pPlayerSelect->AddItem(pFilenameNoExt, kv); //adds it to the combobox
					pFilename = filesystem->FindNext(findHandle); //keep searching
					kv->deleteThis();
				}

				else
				{
					pFilename = filesystem->FindNext(findHandle); //keep searching
				}

			}
			
		}
		else
		{
			pFilename = filesystem->FindNext(findHandle); //keep searching
		}

		
	}
	filesystem->FindClose(findHandle);
}