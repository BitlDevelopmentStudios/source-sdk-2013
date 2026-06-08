//========= Copyright © 1996-2004, Valve LLC, All rights reserved. ============
//
// Purpose: Simple HUD element
//
//=============================================================================

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <igameresources.h>

class CHudGameMessage : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudGameMessage, vgui::Panel );

public:

	CHudGameMessage( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudGameMessage" ) 
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );
		
		m_pIcon = NULL;

		// Never hide
		SetHiddenBits( 0 );
	};

	void	Init( void );
	void	VidInit( void );
	void	Paint( void );
	
	// Callback function for the "GameMessage" user message
	void	MsgFunc_GameMessage( bf_read &msg );

private:
	CHudTexture *m_pIcon;		// Icon texture reference
	wchar_t		m_pText[256];	// Unicode text buffer

	float		m_flStartTime;	// When the message was recevied
	float		m_flDuration;	// Duration of the message

	CPanelAnimationVarAliasType(float, m_iTextX, "textx", "32", "proportional_float");
	CPanelAnimationVarAliasType(float, m_iTextY, "texty", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, m_iIconX, "iconx", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_iIconY, "icony", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_iIconW, "iconw", "32", "proportional_float");
	CPanelAnimationVarAliasType(float, m_iIconH, "iconh", "32", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hMessageFont, "MessageFont", "Default");
};

DECLARE_HUDELEMENT( CHudGameMessage );
DECLARE_HUD_MESSAGE( CHudGameMessage, GameMessage );

void CHudGameMessage::VidInit( void )
{
	// Store off a reference to our icon
	m_pIcon = gHUD.GetIcon( "message_icon" );

	m_pText[0] = '\0';
}

void CHudGameMessage::Init( void )
{
	HOOK_HUD_MESSAGE( CHudGameMessage, GameMessage );
}

void CHudGameMessage::MsgFunc_GameMessage( bf_read &msg )
{
	// Read in our string
	char szString[256];
	msg.ReadString( szString, sizeof(szString) );

	// Convert it to localize friendly unicode
	const wchar_t* pchFmt = g_pVGuiLocalize->Find(szString);
	if (pchFmt && pchFmt[0])
	{
		Q_wcsncpy(m_pText, pchFmt, sizeof(m_pText));
	}
	else
	{
		g_pVGuiLocalize->ConvertANSIToUnicode(szString, m_pText, sizeof(m_pText));
	}

	// Setup our time trackers
	m_flStartTime = gpGlobals->curtime;
	m_flDuration = msg.ReadFloat();
}

void CHudGameMessage::Paint( void )
{
	if ( !m_pIcon )
		return;
	
	// Find our fade based on our time shown
	float dt = ( m_flStartTime - gpGlobals->curtime );
	float flAlpha = SimpleSplineRemapVal( dt, 0.0f, m_flDuration, 255, 0 );
	flAlpha = clamp( flAlpha, 0.0f, 255.0f );

	// Draw our icon
	if (GameResources())
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		Color TeamColor = Color(255, 255, 255, flAlpha);
		if (pPlayer)
		{
			TeamColor = GameResources()->GetTeamColor(pPlayer->GetTeamNumber());
			TeamColor[3] = flAlpha;
		}

		m_pIcon->DrawSelf(m_iIconX, m_iIconY, m_iIconW, m_iIconH, TeamColor);
	}
	else
	{
		m_pIcon->DrawSelf(m_iIconX, m_iIconY, m_iIconW, m_iIconH, Color(255, 255, 255, flAlpha));
	}

	// Draw our text
	surface()->DrawSetTextFont(m_hMessageFont); // set the font	
	if (GameResources())
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		Color TeamColor = Color(255, 255, 255, flAlpha);
		if (pPlayer)
		{
			TeamColor = GameResources()->GetTeamColor(pPlayer->GetTeamNumber());
			TeamColor[3] = flAlpha;
		}

		surface()->DrawSetTextColor(TeamColor);
	}
	else
	{
		surface()->DrawSetTextColor(255, 255, 255, flAlpha); // white
	}

	surface()->DrawSetTextPos(m_iTextX, m_iTextY); // x,y position
	surface()->DrawPrintText( m_pText, wcslen(m_pText) ); // print text

	Color originalbg = GetColor("Panel.BgColor");
	Color bg = originalbg;
	if (flAlpha == 0.0f)
	{
		bg[3] = 0.0f;
	}
	else
	{
		bg[3] = originalbg.a();
	}

	SetBgColor(bg);
}

