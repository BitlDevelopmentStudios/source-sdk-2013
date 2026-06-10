//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
//
// implementation of CHudLives class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "hl2mp/hl2mp_gamerules.h"
#include "hl2mp/c_hl2mp_player.h"

#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_HEALTH -1

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudLives : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudLives, CHudNumericDisplay );

public:
	CHudLives( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

private:
	// old variables
	int		m_iLives;
};	

DECLARE_HUDELEMENT( CHudLives );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudLives::CHudLives( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudLives")
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudLives::Init()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudLives::Reset()
{
	m_iLives = INIT_HEALTH;

	wchar_t *tempString = g_pVGuiLocalize->Find("#Anticitizen_Lives");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"LIVES");
	}
	
	SetDisplayValue(m_iLives);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudLives::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudLives::OnThink()
{
	if (!HL2MPRules())
		return;

	// only show if there's no timer.
	float flCurTime = HL2MPRules()->GetMapRemainingTime();
	C_HL2MP_Player* local = ToHL2MPPlayer(C_BasePlayer::GetLocalPlayer());
	if (flCurTime <= 0 && (local && (local->GetTeamNumber() == TEAM_COMBINE) && (local->GetLifeCount() > 0)) && (HL2MPRules()->GetTimerState() != TIMERSTATE_NONE))
	{
		SetAlpha(255);

		int newLives = local->GetLifeCount();
		// Never below zero
		newLives = MAX(local->GetLifeCount(), 0);

		// Only update the fade if we've changed health
		if (newLives == m_iLives)
		{
			return;
		}

		m_iLives = newLives;

		SetDisplayValue(m_iLives);
	}
	else
	{
		SetAlpha(0);
	}
}