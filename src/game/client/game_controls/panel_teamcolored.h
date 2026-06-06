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
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Frame.h>

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
			BaseClass::SetFgColor(GetTeamColor(pLocalPlayer));
			return;
		}

		BaseClass::SetFgColor(color);
	}

	virtual Color GetFgColor() OVERRIDE
	{
		C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if (pLocalPlayer)
		{
			return GetTeamColor(pLocalPlayer);
		}

		return BaseClass::GetFgColor();
	}

private:
	Color GetTeamColor(C_BasePlayer* pLocalPlayer)
	{
		if (pLocalPlayer)
		{
			int iTeamNumber = pLocalPlayer->GetTeamNumber();
			Color c = GameResources()->GetTeamColor(iTeamNumber);
			return c;
		}

		return BaseClass::GetFgColor();
	}
};

class CEditablePanelTeamColored : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CEditablePanelTeamColored, vgui::EditablePanel);

public:
	CEditablePanelTeamColored(Panel* parent, const char* panelName) : BaseClass(parent, panelName)
	{

	}

	CEditablePanelTeamColored(Panel* parent, const char* panelName, vgui::HScheme scheme) : BaseClass(parent, panelName, scheme)
	{

	}

	virtual void SetFgColor(Color color) OVERRIDE
	{
		C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if (pLocalPlayer)
		{
			BaseClass::SetFgColor(GetTeamColor(pLocalPlayer));
			return;
		}

		BaseClass::SetFgColor(color);
	}

	virtual Color GetFgColor() OVERRIDE
	{
		C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if (pLocalPlayer)
		{
			return GetTeamColor(pLocalPlayer);
		}

		return BaseClass::GetFgColor();
	}

private:
	Color GetTeamColor(C_BasePlayer* pLocalPlayer)
	{
		if (pLocalPlayer)
		{
			int iTeamNumber = pLocalPlayer->GetTeamNumber();
			Color c = GameResources()->GetTeamColor(iTeamNumber);
			return c;
		}

		return BaseClass::GetFgColor();
	}
};

class CFrameTeamColored : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CFrameTeamColored, vgui::Frame);

public:
	CFrameTeamColored(Panel* parent, const char* panelName, bool showTaskbarIcon = true, bool bPopup = true) : BaseClass(parent, panelName, showTaskbarIcon, bPopup)
	{

	}

	virtual void SetFgColor(Color color) OVERRIDE
	{
		C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if (pLocalPlayer)
		{
			BaseClass::SetFgColor(GetTeamColor(pLocalPlayer));
			return;
		}

		BaseClass::SetFgColor(color);
	}

	virtual Color GetFgColor() OVERRIDE
	{
		C_BasePlayer* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if (pLocalPlayer)
		{
			return GetTeamColor(pLocalPlayer);
		}

		return BaseClass::GetFgColor();
	}

private:
	Color GetTeamColor(C_BasePlayer* pLocalPlayer)
	{
		if (pLocalPlayer)
		{
			int iTeamNumber = pLocalPlayer->GetTeamNumber();
			Color c = GameResources()->GetTeamColor(iTeamNumber);
			return c;
		}

		return BaseClass::GetFgColor();
	}
};

#endif // HUD_NUMERICDISPLAY_H