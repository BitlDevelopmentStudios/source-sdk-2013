//======= Maestra Fenix, 2017 ==================================================//
//
// Purpose: Map load background panel
//
//==============================================================================//

#ifndef MAPLOAD_BACKGROUND_H
#define MAPLOAD_BACKGROUND_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui\ISurface.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls\ImagePanel.h>
#include "ienginevgui.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CMapLoadBG : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CMapLoadBG, vgui::EditablePanel);

public:
	// Construction
	CMapLoadBG( char const *panelName );
	~CMapLoadBG();

	void Paint();

protected:
	void ApplySchemeSettings( vgui::IScheme *pScheme );
	void ResizePanel(void);
	void UpdateMainBackground(void);
	void SetRandBackgroundImage(bool bIsWidescreen);

private:
	MESSAGE_FUNC(OnActivate, "activate");

	vgui::ImagePanel *m_pBackground;
	const char* m_szImagePath;
};

#endif	// !MAPLOAD_BACKGROUND_H