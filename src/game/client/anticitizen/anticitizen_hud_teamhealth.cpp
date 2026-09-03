//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "c_team.h"
#include "iclientmode.h"
#include "c_playerresource.h"
#include "c_anticitizen_player_resource.h"
#include "anticitizen_hud_teamhealth.h"
#include "hl2mp_gamerules.h"

#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>

#include <vgui/IVGui.h>

ConVar cl_show_teamhealthbars("cl_show_teamhealthbars", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
ConVar cl_show_teamhealthbars_spec("cl_show_teamhealthbars_spec", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

using namespace vgui;

DECLARE_HUDELEMENT( CAnticitizenHudTeamHealth );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAnticitizenHudTeamHealth::CAnticitizenHudTeamHealth( const char *pszElementName ) : CHudElement( pszElementName ), EditablePanel( NULL, "HudTeamHealth" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	SetHiddenBits( HIDEHUD_MISCSTATUS );

	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);

	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAnticitizenHudTeamHealth::ShouldDraw( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizenHudTeamHealth::RemoveEntity( int nRemove )
{
	FOR_EACH_VEC( m_vecEntitiesToDraw, i )
	{
		if ( m_vecEntitiesToDraw[i].m_nEntIndex == nRemove )
		{
			m_vecEntitiesToDraw.Remove( i );
			return;
		}
	}
}

Color GetTeamColor(C_BasePlayer* pLocalPlayer)
{
	if (pLocalPlayer)
	{
		int iTeamNumber = pLocalPlayer->GetTeamNumber();
		Color c = GameResources()->GetTeamColor(iTeamNumber);
		return c;
	}

	return Color(255, 255, 255, 255);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizenHudTeamHealth::OnTick()
{
	BaseClass::OnTick();

	if (!g_PR)
		return;

	if (!cl_show_teamhealthbars.GetBool())
		return;

	C_HL2MP_Player* pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if (!pLocalPlayer)
		return;

	if (!cl_show_teamhealthbars.GetBool() && (pLocalPlayer->GetTeamNumber() != TEAM_SPECTATOR))
		return;

	if (!cl_show_teamhealthbars_spec.GetBool() && (pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR))
		return;

	int nLocalPlayerTeam = pLocalPlayer->GetTeamNumber();

	// loop through the players
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		if (!g_PR->IsConnected(i))
		{
			RemoveEntity(i);
			continue;
		}

		C_HL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));
		if (!pPlayer || (pPlayer == pLocalPlayer))
		{
			RemoveEntity(i);
			continue;
		}

		int nPlayerTeamNumber = pPlayer->GetTeamNumber();

		// remove the entities we don't want to draw anymore
		if (((nPlayerTeamNumber <= TEAM_UNASSIGNED)) ||
			(!pPlayer->IsAlive()) ||
			((nPlayerTeamNumber != nLocalPlayerTeam) && (nLocalPlayerTeam != TEAM_SPECTATOR)))
		{
			RemoveEntity(i);
			continue;
		}

		// passed all of the tests, so make sure they're in the list
		int nVecIndex = -1;
		FOR_EACH_VEC(m_vecEntitiesToDraw, nTemp)
		{
			if (m_vecEntitiesToDraw[nTemp].m_nEntIndex == i)
			{
				nVecIndex = nTemp;
				break;
			}
		}
		if (nVecIndex == -1)
		{
			nVecIndex = m_vecEntitiesToDraw.AddToTail();
		}

		// set the player index
		m_vecEntitiesToDraw[nVecIndex].m_nEntIndex = i;

		// use actual name or disguised name?
		g_pVGuiLocalize->ConvertANSIToUnicode(g_PR->GetPlayerName(i), m_vecEntitiesToDraw[nVecIndex].m_wszName, sizeof(m_vecEntitiesToDraw[nVecIndex].m_wszName));
		m_vecEntitiesToDraw[nVecIndex].m_nNameWidth = UTIL_ComputeStringWidth(m_hNameFont, m_vecEntitiesToDraw[nVecIndex].m_wszName);

		m_vecEntitiesToDraw[nVecIndex].m_nOffset = (VEC_HULL_MAX_SCALED(pPlayer).z);

		// use actual health or disguised health?
		float flHealth = 1.0f;
		int maxHealth = pPlayer->GetMaxHealth();

		// TODO: max health is not networked......
		if (pPlayer->GetPlayerClass() > CLS_INVALID)
		{
			const CAnticitizen_FilePlayerClassInfo_t& info = pPlayer->GetPlayerClassInfo();
			maxHealth = info.iHealth;
		}

		flHealth = (float)(pPlayer->GetHealth()) / (float)(maxHealth);

		// don't show buffed health for this simple bar
		if (flHealth > 1.0f)
		{
			flHealth = 1.0f;
		}
		m_vecEntitiesToDraw[nVecIndex].m_flHealth = flHealth;

		m_vecEntitiesToDraw[nVecIndex].m_clrBarFGColor = GetTeamColor(pPlayer);

		m_vecEntitiesToDraw[nVecIndex].m_bIsFreeman = (pPlayer->GetTeamNumber() == TEAM_FREEMAN);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizenHudTeamHealth::Paint()
{
	BaseClass::Paint();

	if (!g_PR)
		return;

	C_HL2MP_Player* pLocalPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
	if (!pLocalPlayer)
		return;

	if (!cl_show_teamhealthbars.GetBool() && (pLocalPlayer->GetTeamNumber() != TEAM_SPECTATOR))
		return;

	if (!cl_show_teamhealthbars_spec.GetBool() && (pLocalPlayer->GetTeamNumber() == TEAM_SPECTATOR))
		return;

	int nNameOffset = 37;
	int nHealthWidth = 70;
	int nHealthHeight = 6;
	int nBGOffset = 1;
	int nBGTextOffset = 1;
	int nBGWidth = (nHealthWidth + nBGOffset);
	int nBGHeight = (nHealthHeight + nBGOffset);

	FOR_EACH_VEC( m_vecEntitiesToDraw, i )
	{
		int nEntIndex = m_vecEntitiesToDraw[i].m_nEntIndex;
		if ( IsPlayerIndex( nEntIndex ) && !g_PR->IsConnected( nEntIndex ) )
			continue;

		C_BaseEntity *pEnt = cl_entitylist->GetEnt( nEntIndex );
		if ( !pEnt )
			continue;

		Vector vecPos = pEnt->GetAbsOrigin();
		vecPos.z += m_vecEntitiesToDraw[i].m_nOffset;

		int iX, iY;
		Vector vecWorld( vecPos.x, vecPos.y, vecPos.z );
		if ( GetVectorInHudSpace( vecWorld, iX, iY ) )
		{
			int xTextPos = iX - (m_vecEntitiesToDraw[i].m_nNameWidth / 2);
			int yTextPos = iY - nNameOffset;

			vgui::surface()->DrawSetTextFont((m_vecEntitiesToDraw[i].m_bIsFreeman ? m_hNameFreemanFont : m_hNameFont));
			vgui::surface()->DrawSetTextPos(xTextPos + nBGTextOffset, yTextPos + nBGTextOffset);
			vgui::surface()->DrawSetTextColor(GetBgColor());
			vgui::surface()->DrawPrintText(m_vecEntitiesToDraw[i].m_wszName, wcslen(m_vecEntitiesToDraw[i].m_wszName), vgui::FONT_DRAW_NONADDITIVE);

 			// draw the name
			vgui::surface()->DrawSetTextPos(xTextPos, yTextPos);
			vgui::surface()->DrawSetTextColor( m_vecEntitiesToDraw[i].m_clrBarFGColor);
			vgui::surface()->DrawPrintText( m_vecEntitiesToDraw[i].m_wszName, wcslen( m_vecEntitiesToDraw[i].m_wszName ), vgui::FONT_DRAW_NONADDITIVE );

			int xHealthPos = iX - 35;
			int yHealthPos = iY - 10;

			// draw the "border"
			vgui::surface()->DrawSetColor(GetBgColor());
			vgui::surface()->DrawFilledRect((xHealthPos - nBGOffset), (yHealthPos - nBGOffset), xHealthPos + nBGWidth, yHealthPos + nBGHeight);

			float adjuster = 0.35f;

			Color bgcol = Color((m_vecEntitiesToDraw[i].m_clrBarFGColor.r() * adjuster),
				(m_vecEntitiesToDraw[i].m_clrBarFGColor.g() * adjuster),
				(m_vecEntitiesToDraw[i].m_clrBarFGColor.b() * adjuster),
				m_vecEntitiesToDraw[i].m_clrBarFGColor.a());

			// draw the health bar background
			vgui::surface()->DrawSetColor(bgcol);
			vgui::surface()->DrawFilledRect( xHealthPos, yHealthPos, xHealthPos + nHealthWidth, yHealthPos + nHealthHeight );

			// draw the health bar
			vgui::surface()->DrawSetColor(m_vecEntitiesToDraw[i].m_clrBarFGColor);
			vgui::surface()->DrawFilledRect( xHealthPos, yHealthPos, xHealthPos + ( nHealthWidth * m_vecEntitiesToDraw[i].m_flHealth ), yHealthPos + nHealthHeight );
		}
	}
}
