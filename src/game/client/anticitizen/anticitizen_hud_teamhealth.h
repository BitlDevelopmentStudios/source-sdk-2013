//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef ANTICITIZEN_HUD_TEAM_HEALTH_H
#define ANTICITIZEN_HUD_TEAM_HEALTH_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/EditablePanel.h>

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CAnticitizenHudTeamHealth : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CAnticitizenHudTeamHealth, vgui::EditablePanel );
public:
	CAnticitizenHudTeamHealth( const char *pElementName );
	virtual ~CAnticitizenHudTeamHealth(){}

	virtual bool	ShouldDraw( void ) OVERRIDE;
	virtual void	OnTick() OVERRIDE;
	virtual void	Paint() OVERRIDE;

	void RemoveEntity( int nRemove );

private:

	typedef struct
	{
		int m_nEntIndex;
		wchar_t m_wszName[MAX_PLAYER_NAME_LENGTH];
		int m_nNameWidth;
		float m_flHealth;
		Color m_clrBarFGColor;
		int m_nOffset;
		bool m_bIsFreeman;
	} team_health_t;

	CUtlVector< team_health_t > m_vecEntitiesToDraw;
	CPanelAnimationVar( vgui::HFont, m_hNameFont, "name_font", "TeamHealthFont" );
	CPanelAnimationVar(vgui::HFont, m_hNameFreemanFont, "freeman_font", "TeamHealthFreemanFont");
};

#endif // ANTICITIZEN_HUD_TEAM_HEALTH_H
