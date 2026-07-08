//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef TipsDialog_H
#define TipsDialog_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

//-----------------------------------------------------------------------------
// Purpose: dialog for launching a listenserver
//-----------------------------------------------------------------------------
class CTipsDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CTipsDialog,  vgui::Frame );

public:
	CTipsDialog(vgui::Panel *parent);
	~CTipsDialog();
	
	// returns currently entered information about the server
	void DialogInit();

private:
	virtual void OnCommand( const char *command );
	virtual void OnClose();
	virtual void OnNext();
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	
	vgui::Label *m_lCurrentTip;
};


#endif
