//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>
#include "hud.h"
#include "hud_suitpower.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "hl2mp_gamerules.h"
#include "game_controls/panel_teamcolored.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Shows the flashlight icon
//-----------------------------------------------------------------------------
class CHudFreemanHealth : public CHudElement, public CPanelTeamColored
{
	DECLARE_CLASS_SIMPLE( CHudFreemanHealth, CPanelTeamColored);

public:
	CHudFreemanHealth( const char *pElementName );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	//i am shocked i am able to compile this mod with this not having any function body.
	//int GetXpMultiplier();
	bool ShouldDraw();

protected:
	virtual void Paint();

private:
	void Reset( void );

	CPanelAnimationVarAliasType( float, m_flBarInsetX, "BarInsetX", "5", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarInsetY, "BarInsetY", "15", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarWidth, "BarWidth", "140", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarHeight, "BarHeight", "5", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarChunkWidth, "BarChunkWidth", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flBarChunkGap, "BarChunkGap", "1", "proportional_float" );
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );
	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "5", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "3", "proportional_float" );
	CPanelAnimationVarAliasType(float, text_xpos2, "text_xpos2", "5", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos2, "text_ypos2", "13", "proportional_float");
};	

using namespace vgui;


DECLARE_HUDELEMENT( CHudFreemanHealth );


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudFreemanHealth::CHudFreemanHealth( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "HudFreemanHealth" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pScheme - 
//-----------------------------------------------------------------------------
void CHudFreemanHealth::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Purpose: Start with our background off
//-----------------------------------------------------------------------------
void CHudFreemanHealth::Reset( void )
{
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "VitalsOn" );
}

bool CHudFreemanHealth::ShouldDraw(void)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

	bool bFreemanExists = (HL2MPRules() && (HL2MPRules()->GetFreeman() != NULL));

	bool bNeedsDraw = pPlayer && (pPlayer->GetTeamNumber() != TEAM_FREEMAN) && bFreemanExists && (GetAlpha() > 0);

	return (bNeedsDraw && CHudElement::ShouldDraw());
}

//-----------------------------------------------------------------------------
// Purpose: draws the flashlight icon
//-----------------------------------------------------------------------------
void CHudFreemanHealth::Paint()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	
	if (pPlayer)
	{
		// get bar chunks
		int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
		int enabledChunks = (chunkCount * HL2MPRules()->GetFreemanHealthFraction()); //+ 0.5f);
		bool bFreemanDead = ((HL2MPRules()->GetFreemanHealth() == 0) || !HL2MPRules()->IsFreemanAlive());

		if (bFreemanDead)
		{
			enabledChunks = chunkCount;
		}

		Color clrHealth;
		clrHealth = GetFgColor();
		Color clrText;
		clrText = GetFgColor();

		// draw our name
		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawSetTextColor(clrText);
		surface()->DrawSetTextPos(text_xpos, text_ypos);

		wchar_t* tempString = g_pVGuiLocalize->Find("#Anticitizen_FreemanVitals");

		if (pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
		{
			if (HL2MPRules()->IsInFinalRound())
			{
				tempString = g_pVGuiLocalize->Find("#Anticitizen_FreemanVitals_Spectator");
			}
			else
			{
				tempString = g_pVGuiLocalize->Find("#Anticitizen_FreemanVitals_Spectator2");
			}
		}

		if (tempString)
		{
			surface()->DrawPrintText(tempString, wcslen(tempString));
		}
		else
		{
			surface()->DrawPrintText(L"TARGET VITALS", wcslen(L"TARGET VITALS"));
		}

		if (!bFreemanDead)
		{
			// render the health bar

			// draw the suit power bar
			surface()->DrawSetColor(clrHealth);
			int xpos = m_flBarInsetX, ypos = m_flBarInsetY;
			for (int i = 0; i < enabledChunks; i++)
			{
				surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
				xpos += (m_flBarChunkWidth + m_flBarChunkGap);
			}

			// Be even less transparent than we already are
			clrHealth[3] = clrHealth[3] / 8;

			// draw the exhausted portion of the bar.
			surface()->DrawSetColor(clrHealth);
			for (int i = enabledChunks; i < chunkCount; i++)
			{
				surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
				xpos += (m_flBarChunkWidth + m_flBarChunkGap);
			}
		}
		else
		{
			// render the "dead" version of the bar

			// Be even less transparent than we already are
			clrHealth[3] = clrHealth[3] / 8;

			// draw the suit power bar
			surface()->DrawSetColor(clrHealth);
			int xpos = m_flBarInsetX, ypos = m_flBarInsetY;
			for (int i = 0; i < enabledChunks; i++)
			{
				surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
				xpos += (m_flBarChunkWidth + m_flBarChunkGap);
			}
		}
	}
}
