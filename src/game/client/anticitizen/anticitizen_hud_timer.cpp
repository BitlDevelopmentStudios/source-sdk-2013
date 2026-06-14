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
// implementation of CHudTimer class
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
#include "hud_basetimer.h"
#include "hl2mp/hl2mp_gamerules.h"

#include "convar.h"

#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define INIT_TIMER -1

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudTimer : public CHudElement, public CHudBaseTimer
{
	DECLARE_CLASS_SIMPLE(CHudTimer, CHudBaseTimer);

public:
	CHudTimer(const char* pElementName);
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void OnThink();

private:
	// old variables
};

DECLARE_HUDELEMENT(CHudTimer);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudTimer::CHudTimer(const char* pElementName) : CHudElement(pElementName), CHudBaseTimer(NULL, "HudTimer")
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTimer::Init()
{
	Reset();
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("TimerInit");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTimer::Reset()
{
	SetMinutes(INIT_TIMER);
	SetSeconds(INIT_TIMER);
	SetAlpha(255);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTimer::VidInit()
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudTimer::OnThink()
{
	if (!HL2MPRules())
		return;

	float flCurTime = HL2MPRules()->GetMapRemainingTime();
	if (flCurTime > 0)
	{
		SetAlpha(255);
		int iRemain = (int)flCurTime;
		int iMinutes, iSeconds;
		iMinutes = iRemain / 60;
		iSeconds = iRemain % 60;
		SetMinutes(iMinutes);
		SetSeconds(iSeconds);

		if (iMinutes == 0 && iSeconds < 30)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("TimerBelow30");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("TimerAbove30");
		}

		// now, we set the label text depending on the type of timer.

		const char *szLabel = "";

		switch (HL2MPRules()->GetTimerState())
		{
			case TIMERSTATE_ROUNDSTART:
				szLabel = "#Anticitizen_TimerRound";
				break;
			case TIMERSTATE_GAMESTART:
				szLabel = "#Anticitizen_TimerGame";
				break;
			case TIMERSTATE_RESTART:
				szLabel = "#Anticitizen_TimerRestart";
				break;
			case TIMERSTATE_CHANGELEVEL:
				szLabel = "#Anticitizen_TimerChangelevel";
				break;
		}

		wchar_t wchText[256];	// Unicode text buffer
		const wchar_t* pchFmt = g_pVGuiLocalize->Find(szLabel);
		if (pchFmt && pchFmt[0])
		{
			Q_wcsncpy(wchText, pchFmt, sizeof(wchText));
		}
		else
		{
			g_pVGuiLocalize->ConvertANSIToUnicode(szLabel, wchText, sizeof(wchText));
		}

		SetLabelText(wchText);
	}
	else
	{
		SetAlpha(0);
	}
}