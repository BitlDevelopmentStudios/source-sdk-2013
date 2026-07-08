//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "TipsDialog.h"

#include <stdio.h>

using namespace vgui;

#include <vgui/ILocalize.h>

#include "filesystem.h"
#include <KeyValues.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/BitmapImagePanel.h>
#include "tier1/convar.h"
#include "anticitizen_tips.h"

// for SRC
#include <vstdlib/random.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTipsDialog::CTipsDialog(vgui::Panel *parent) : BaseClass(NULL, "TipsDialog")
{
	SetSize(348, 460);
	//SetOKButtonText("#GameUI_Start");
	
	// we can use this if we decide we want to put "listen server" at the end of the game name
	m_lCurrentTip = new Label(this, "CurrentTipLabel", "");

	LoadControlSettings("Resource/TipsDialog.res");

	// create KeyValues object to load/save config options

	DialogInit();
	SetSizeable(false);
	SetDeleteSelfOnClose(true);
	MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTipsDialog::~CTipsDialog()
{
}

void CTipsDialog::DialogInit()
{
	m_lCurrentTip->SetText(UTIL_GetRandomTipUnicode());
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTipsDialog::OnClose()
{
	BaseClass::OnClose();
	MarkForDeletion();
}

void CTipsDialog::OnNext()
{
	m_lCurrentTip->SetText(UTIL_GetRandomTipUnicode());
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *command - 
//-----------------------------------------------------------------------------
void CTipsDialog::OnCommand(const char *command)
{
	if (!stricmp(command, "Next"))
	{
		OnNext();
		return;
	}
	else if ( !stricmp( command, "Ok" ) )
	{
		OnClose();
		return;
	}

	//BaseClass::OnCommand( command );
}

void CTipsDialog::OnKeyCodeTyped(KeyCode code)
{
	// force ourselves to be closed if the escape key it pressed
	if (code == KEY_ESCAPE)
	{
		Close();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

CON_COMMAND(tipsdialog, "")
{
	CTipsDialog* pCTipsDialog = new CTipsDialog(NULL);
	pCTipsDialog->Activate();
}