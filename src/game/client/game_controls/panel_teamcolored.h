//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PANEL_TEAMCOLORED_H
#define PANEL_TEAMCOLORED_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseplayer.h"
#include "igameresources.h"

#include <vgui_controls/Panel.h>

//-----------------------------------------------------------------------------
// Purpose: Allows HUD elements to use team colors.
//-----------------------------------------------------------------------------
class CPanelTeamColored : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CPanelTeamColored, vgui::Panel );

public:
	CPanelTeamColored() : BaseClass()
	{

	}

	CPanelTeamColored(Panel* parent) : BaseClass(parent)
	{

	}

	CPanelTeamColored(Panel* parent, const char* panelName) : BaseClass(parent, panelName)
	{

	}

	CPanelTeamColored(Panel* parent, const char* panelName, vgui::HScheme scheme) : BaseClass(parent, panelName, scheme)
	{

	}

	virtual void SetFgColor(Color color) OVERRIDE
	{
		C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if (pLocalPlayer)
		{
			int iTeamNumber = pLocalPlayer->GetTeamNumber();
			BaseClass::SetFgColor(GameResources()->GetTeamColor(iTeamNumber));
			return;
		}

		BaseClass::SetFgColor(color);
	}
};

#endif // HUD_NUMERICDISPLAY_H