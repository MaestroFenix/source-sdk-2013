//==========================================================================//
//
// Purpose: New playermodel panel created by Maestro Fenix 2014
//
// $NoKeywords: $
//===========================================================================//
class NewPlayermodelPanel
{
public:
	virtual void		Create(vgui::VPANEL parent) = 0;
	virtual void		Destroy(void) = 0;
	virtual void		Activate(void) = 0;
};

extern NewPlayermodelPanel* mypanel;