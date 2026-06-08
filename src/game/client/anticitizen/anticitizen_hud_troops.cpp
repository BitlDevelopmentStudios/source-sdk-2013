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
// implementation of CHudTroops class
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

#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_HEALTH -1

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudTroops : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudTroops, CHudNumericDisplay );

public:
	CHudTroops( const char *pElementName );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void OnThink();

private:
	// old variables
	int		m_iTroops;
};	

DECLARE_HUDELEMENT( CHudTroops );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudTroops::CHudTroops( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudTroops")
{
	SetHiddenBits( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTroops::Init()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTroops::Reset()
{
	m_iTroops = INIT_HEALTH;

	wchar_t *tempString = g_pVGuiLocalize->Find("#Anticitizen_RemainingTroops");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"SOLDIERS REMAINING");
	}
	
	SetDisplayValue(m_iTroops);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTroops::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTroops::OnThink()
{
	if (!HL2MPRules())
		return;

	// only show if there's no timer.
	float flCurTime = HL2MPRules()->GetMapRemainingTime();
	if (flCurTime <= 0 && (HL2MPRules()->GetRemainingSoldierCount() > 0) && (HL2MPRules()->GetTimerState() != TIMERSTATE_NONE))
	{
		SetAlpha(255);
		
		int newTroops = HL2MPRules()->GetRemainingSoldierCount();
		// Never below zero
		newTroops = MAX( HL2MPRules()->GetRemainingSoldierCount(), 0);

		// Only update the fade if we've changed health
		if ( newTroops == m_iTroops )
		{
			return;
		}

		m_iTroops = newTroops;

		SetDisplayValue(m_iTroops);
	}
	else
	{
		SetAlpha(0);
	}
}