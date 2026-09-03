//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hl2mp_gamerules.h"
#include "viewport_panel_names.h"
#include "gameeventdefs.h"
#include <KeyValues.h>
#include "ammodef.h"
#include "fmtstr.h"
#include "achievements_anticitizen.h"
#include "achievementmgr.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_anticitizen_player_resource.h"
	#include "c_team.h"
#else

	#include "nav_mesh.h"
	#include "eventqueue.h"
	#include "player.h"
	#include "gamerules.h"
	#include "game.h"
	#include "items.h"
	#include "entitylist.h"
	#include "mapentities.h"
	#include "in_buttons.h"
	#include <ctype.h>
	#include "voice_gamemgr.h"
	#include "iscorer.h"
	#include "hl2mp_player.h"
	#include "team.h"
	#include "voice_gamemgr.h"
	#include "hl2mp_gameinterface.h"
	#include "hl2mp_cvars.h"
	#include "player_resource.h"
	#include "anticitizen_player_resource.h"
	#include "bot/hl2mp_bot.h"

extern void respawn(CBaseEntity *pEdict, bool fCopyCorpse);

extern bool FindInList( const char **pStrings, const char *pToFind );

ConVar sv_hl2mp_weapon_respawn_time( "sv_hl2mp_weapon_respawn_time", "20", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_hl2mp_item_respawn_time( "sv_hl2mp_item_respawn_time", "30", FCVAR_GAMEDLL | FCVAR_NOTIFY );
ConVar sv_report_client_settings("sv_report_client_settings", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY );

ConVar sv_minplayerstostart("sv_minplayerstostart", "2", FCVAR_GAMEDLL | FCVAR_NOTIFY);

ConVar sv_startwaitime("sv_startwaitime", "30", FCVAR_GAMEDLL | FCVAR_NOTIFY);
ConVar sv_startplaywaitime("sv_startplaywaitime", "5", FCVAR_GAMEDLL | FCVAR_NOTIFY);

ConVar sv_freemanroundlimit("sv_freemanroundlimit", "3", FCVAR_GAMEDLL | FCVAR_NOTIFY);

ConVar sv_roundlimit("sv_roundlimit", "5", FCVAR_GAMEDLL | FCVAR_NOTIFY);

ConVar sv_spectatorlimit("sv_spectatorlimit", "2", FCVAR_GAMEDLL | FCVAR_NOTIFY);

ConVar sv_friendlyfire_deathnotice("sv_friendlyfire_deathnotice", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY);

ConVar sv_randomize_freeman_player("sv_randomize_freeman_player", "0", FCVAR_GAMEDLL | FCVAR_NOTIFY);

extern ConVar mp_chattime;

extern CBaseEntity	 *g_pLastCombineSpawn;
extern CBaseEntity	 *g_pLastRebelSpawn;

#define WEAPON_MAX_DISTANCE_FROM_SPAWN 64

#endif
ConVar hl2mp_avoidteammates("hl2mp_avoidteammates", "1", FCVAR_REPLICATED, "If enabled, players on the same team will not collide with each other.");

REGISTER_GAMERULES_CLASS( CHL2MPRules );

BEGIN_NETWORK_TABLE_NOBASE( CHL2MPRules, DT_HL2MPRules )
#ifdef CLIENT_DLL
RecvPropBool(RECVINFO(m_bFinalRound)),
RecvPropBool(RECVINFO(m_bIsInIntermission)),
RecvPropFloat(RECVINFO(m_flGameStartTime)),
RecvPropFloat(RECVINFO(m_flGameEndTime)),
RecvPropInt(RECVINFO(m_iTimerType)),
RecvPropInt(RECVINFO(m_iSoldiers)),
RecvPropInt(RECVINFO(m_iRoundState)),
RecvPropInt(RECVINFO(m_iCurrentRound)),
RecvPropInt(RECVINFO(m_iGameEndReason)),
RecvPropFloat(RECVINFO(m_flTimeSinceGameStart)),
#else
SendPropBool(SENDINFO(m_bFinalRound)),
SendPropBool(SENDINFO(m_bIsInIntermission)),
SendPropFloat(SENDINFO(m_flGameStartTime)),
SendPropFloat(SENDINFO(m_flGameEndTime)),
SendPropInt(SENDINFO(m_iTimerType)),
SendPropInt(SENDINFO(m_iSoldiers)),
SendPropInt(SENDINFO(m_iRoundState)),
SendPropInt(SENDINFO(m_iCurrentRound)),
SendPropInt(SENDINFO(m_iGameEndReason)),
SendPropFloat(SENDINFO(m_flTimeSinceGameStart)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( hl2mp_gamerules, CHL2MPGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( HL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )

static HL2MPViewVectors g_HL2MPViewVectors(
	Vector( 0, 0, 64 ),       //VEC_VIEW (m_vView) 
							  
	Vector(-16, -16, 0 ),	  //VEC_HULL_MIN (m_vHullMin)
	Vector( 16,  16,  72 ),	  //VEC_HULL_MAX (m_vHullMax)
							  					
	Vector(-16, -16, 0 ),	  //VEC_DUCK_HULL_MIN (m_vDuckHullMin)
	Vector( 16,  16,  36 ),	  //VEC_DUCK_HULL_MAX	(m_vDuckHullMax)
	Vector( 0, 0, 28 ),		  //VEC_DUCK_VIEW		(m_vDuckView)
							  					
	Vector(-10, -10, -10 ),	  //VEC_OBS_HULL_MIN	(m_vObsHullMin)
	Vector( 10,  10,  10 ),	  //VEC_OBS_HULL_MAX	(m_vObsHullMax)
							  					
	Vector( 0, 0, 14 ),		  //VEC_DEAD_VIEWHEIGHT (m_vDeadViewHeight)

	Vector(-16, -16, 0 ),	  //VEC_CROUCH_TRACE_MIN (m_vCrouchTraceMin)
	Vector( 16,  16,  60 )	  //VEC_CROUCH_TRACE_MAX (m_vCrouchTraceMax)
);

static const char *s_PreserveEnts[] =
{
	"ai_network",
	"ai_hint",
	"hl2mp_gamerules",
	"team_manager",
	"anticitizen_player_manager",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"func_brush",
	"func_wall",
	"func_buyzone",
	"func_illusionary",
	"infodecal",
	"info_projecteddecal",
	"info_node",
	"info_target",
	"info_node_hint",
	"info_player_deathmatch",
	"info_player_combine",
	"info_player_rebel",
	"info_player_freeman",
	"info_map_parameters",
	"keyframe_rope",
	"move_rope",
	"info_ladder",
	"player",
	"point_viewcontrol",
	"scene_manager",
	"shadow_control",
	"sky_camera",
	"soundent",
	"trigger_soundscape",
	"viewmodel",
	"predicted_viewmodel",
	#ifdef C_ARMS
	"hand_viewmodel", // Our new viewmodel entity
	#endif
	"worldspawn",
	"point_devshot_camera",
	"env_spritetrail", // players spawn with and manage this entity themselves
	"", // END Marker
};

#ifdef CLIENT_DLL
	void RecvProxy_HL2MPRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CHL2MPRules *pRules = HL2MPRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CHL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )
		RecvPropDataTable( "hl2mp_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_HL2MPRules ), RecvProxy_HL2MPRules )
	END_RECV_TABLE()
#else
	void* SendProxy_HL2MPRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CHL2MPRules *pRules = HL2MPRules();
		Assert( pRules );
		return pRules;
	}

	BEGIN_SEND_TABLE( CHL2MPGameRulesProxy, DT_HL2MPGameRulesProxy )
		SendPropDataTable( "hl2mp_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_HL2MPRules ), SendProxy_HL2MPRules )
	END_SEND_TABLE()
#endif

#ifndef CLIENT_DLL

	class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
	{
	public:
		virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
		{
			return ( pListener->GetTeamNumber() == pTalker->GetTeamNumber() );
		}
	};
	CVoiceGameMgrHelper g_VoiceGameMgrHelper;
	IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

#endif

// NOTE: the indices here must match TEAM_TERRORIST, TEAM_CT, TEAM_SPECTATOR, etc.
char *sTeamNames[] =
{
	"Unassigned",
	"Spectator",
	"Combine",
	"Freeman",
};

CHL2MPRules::CHL2MPRules()
{
#ifndef CLIENT_DLL
	// Create the team managers
	for ( int i = 0; i < ARRAYSIZE( sTeamNames ); i++ )
	{
		CTeam *pTeam = static_cast<CTeam*>(CreateEntityByName( "team_manager" ));
		pTeam->Init( sTeamNames[i], i );

		g_Teams.AddToTail( pTeam );
	}

	m_flIntermissionEndTime = 0.0f;
	m_bIsInIntermission = false;
	m_flGameStartTime = 0;
	m_flGameEndTime = 0;
	m_flTimeSinceGameStart = 0;
	m_iTimerType = TIMERSTATE_NONE;
	m_iSoldiers = 0;
	// the init state is round 1.
	m_iCurrentRound = 1;

	m_hRespawnableItemsAndWeapons.RemoveAll();
	m_bCompleteReset = false;
	m_bChangelevelDone = false;
	m_bHasMinPlayersToStart = false;
	m_bLastSquadMemberAnnounced = false;
	pFreeman = NULL;
	pNextPlayerToBecomeFreeman = NULL;
	m_iRoundState = STATE_PREROUND;
	m_bStartedStartClock = false;
	m_bAnnouncedGameStart = false;
	m_bAnnouncedGameEnd = false;
	m_bGaveGameEndAchievements = false;
	m_bSentGameEndEvent = false;
	m_bStrippedFlags = false;
	m_bJustEnded = false;
	m_bReassignSpectators = false;
	m_uiFreemanID = 0;
	m_uiLastFreemanID = 0;
	m_iNumTimesFreemanIDShowedUpIFuckingHateThis = 0;

#endif
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Counts the accumulated # of primary and secondary attacks from all
//			weapons (except grav gun).  If bBulletOnly is true, only counts
//			attacks with ammo that does bullet damage.
//-----------------------------------------------------------------------------
int CalcPlayerAttacks(CBasePlayer *pPlayer, bool bBulletOnly)
{
	CAmmoDef* pAmmoDef = GetAmmoDef();
	if (!pPlayer || !pAmmoDef)
		return 0;

	int iTotalAttacks = 0;
	int iWeapons = pPlayer->WeaponCount();
	for (int i = 0; i < iWeapons; i++)
	{
		CBaseHL2MPCombatWeapon* pWeapon = dynamic_cast<CBaseHL2MPCombatWeapon*>(pPlayer->GetWeapon(i));
		if (pWeapon)
		{
			// add primary attacks if we were asked for all attacks, or only if it uses bullet ammo if we were asked to count bullet attacks
			if (!bBulletOnly || 
				(pAmmoDef->m_AmmoType[pWeapon->GetPrimaryAmmoType()].nDamageType == DMG_BULLET) || 
				(pAmmoDef->m_AmmoType[pWeapon->GetPrimaryAmmoType()].nDamageType == (DMG_BULLET | DMG_BUCKSHOT)) ||
				(pAmmoDef->m_AmmoType[pWeapon->GetPrimaryAmmoType()].nDamageType == (DMG_BULLET | DMG_SNIPER)))
			{
				iTotalAttacks += pWeapon->m_iPrimaryAttacks;
			}
			// add secondary attacks if we were asked for all attacks, or only if it uses bullet ammo if we were asked to count bullet attacks
			if (!bBulletOnly || 
				(pAmmoDef->m_AmmoType[pWeapon->GetSecondaryAmmoType()].nDamageType == DMG_BULLET) ||
				(pAmmoDef->m_AmmoType[pWeapon->GetSecondaryAmmoType()].nDamageType == (DMG_BULLET | DMG_BUCKSHOT)) ||
				(pAmmoDef->m_AmmoType[pWeapon->GetSecondaryAmmoType()].nDamageType == (DMG_BULLET | DMG_SNIPER)))
			{
				iTotalAttacks += pWeapon->m_iSecondaryAttacks;
			}
		}
	}
	return iTotalAttacks;
}
#endif

int CHL2MPRules::GetFreemanBulletsShot()
{
#ifndef CLIENT_DLL
	if (!pFreeman)
		return -1;

	// get # of attacks w/bullet weapons
	int iBulletAttackCount = CalcPlayerAttacks(pFreeman, true);
	return iBulletAttackCount;
#else
	return 0;
#endif
}

const CViewVectors* CHL2MPRules::GetViewVectors()const
{
	return &g_HL2MPViewVectors;
}

const HL2MPViewVectors* CHL2MPRules::GetHL2MPViewVectors()const
{
	return &g_HL2MPViewVectors;
}
	
CHL2MPRules::~CHL2MPRules( void )
{
#ifndef CLIENT_DLL
	// Note, don't delete each team since they are in the gEntList and will 
	// automatically be deleted from there, instead.
	g_Teams.Purge();
#endif
}

void CHL2MPRules::CreateStandardEntities( void )
{

#ifndef CLIENT_DLL
	// Create the entity that will send our data to the client.
	g_pPlayerResource = (CPlayerResource*)CBaseEntity::Create("anticitizen_player_manager", vec3_origin, vec3_angle);

	g_pLastCombineSpawn = NULL;
	g_pLastRebelSpawn = NULL;

#ifdef DBGFLAG_ASSERT
	CBaseEntity *pEnt = 
#endif
	CBaseEntity::Create( "hl2mp_gamerules", vec3_origin, vec3_angle );
	Assert( pEnt );
#endif
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CHL2MPRules::FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( weaponstay.GetInt() > 0 )
	{
		// make sure it's only certain weapons
		if ( !(pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
		{
			return 0;		// weapon respawns almost instantly
		}
	}

	return sv_hl2mp_weapon_respawn_time.GetFloat();
#endif

	return 0;		// weapon respawns almost instantly
}


bool CHL2MPRules::IsIntermission( void )
{
	return m_bIsInIntermission;
}

void CHL2MPRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL
	if ( IsIntermission() )
		return;

	CHL2MP_Player* pHL2MPPlayer = ToHL2MPPlayer(pVictim);
	if (pHL2MPPlayer)
	{
		if (!pHL2MPPlayer->m_bInitialSpawn && pHL2MPPlayer->GetLifeCount() > 0)
		{
			pHL2MPPlayer->SetLifeCount(pHL2MPPlayer->GetLifeCount() - 1);
			DevMsg("LIVES: %i\n", pHL2MPPlayer->GetLifeCount());
		}
	}

	BaseClass::PlayerKilled( pVictim, info );
#endif
}

int CHL2MPRules::GetSpectatorCount(void)
{
#ifndef CLIENT_DLL
	CTeam* pSpectatorTeam = g_Teams[TEAM_SPECTATOR];
	int iSpectators = 0;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

		if (!pPlayer)
			continue;

		if (pPlayer->GetTeam() != pSpectatorTeam)
			continue;

		iSpectators += 1;
	}

	return iSpectators;
#else
	return 0;
#endif
}

int CHL2MPRules::GetSoldierCount(void)
{
#ifndef CLIENT_DLL
	CTeam* pCombine = g_Teams[TEAM_COMBINE];
	int iPlayers = 0;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

		if (!pPlayer)
			continue;

		if (pPlayer->m_bChosenToSpectate)
			continue;

		if (pPlayer->GetTeam() != pCombine)
			continue;

		iPlayers += 1;
	}

	return iPlayers;
#else
	return 0;
#endif
}

#ifndef CLIENT_DLL
extern ConVar sv_disablelives;
#endif

int CHL2MPRules::GetRemainingSoldierCount(void)
{
#ifndef  CLIENT_DLL
	if (sv_disablelives.GetBool())
		return 999;

	if (m_iRoundState != STATE_PLAYING)
		return 0;

	CTeam* pCombine = g_Teams[TEAM_COMBINE];
	int iLives = 0;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

		if (!pPlayer)
			continue;

		if (pPlayer->GetLifeCount() == -1)
			continue;

		if (pPlayer->m_bChosenToSpectate)
			continue;

		if (pPlayer->GetTeam() != pCombine)
			continue;

		if (!pPlayer->m_bInitialSpawn && (pPlayer->GetLifeCount() > 0))
		{
			iLives += pPlayer->GetLifeCount();
		}
	}

	m_iSoldiers = iLives;
#endif

	return m_iSoldiers;
}

void CHL2MPRules::CheckLastMemberLeft(void)
{
#ifndef CLIENT_DLL
	if (m_iRoundState != STATE_PLAYING)
		return;

	if (!HasRoundStarted())
		return;

	if (GetRemainingSoldierCount() == 1 && !m_bLastSquadMemberAnnounced)
	{
		CTeam* pCombine = g_Teams[TEAM_COMBINE];

		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

			if (!pPlayer)
				continue;

			if (!pPlayer->IsAlive())
				continue;

			if (pPlayer->GetTeam() != pCombine)
				continue;

			pPlayer->SpeakSentence("LAST_OF_SQUAD", SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
			pPlayer->ToggleGlow(true);
			Color teamColor = COLOR_RED;
			pPlayer->SetGlowColor(teamColor.r(), teamColor.g(), teamColor.b(), teamColor.a());
			m_bLastSquadMemberAnnounced = true;
		}
	}
#endif
}

void CHL2MPRules::SelectFreeman(void)
{
#ifndef CLIENT_DLL
	if (pFreeman)
		return;

	CHL2MP_Player* pPlayer = NULL;

	if (sv_randomize_freeman_player.GetBool())
	{
		if (pNextPlayerToBecomeFreeman)
		{
			pNextPlayerToBecomeFreeman = NULL;
		}
	}

	if (pNextPlayerToBecomeFreeman)
	{
		if (!pNextPlayerToBecomeFreeman->IsDisconnecting())
		{
			pPlayer = pNextPlayerToBecomeFreeman;
		}
		else
		{
			pNextPlayerToBecomeFreeman = NULL;
			SelectFreeman();
			return;
		}
	}
	else
	{
		int iPlayerCount = UTIL_GetPlayerCount();
		random->SetSeed(gpGlobals->curtime);
		int iRandPlayer = random->RandomInt(1, iPlayerCount);

		pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(iRandPlayer));

		if (pPlayer->m_bChosenToSpectate || pPlayer->IsDisconnecting())
		{
			// reroll.
			SelectFreeman();
			return;
		}
	}

	if (pPlayer)
	{
		if (pPlayer->GetActiveWeapon())
		{
			if (pPlayer->GetActiveWeapon()->IsIronsighted())
			{
				pPlayer->GetActiveWeapon()->DisableIronsights();
			}
		}

		pPlayer->ShowViewPortPanel(PANEL_CLASS, false);
		pPlayer->ResetPlayerClass();
		pPlayer->ChangeTeam(TEAM_FREEMAN);
		pPlayer->SetPlayerClass(CLS_FREEMAN);
		pPlayer->SetChosenClass(true);
		pPlayer->m_bChosenToSpectate = false;

		m_uiFreemanID = pPlayer->GetSteamIDAsUInt64();

		if (m_uiFreemanID == m_uiLastFreemanID)
		{
			m_iNumTimesFreemanIDShowedUpIFuckingHateThis += 1;
		}
		else
		{
			m_iNumTimesFreemanIDShowedUpIFuckingHateThis = 0;
		}

		pFreeman = pPlayer;
	}
#endif
}

void CHL2MPRules::ReassignSpectators(void)
{
#ifndef CLIENT_DLL
	CTeam* pSpec = g_Teams[TEAM_SPECTATOR];

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

		if (!pPlayer)
			continue;

		if (!pPlayer->IsAlive())
			continue;

		if (pPlayer->GetTeam() != pSpec)
			continue;

		if (pPlayer->m_bChosenToSpectate)
			continue;

		pPlayer->ShowViewPortPanel(PANEL_CLASS, false);
		pPlayer->HandleCommand_JoinClass(CLS_RAND, false);
	}
#endif
}

bool CHL2MPRules::IsFreemanAlive(void)
{
#ifdef CLIENT_DLL
	C_HL2MP_Player* pFreeman = GetFreeman();
#endif

	if (!pFreeman)
		return false;

	if (pFreeman)
	{
#ifndef CLIENT_DLL
		if (pFreeman->IsDisconnecting())
		{
			return false;
		}
#endif

		if (!pFreeman->IsAlive())
		{
			return false;
		}

		if (pFreeman->GetLifeCount() == 0)
		{
			return false;
		}

		if (pFreeman->GetTeamNumber() == TEAM_SPECTATOR)
		{
			return false;
		}
	}

	return true;
}

#ifdef CLIENT_DLL
CHL2MP_Player* CHL2MPRules::GetFreeman(void)
{
	C_Team* pFreemanTeam = GetGlobalTeam(TEAM_FREEMAN);

	if (pFreemanTeam)
	{
		// probably overly complex.
		if (pFreemanTeam->GetNumPlayers() > 0)
		{
			for (int i = 0; i < pFreemanTeam->GetNumPlayers(); ++i)
			{
				if (!pFreemanTeam->GetPlayer(i))
					continue;

				return ToHL2MPPlayer(pFreemanTeam->GetPlayer(i));
			}
		}
	}

	return NULL;
}
#else
CHL2MP_Player* CHL2MPRules::GetFreeman(void)
{ 
	return pFreeman; 
}
#endif

int CHL2MPRules::GetFreemanHealth(void) 
{
#ifdef CLIENT_DLL
	C_HL2MP_Player* pFreeman = GetFreeman();
#endif

	if (!pFreeman)
	{
		return -1;
	}

	if (!IsFreemanAlive())
	{
		return -1;
	}

	return pFreeman->GetHealth();
}

int CHL2MPRules::GetFreemanMaxHealth(void)
{
#ifdef CLIENT_DLL
	C_HL2MP_Player* pFreeman = GetFreeman();
#endif

	if (!pFreeman)
	{
		return -1;
	}

	if (!IsFreemanAlive())
	{
		return -1;
	}

	float maxHealth = 100.0f;

	if (pFreeman)
	{
		// we don't need the class check here lmao
		const CAnticitizen_FilePlayerClassInfo_t& pPlayerClassInfo = pFreeman->GetPlayerClassInfo();
		maxHealth = pPlayerClassInfo.iHealth;
	}

	return maxHealth;
}

float CHL2MPRules::GetFreemanHealthFraction(void)
{
#ifdef CLIENT_DLL
	C_HL2MP_Player* pFreeman = GetFreeman();
#endif

	if (!pFreeman)
	{
		return -1;
	}

	if (!IsFreemanAlive())
	{
		return -1;
	}

	float healthPerc = (float)GetFreemanHealth() / (float)GetFreemanMaxHealth();
	healthPerc = clamp(healthPerc, 0.0f, 1.0f);

	return healthPerc;
}

int CHL2MPRules::CheckCanEndGame(void)
{
	// no longer at the player minimum.
	if (!m_bHasMinPlayersToStart)
	{
		return GAME_END_NOTENOUGHPLAYERS;
	}

#ifdef CLIENT_DLL
	C_HL2MP_Player* pFreeman = GetFreeman();
#endif

	// freeman is dead
	if (!IsFreemanAlive() && (pFreeman->GetLifeCount() == 0))
	{
		return GAME_END_FREEMANDEAD;
	}

	// soldiers are dead
	if (GetRemainingSoldierCount() == 0)
	{
		return GAME_END_NOMORESOLDIERS;
	}

	return GAME_NOT_ENDED;
}

void CHL2MPRules::Announce(bool gameend)
{
#ifndef CLIENT_DLL
	if (!gameend)
	{
		if (!m_bAnnouncedGameStart)
		{
			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

				if (!pPlayer)
					continue;

				CSingleUserRecipientFilter user(pPlayer);
				user.UsePredictionRules();
				EmitSound_t params;

				if (pPlayer->GetTeamNumber() == TEAM_COMBINE)
				{
					params.m_pSoundName = "Announcer.RoundStart.Combine";
				}
				else if (pPlayer->GetTeamNumber() == TEAM_FREEMAN)
				{
					params.m_pSoundName = "Announcer.RoundStart.Freeman";
				}

				pPlayer->EmitSound(user, pPlayer->entindex(), params);
			}

			m_bAnnouncedGameStart = true;
		}
	}
	else
	{
		if (!m_bAnnouncedGameEnd)
		{
			bool bFreemanLost = false;
			bool bCombineLost = false;

			switch (m_iGameEndReason)
			{
				case GAME_END_FREEMANDEAD:
				{
					bFreemanLost = true;
					break;
				}
				case GAME_END_NOMORESOLDIERS:
				{
					bCombineLost = true;
					break;
				}
			}

			for (int i = 0; i < MAX_PLAYERS; i++)
			{
				CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

				if (!pPlayer)
					continue;

				CSingleUserRecipientFilter user(pPlayer);
				user.UsePredictionRules();
				EmitSound_t params;

				if (pPlayer->GetTeamNumber() == TEAM_COMBINE)
				{
					if (bCombineLost)
					{
						params.m_pSoundName = "Announcer.RoundEnd.Lose.Combine";
					}
					else
					{
						params.m_pSoundName = "Announcer.RoundEnd.Win.Combine";
					}
				}
				else if (pPlayer->GetTeamNumber() == TEAM_FREEMAN)
				{
					if (bFreemanLost)
					{
						params.m_pSoundName = "Announcer.RoundEnd.Lose.Freeman";
					}
					else
					{
						params.m_pSoundName = "Announcer.RoundEnd.Win.Freeman";
					}
				}

				pPlayer->EmitSound(user, pPlayer->entindex(), params);
			}
		
			m_bAnnouncedGameEnd = true;
		}
	}
#endif
}

#ifndef CLIENT_DLL
void CHL2MPRules::SetNextPlayerToBecomeFreeman(CHL2MP_Player* pPlayer)
{
	if (!sv_randomize_freeman_player.GetBool())
	{
		pNextPlayerToBecomeFreeman = pPlayer;
	}
}
#endif

void CHL2MPRules::AwardGameEndAchievements()
{
#ifndef CLIENT_DLL
	if (!m_bGaveGameEndAchievements)
	{
		bool bFreemanLost = false;
		bool bCombineLost = false;

		switch (m_iGameEndReason)
		{
			case GAME_END_FREEMANDEAD:
			{
				bFreemanLost = true;
				break;
			}
			case GAME_END_NOMORESOLDIERS:
			{
				bCombineLost = true;
				break;
			}
		}

		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(i));

			if (!pPlayer)
				continue;

			if (pPlayer->GetTeamNumber() == TEAM_COMBINE)
			{
				if (!bCombineLost)
				{
					pPlayer->AwardAchievement(ACHIEVEMENT_ANTICITIZEN_KILL_FREEMAN);

					if (HL2MPRules()->GetTimeSinceGameStart() < 60.0f)
					{
						pPlayer->AwardAchievement(ACHIEVEMENT_ANTICITIZEN_KILL_FREEMAN_LESSTIME);
					}
				}
				else
				{
					continue;
				}
			}
			else if (pPlayer->GetTeamNumber() == TEAM_FREEMAN)
			{
				if (!bFreemanLost)
				{
					pPlayer->AwardAchievement(ACHIEVEMENT_ANTICITIZEN_KILL_COMBINE);

					int bulletsShot = GetFreemanBulletsShot();
					if (bulletsShot == 0)
					{
						pPlayer->AwardAchievement(ACHIEVEMENT_ANTICITIZEN_KILL_COMBINE_GRAVITYGUN);
					}
				}
				else
				{
					continue;
				}
			}
		}

		m_bGaveGameEndAchievements = true;
	}
#endif
}

void CHL2MPRules::Think( void )
{

#ifndef CLIENT_DLL
	
	CGameRules::Think();

	// set every tick for player management.
	m_bHasMinPlayersToStart = (UTIL_GetPlayerCount() >= sv_minplayerstostart.GetInt());

	switch (m_iRoundState)
	{
		case STATE_PREROUND:
		{
			m_bCompleteReset = false;

			if (m_bJustEnded)
			{
				RestartGame(true);
				m_bJustEnded = false;
			}

			LeaveIntermission();

			if (m_bHasMinPlayersToStart)
			{
				if (!m_bStartedStartClock)
				{
					m_flGameStartTime = gpGlobals->curtime + sv_startwaitime.GetInt();
					m_bStartedStartClock = true;
				}

				if (m_bStartedStartClock && (m_flGameStartTime < gpGlobals->curtime))
				{
					m_iRoundState = STATE_PLAYING;
					// add a delay that is overrided by the below code.
					// This is to fix a bug where CheckLastMemberLeft() is called too early.
					m_flGameStartTime = gpGlobals->curtime + 0.15f;

					if (!pFreeman)
					{
						SelectFreeman();
					}

					if (!m_bReassignSpectators)
					{
						ReassignSpectators();
						m_bReassignSpectators = true;
					}

					if (!m_bCompleteReset)
					{
						RestartGame();
						m_bCompleteReset = true;
					}

					IGameEvent* event = gameeventmanager->CreateEvent("anticitizen_round_start");
					if (event)
					{
						gameeventmanager->FireEvent(event);
					}

					m_flGameStartTime = gpGlobals->curtime + sv_startplaywaitime.GetInt();
					m_flTimeSinceGameStart = gpGlobals->curtime;
				}
				else
				{
					SendHudMessage(NULL, "#Anticitizen_RoundStarting", 0.5f);
					m_iTimerType = TIMERSTATE_ROUNDSTART;
				}
			}
			else
			{
				SendHudMessage(NULL, "#Anticitizen_WaitingforPlayers", 0.5f);
				m_flGameStartTime = -1;
				m_iTimerType = TIMERSTATE_NONE;
				m_bStartedStartClock = false;
			}

			break;
		}

		case STATE_PLAYING:
		{
			if (HasRoundStarted())
			{
				if (!m_bStrippedFlags)
				{
					for (int i = 0; i < MAX_PLAYERS; i++)
					{
						CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

						if (!pPlayer)
							continue;

						if (pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
							continue;

						pPlayer->RemoveFlag(FL_FROZEN);
						pPlayer->RemoveFlag(FL_GODMODE);
						pPlayer->RemoveFlag(FL_NOTARGET);
					}

					m_bStrippedFlags = true;
				}

				Announce();
				CheckLastMemberLeft();

				m_iGameEndReason = CheckCanEndGame();

				if (!g_fGameOver && (m_iGameEndReason > GAME_NOT_ENDED))
				{
					m_iRoundState = STATE_COMPLETION;
					m_iCurrentRound++;
					GoToIntermission();
				}
			}
			else
			{
				SendHudMessage(NULL, "#Anticitizen_GameStarting", 0.5f);
				m_iTimerType = TIMERSTATE_GAMESTART;

				for (int i = 0; i < MAX_PLAYERS; i++)
				{
					CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

					if (!pPlayer)
						continue;

					if (pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
						continue;

					pPlayer->AddFlag(FL_FROZEN);
					pPlayer->AddFlag(FL_GODMODE);
					pPlayer->AddFlag(FL_NOTARGET);
					//SHUT.
					pPlayer->SentenceStop();
				}

				m_bStrippedFlags = false;
			}

			break;
		}

		case STATE_COMPLETION:
		{
			if (g_fGameOver)
			{
				if (m_flIntermissionEndTime < gpGlobals->curtime)
				{
					m_bJustEnded = true;

					if ((!m_bChangelevelDone) && (m_iCurrentRound > sv_roundlimit.GetInt()))
					{
						ChangeLevel(); // intermission is over
						m_bChangelevelDone = true;
					}
					else
					{
						m_iRoundState = STATE_PREROUND;
					}
				}
				else
				{
					const char* szPhrase = "";

					switch (m_iGameEndReason)
					{
						case GAME_END_NOTENOUGHPLAYERS:
							szPhrase = "#Anticitizen_NotEnoughPlayers";
							break;
						case GAME_END_FREEMANDEAD:
							szPhrase = "#Anticitizen_FreemanDead";
							break;
						case GAME_END_NOMORESOLDIERS:
							szPhrase = "#Anticitizen_SoldiersDead";
							break;
					}

					if (pFreeman && !pFreeman->IsDisconnecting())
					{
						m_uiLastFreemanID = pFreeman->GetSteamIDAsUInt64();

						// the freeman is wise enough that we may choose him again.
						// hopefully he will not abuse this power.
						if (m_iGameEndReason == GAME_END_NOMORESOLDIERS)
						{
							CSteamID id;
							pFreeman->GetSteamID(&id);
							SetNextPlayerToBecomeFreeman(ToHL2MPPlayer(UTIL_PlayerBySteamID(id)));
						}
					}

					int iWinningTeam = -1;

					if (m_iGameEndReason == GAME_END_NOMORESOLDIERS)
					{
						iWinningTeam = TEAM_FREEMAN;
					}
					else if (m_iGameEndReason == GAME_END_FREEMANDEAD)
					{
						iWinningTeam = TEAM_COMBINE;
					}

					if (!m_bSentGameEndEvent)
					{
						CTeam* pWinningTeam = GetGlobalTeam(iWinningTeam);

						if (pWinningTeam)
						{
							pWinningTeam->SetRoundsWon(pWinningTeam->GetRoundsWon() + 1);
						}

						IGameEvent* event = gameeventmanager->CreateEvent("anticitizen_round_end");
						if (event)
						{
							event->SetInt("team", iWinningTeam);
							event->SetInt("winreason", m_iGameEndReason);
							gameeventmanager->FireEvent(event);
						}

						m_bSentGameEndEvent = true;
					}

					Announce(true);
					AwardGameEndAchievements();

					SendHudMessage(NULL, szPhrase, 0.5f);
				}
			}

			break;
		}
	}

	ManageObjectRelocation();

#endif
}

void CHL2MPRules::GoToIntermission(void)
{
#ifndef CLIENT_DLL
	if (g_fGameOver)
		return;

	g_fGameOver = true;

	m_flGameEndTime = m_flIntermissionEndTime = gpGlobals->curtime + mp_chattime.GetInt();
	if (m_iCurrentRound > sv_roundlimit.GetInt())
	{
		m_iTimerType = TIMERSTATE_CHANGELEVEL;
	}
	else
	{
		m_iTimerType = TIMERSTATE_RESTART;
	}

	if (m_iCurrentRound == sv_roundlimit.GetInt())
	{
		m_bFinalRound = true;
	}
	else
	{
		m_bFinalRound = false;
	}

	m_bIsInIntermission = true;
	m_flTimeSinceGameStart = 0;

	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		if ( !pPlayer )
			continue;

		pPlayer->ShowViewPortPanel(PANEL_SCOREBOARD);
		pPlayer->AddFlag( FL_FROZEN );
		pPlayer->AddFlag(FL_GODMODE);
		pPlayer->AddFlag(FL_NOTARGET);
		pPlayer->ToggleGlow(false);
		//SHUT.
		pPlayer->SentenceStop();
	}

	m_bStrippedFlags = false;
#endif
	
}

void CHL2MPRules::LeaveIntermission(void)
{
#ifndef CLIENT_DLL
	if (!g_fGameOver)
		return;

	g_fGameOver = false;

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		if (!pPlayer)
			continue;

		pPlayer->ShowViewPortPanel(PANEL_SCOREBOARD, false);
		pPlayer->RemoveFlag(FL_FROZEN);
		pPlayer->RemoveFlag(FL_GODMODE);
		pPlayer->RemoveFlag(FL_NOTARGET);
		pPlayer->ToggleGlow(false);
		//SHUT.
		pPlayer->SentenceStop();
	}
#endif

}

// when we are within this close to running out of entities,  items 
// marked with the ITEM_FLAG_LIMITINWORLD will delay their respawn
#define ENTITY_INTOLERANCE	100

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CHL2MPRules::FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( pWeapon && (pWeapon->GetWeaponFlags() & ITEM_FLAG_LIMITINWORLD) )
	{
		if ( gEntList.NumberOfEntities() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE) )
			return 0;

		// we're past the entity tolerance level,  so delay the respawn
		return FlWeaponRespawnTime( pWeapon );
	}
#endif
	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHL2MPRules::VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	CWeaponHL2MPBase *pHL2Weapon = dynamic_cast< CWeaponHL2MPBase*>( pWeapon );

	if ( pHL2Weapon )
	{
		return pHL2Weapon->GetOriginalSpawnOrigin();
	}
#endif
	
	return pWeapon->GetAbsOrigin();
}

#ifndef CLIENT_DLL

CItem* IsManagedObjectAnItem( CBaseEntity *pObject )
{
	return dynamic_cast< CItem*>( pObject );
}

CWeaponHL2MPBase* IsManagedObjectAWeapon( CBaseEntity *pObject )
{
	return dynamic_cast< CWeaponHL2MPBase*>( pObject );
}

bool GetObjectsOriginalParameters( CBaseEntity *pObject, Vector &vOriginalOrigin, QAngle &vOriginalAngles )
{
	if ( CItem *pItem = IsManagedObjectAnItem( pObject ) )
	{
		if ( pItem->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;
		
		vOriginalOrigin = pItem->GetOriginalSpawnOrigin();
		vOriginalAngles = pItem->GetOriginalSpawnAngles();

		pItem->m_flNextResetCheckTime = gpGlobals->curtime + sv_hl2mp_item_respawn_time.GetFloat();
		return true;
	}
	else if ( CWeaponHL2MPBase *pWeapon = IsManagedObjectAWeapon( pObject )) 
	{
		if ( pWeapon->m_flNextResetCheckTime > gpGlobals->curtime )
			 return false;

		vOriginalOrigin = pWeapon->GetOriginalSpawnOrigin();
		vOriginalAngles = pWeapon->GetOriginalSpawnAngles();

		pWeapon->m_flNextResetCheckTime = gpGlobals->curtime + sv_hl2mp_weapon_respawn_time.GetFloat();
		return true;
	}

	return false;
}

void CHL2MPRules::ManageObjectRelocation( void )
{
	int iTotal = m_hRespawnableItemsAndWeapons.Count();

	if ( iTotal > 0 )
	{
		for ( int i = 0; i < iTotal; i++ )
		{
			CBaseEntity *pObject = m_hRespawnableItemsAndWeapons[i].Get();
			
			if ( pObject )
			{
				Vector vSpawOrigin;
				QAngle vSpawnAngles;

				if ( GetObjectsOriginalParameters( pObject, vSpawOrigin, vSpawnAngles ) == true )
				{
					float flDistanceFromSpawn = (pObject->GetAbsOrigin() - vSpawOrigin ).Length();

					if ( flDistanceFromSpawn > WEAPON_MAX_DISTANCE_FROM_SPAWN )
					{
						bool shouldReset = false;
						IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();

						if ( pPhysics )
						{
							shouldReset = pPhysics->IsAsleep();
						}
						else
						{
							shouldReset = (pObject->GetFlags() & FL_ONGROUND) ? true : false;
						}

						if ( shouldReset )
						{
							pObject->Teleport( &vSpawOrigin, &vSpawnAngles, NULL );
							pObject->EmitSound( "AlyxEmp.Charge" );

							IPhysicsObject *pPhys = pObject->VPhysicsGetObject();

							if ( pPhys )
							{
								pPhys->Wake();
							}
						}
					}
				}
			}
		}
	}
}

//=========================================================
//AddLevelDesignerPlacedWeapon
//=========================================================
void CHL2MPRules::AddLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) == -1 )
	{
		m_hRespawnableItemsAndWeapons.AddToTail( pEntity );
	}
}

//=========================================================
//RemoveLevelDesignerPlacedWeapon
//=========================================================
void CHL2MPRules::RemoveLevelDesignerPlacedObject( CBaseEntity *pEntity )
{
	if ( m_hRespawnableItemsAndWeapons.Find( pEntity ) != -1 )
	{
		m_hRespawnableItemsAndWeapons.FindAndRemove( pEntity );
	}
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHL2MPRules::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->GetOriginalSpawnOrigin();
}

//=========================================================
// What angles should this item use to respawn?
//=========================================================
QAngle CHL2MPRules::VecItemRespawnAngles( CItem *pItem )
{
	return pItem->GetOriginalSpawnAngles();
}

//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CHL2MPRules::FlItemRespawnTime( CItem *pItem )
{
	return sv_hl2mp_item_respawn_time.GetFloat();
}

//=========================================================
// CanHaveWeapon - returns false if the player is not allowed
// to pick up this weapon
//=========================================================
bool CHL2MPRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem )
{
	if ( weaponstay.GetInt() > 0 )
	{
		if ( pPlayer->Weapon_OwnsThisType( pItem->GetClassname(), pItem->GetSubType() ) )
			 return false;
	}

	return BaseClass::CanHavePlayerItem( pPlayer, pItem );
}

typedef bool (*BIgnoreConvarChangeFunc)(void);

struct convar_tags_t
{
	const char* pszConVar;
	const char* pszTag;
	BIgnoreConvarChangeFunc ignoreConvarFunc;
};

// The list of convars that automatically turn on tags when they're changed.
// Convars in this list need to have the FCVAR_NOTIFY flag set on them, so the
// tags are recalculated and uploaded to the master server when the convar is changed.
convar_tags_t convars_to_check_for_tags[] =
{
	{ "mp_friendlyfire", "friendlyfire", NULL },
	{ "mp_respawnwavetime", "respawntimes", NULL },
	{ "mp_fadetoblack", "fadetoblack", NULL },
	{ "mp_disable_respawn_times", "norespawntime", NULL },
	{ "hl2mp_bot_count", "bots", NULL },
	{ "sv_randomize_freeman_player", "randomizedfreeman", NULL}
};

//-----------------------------------------------------------------------------
// Purpose: Engine asks for the list of convars that should tag the server
//-----------------------------------------------------------------------------
void CHL2MPRules::GetTaggedConVarList(KeyValues* pCvarTagList)
{
	BaseClass::GetTaggedConVarList(pCvarTagList);

	for (int i = 0; i < ARRAYSIZE(convars_to_check_for_tags); i++)
	{
		if (convars_to_check_for_tags[i].ignoreConvarFunc && convars_to_check_for_tags[i].ignoreConvarFunc())
			continue;

		KeyValues* pKV = new KeyValues("tag");
		pKV->SetString("convar", convars_to_check_for_tags[i].pszConVar);
		pKV->SetString("tag", convars_to_check_for_tags[i].pszTag);

		pCvarTagList->AddSubKey(pKV);
	}
}

#endif

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CHL2MPRules::WeaponShouldRespawn( CBaseCombatWeapon *pWeapon )
{
#ifndef CLIENT_DLL
	if ( pWeapon->HasSpawnFlags( SF_NORESPAWN ) )
	{
		return GR_WEAPON_RESPAWN_NO;
	}
#endif

	return GR_WEAPON_RESPAWN_YES;
}

//-----------------------------------------------------------------------------
// Purpose: Player has just left the game
//-----------------------------------------------------------------------------
void CHL2MPRules::ClientDisconnected( edict_t *pClient )
{
#ifndef CLIENT_DLL
	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );
	if (pPlayer)
	{
		pPlayer->SetConnected(PlayerDisconnecting);

		// Remove the player from his team
		if (pPlayer->GetTeam())
		{
			pPlayer->GetTeam()->RemovePlayer(pPlayer);
		}

		if (pPlayer == pNextPlayerToBecomeFreeman)
		{
			pNextPlayerToBecomeFreeman = NULL;
		}

		if (pPlayer == pFreeman)
		{
			pFreeman = NULL;
		}
	}

	BaseClass::ClientDisconnected(pClient);
#endif
}


//=========================================================
// Deathnotice. 
//=========================================================
void CHL2MPRules::DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
#ifndef CLIENT_DLL
	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_ID = 0;

	// Find the killer & the scorer
	CBaseEntity *pInflictor = info.GetInflictor();
	CBaseEntity *pKiller = info.GetAttacker();
	CBasePlayer *pScorer = GetDeathScorer( pKiller, pInflictor );

	if (!sv_friendlyfire_deathnotice.GetBool() && pScorer && pVictim && (pScorer->GetTeamNumber() == pVictim->GetTeamNumber()) && (pScorer != pVictim) )
		return;

	// Custom kill type?
	if ( info.GetDamageCustom() )
	{
		killer_weapon_name = GetDamageCustomString( info );
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
		}
	}
	else
	{
		// Is the killer a client?
		if ( pScorer )
		{
			killer_ID = pScorer->GetUserID();
			
			if ( pInflictor )
			{
				if ( pInflictor == pScorer )
				{
					// If the inflictor is the killer,  then it must be their current weapon doing the damage
					if ( pScorer->GetActiveWeapon() )
					{
						killer_weapon_name = pScorer->GetActiveWeapon()->GetClassname();
					}
				}
				else
				{
					killer_weapon_name = pInflictor->GetClassname();  // it's just that easy
				}
			}
		}
		else
		{
			killer_weapon_name = pInflictor->GetClassname();
		}

		// strip the NPC_* or weapon_* from the inflictor's classname
		if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
		{
			killer_weapon_name += 7;
		}
		else if ( strncmp( killer_weapon_name, "npc_", 4 ) == 0 )
		{
			killer_weapon_name += 4;
		}
		else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
		{
			killer_weapon_name += 5;
		}
		else if ( strstr( killer_weapon_name, "physics" ) )
		{
			killer_weapon_name = "physics";
		}

		if ( strcmp( killer_weapon_name, "prop_combine_ball" ) == 0 )
		{
			killer_weapon_name = "combine_ball";
		}
		else if ( strcmp( killer_weapon_name, "grenade_ar2" ) == 0 )
		{
			killer_weapon_name = "smg1_grenade";
		}
		/*else if (strcmp(killer_weapon_name, "satchel") == 0 || strcmp(killer_weapon_name, "tripmine") == 0)
		{
			killer_weapon_name = "slam";
		}*/
	}

	IGameEvent *event = gameeventmanager->CreateEvent( "player_death" );
	if( event )
	{
		event->SetInt("userid", pVictim->GetUserID() );
		event->SetInt("attacker", killer_ID );
		event->SetString("weapon", killer_weapon_name );
		event->SetInt( "priority", 7 );
		gameeventmanager->FireEvent( event );
	}
#endif

}

void CHL2MPRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	
	CHL2MP_Player *pHL2Player = ToHL2MPPlayer( pPlayer );

	if ( pHL2Player == NULL )
		return;

	if ( sv_report_client_settings.GetInt() == 1 )
	{
		UTIL_LogPrintf( "\"%s\" cl_cmdrate = \"%s\"\n", pHL2Player->GetPlayerName(), engine->GetClientConVarValue( pHL2Player->entindex(), "cl_cmdrate" ));
	}

	const char* pszFov = engine->GetClientConVarValue(pPlayer->entindex(), "fov_desired");
	int iFov = atoi(pszFov);
	iFov = clamp(iFov, 75, MAX_FOV);

	pPlayer->SetDefaultFOV(iFov);

	BaseClass::ClientSettingsChanged( pPlayer );
#endif
	
}

int CHL2MPRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
#ifndef CLIENT_DLL
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() )
		return GR_NOTTEAMMATE;

	// Check for an ai_relationship override
	if (Disposition_t nRel = pPlayer->MyCombatCharacterPointer()->IRelationType(pTarget))
	{
		if (nRel == D_HT || nRel == D_FR)
			return GR_NOTTEAMMATE;
		else if (nRel == D_LI)
			return GR_TEAMMATE;
	}

	if ( (*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp( GetTeamID(pPlayer), GetTeamID(pTarget) ) )
	{
		return GR_TEAMMATE;
	}
#endif

	return GR_NOTTEAMMATE;
}

const char *CHL2MPRules::GetGameDescription( void )
{ 
	return "ANTICITIZEN ONE";
} 

bool CHL2MPRules::IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer )
{
	return true;
}

float CHL2MPRules::GetMapRemainingTime()
{
	float timeleft = 0.0f;
	// timelimit is in minutes

	if (IsIntermission())
	{
		// if timelimit is disabled, return 0
		if (m_flGameEndTime <= 0)
			return 0;

		timeleft = (m_flGameEndTime - gpGlobals->curtime);
	}
	else
	{
		// if timelimit is disabled, return 0
		if (m_flGameStartTime <= 0)
			return 0;

		timeleft = (m_flGameStartTime - gpGlobals->curtime);
	}

	return timeleft;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MPRules::Precache( void )
{
	CBaseEntity::PrecacheScriptSound( "AlyxEmp.Charge" );

	CBaseEntity::PrecacheScriptSound("Announcer.RoundStart.Freeman");
	CBaseEntity::PrecacheScriptSound("Announcer.RoundStart.Combine");
	CBaseEntity::PrecacheScriptSound("Announcer.RoundEnd.Lose.Freeman");
	CBaseEntity::PrecacheScriptSound("Announcer.RoundEnd.Win.Freeman");
	CBaseEntity::PrecacheScriptSound("Announcer.RoundEnd.Lose.Combine");
	CBaseEntity::PrecacheScriptSound("Announcer.RoundEnd.Win.Combine");
}

#ifdef GAME_DLL
bool CHL2MPRules::IsOfficialMap( void )
{ 
	static const char *s_OfficialMaps[] =
	{
		// ANTICITIZEN ONE
		"hunt_prison",
		"hunt_lockdown",
		"hunt_lighthouse",

		// hl2mp
		//"devtest",
		//"dm_lockdown",
		"dm_overwatch",
		"dm_powerhouse",
		"dm_resistance",
		"dm_runoff",
		"dm_steamlab",
		"dm_underpass",
		"halls3",
	};

	char szCurrentMap[MAX_MAP_NAME];
	Q_strncpy( szCurrentMap, STRING( gpGlobals->mapname ), sizeof( szCurrentMap ) );

	for ( int i = 0; i < ARRAYSIZE( s_OfficialMaps ); ++i )
	{
		if ( !Q_stricmp( s_OfficialMaps[i], szCurrentMap ) )
		{
			return true;
		}
	}

	return BaseClass::IsOfficialMap();
}
#endif

bool CHL2MPRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		V_swap(collisionGroup0,collisionGroup1);
	}

	if ( (collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT) &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 

}

bool CHL2MPRules::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
{
#ifndef CLIENT_DLL
	if( BaseClass::ClientCommand( pEdict, args ) )
		return true;


	CHL2MP_Player *pPlayer = (CHL2MP_Player *) pEdict;

	if ( pPlayer->ClientCommand( args ) )
		return true;
#endif

	return false;
}

#ifdef CLIENT_DLL

	ConVar cl_autowepswitch(
		"cl_autowepswitch",
		"1",
		FCVAR_ARCHIVE | FCVAR_USERINFO,
		"Automatically switch to picked up weapons (if more powerful)" );

#else

	bool CHL2MPRules::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon )
	{		
		if ( pPlayer->GetActiveWeapon() && pPlayer->IsNetClient() )
		{
			// Player has an active item, so let's check cl_autowepswitch.
			const char *cl_autowepswitch = engine->GetClientConVarValue( engine->IndexOfEdict( pPlayer->edict() ), "cl_autowepswitch" );
			if ( cl_autowepswitch && atoi( cl_autowepswitch ) <= 0 )
			{
				return false;
			}
		}

		return BaseClass::FShouldSwitchWeapon( pPlayer, pWeapon );
	}

#endif

#ifndef CLIENT_DLL

void CHL2MPRules::RestartGame(bool gameend)
{
	CleanUpMap();
	
	// now respawn all players
	for (int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*) UTIL_PlayerByIndex( i );

		if ( !pPlayer )
			continue;

		pPlayer->Reset(gameend);
		pPlayer->Spawn();
	}

	// Respawn entities (glass, doors, etc..)

	CTeam *pRebels = GetGlobalTeam( TEAM_FREEMAN );
	CTeam *pCombine = GetGlobalTeam( TEAM_COMBINE );

	if ( pRebels )
	{
		pRebels->SetScore( 0 );
	}

	if ( pCombine )
	{
		pCombine->SetScore( 0 );
	}

	if (gameend)
	{
		pFreeman = NULL;
		m_uiFreemanID = 0;

		int freemanroundlimit = sv_freemanroundlimit.GetInt();
		if ((freemanroundlimit > 0) && (m_iNumTimesFreemanIDShowedUpIFuckingHateThis >= freemanroundlimit))
		{
			pNextPlayerToBecomeFreeman = NULL;
			m_iNumTimesFreemanIDShowedUpIFuckingHateThis = 0;
		}

		m_iGameEndReason = GAME_NOT_ENDED;
		m_bReassignSpectators = false;

		m_bAnnouncedGameStart = false;
		m_bAnnouncedGameEnd = false;
		m_bGaveGameEndAchievements = false;
		m_bSentGameEndEvent = false;
	}

	m_flTimeSinceGameStart = 0;
	m_flIntermissionEndTime = 0;
	m_bIsInIntermission = false;
	m_flGameEndTime = 0;
	m_bHasMinPlayersToStart = false;
	m_iTimerType = TIMERSTATE_NONE;
	if (gameend)
	{
		m_bLastSquadMemberAnnounced = false;
		m_bStartedStartClock = false;
		m_iSoldiers = 0;
	}
	else
	{
		m_bStartedStartClock = true;
	}
	m_flGameStartTime = 0;

	IGameEvent * event = gameeventmanager->CreateEvent( "round_start" );
	if ( event )
	{
		event->SetInt("fraglimit", 0 );
		event->SetInt( "priority", 6 ); // HLTV event priority, not transmitted

		event->SetString("objective","DEATHMATCH");

		gameeventmanager->FireEvent( event );
	}
}

#ifdef GAME_DLL
void CHL2MPRules::SendHudMessage(CBasePlayer* pToPlayer, const char* text, float flDuration)
{
	SendHudMessage(pToPlayer, MAKE_STRING(text), flDuration);
}

void CHL2MPRules::SendHudMessage(CBasePlayer* pToPlayer, string_t text, float flDuration)
{
	CRecipientFilter filter;

	if (pToPlayer)
	{
		filter.AddRecipient(pToPlayer);
	}
	else
	{
		filter.AddAllPlayers();
	}

	filter.MakeReliable();

	// Start the message block
	UserMessageBegin(filter, "GameMessage");
		// Send our text to the client
		WRITE_STRING(STRING(text));
		WRITE_FLOAT(flDuration);
	// End the message block
	MessageEnd();
}

void CHL2MPRules::OnNavMeshLoad( void )
{
	TheNavMesh->SetPlayerSpawnName( "info_player_deathmatch" );
}

//=========================================================
//=========================================================
void CHL2MPRules::PlayerSpawn(CBasePlayer* pPlayer)
{
	BaseClass::PlayerSpawn(pPlayer);

	CheckLastMemberLeft();
}
#endif

void CHL2MPRules::CleanUpMap()
{
	// Recreate all the map entities from the map data (preserving their indices),
	// then remove everything else except the players.

	// Get rid of all entities except players.
	CBaseEntity *pCur = gEntList.FirstEnt();
	while ( pCur )
	{
		CBaseHL2MPCombatWeapon *pWeapon = dynamic_cast< CBaseHL2MPCombatWeapon* >( pCur );
		// Weapons with owners don't want to be removed..
		if ( pWeapon )
		{
			if ( !pWeapon->GetPlayerOwner() )
			{
				UTIL_Remove( pCur );
			}
		}
		// remove entities that has to be restored on roundrestart (breakables etc)
		else if ( !FindInList( s_PreserveEnts, pCur->GetClassname() ) )
		{
			UTIL_Remove( pCur );
		}

		pCur = gEntList.NextEnt( pCur );
	}

	// Really remove the entities so we can have access to their slots below.
	gEntList.CleanupDeleteList();

	// Cancel all queued events, in case a func_bomb_target fired some delayed outputs that
	// could kill respawning CTs
	g_EventQueue.Clear();

	// Now reload the map entities.
	class CHL2MPMapEntityFilter : public IMapEntityFilter
	{
	public:
		virtual bool ShouldCreateEntity( const char *pClassname )
		{
			// Don't recreate the preserved entities.
			if ( !FindInList( s_PreserveEnts, pClassname ) )
			{
				return true;
			}
			else
			{
				// Increment our iterator since it's not going to call CreateNextEntity for this ent.
				if ( m_iIterator != g_MapEntityRefs.InvalidIndex() )
					m_iIterator = g_MapEntityRefs.Next( m_iIterator );

				return false;
			}
		}


		virtual CBaseEntity* CreateNextEntity( const char *pClassname )
		{
			if ( m_iIterator == g_MapEntityRefs.InvalidIndex() )
			{
				// This shouldn't be possible. When we loaded the map, it should have used 
				// CCSMapLoadEntityFilter, which should have built the g_MapEntityRefs list
				// with the same list of entities we're referring to here.
				Assert( false );
				return NULL;
			}
			else
			{
				CMapEntityRef &ref = g_MapEntityRefs[m_iIterator];
				m_iIterator = g_MapEntityRefs.Next( m_iIterator );	// Seek to the next entity.

				if ( ref.m_iEdict == -1 || engine->PEntityOfEntIndex( ref.m_iEdict ) )
				{
					// Doh! The entity was delete and its slot was reused.
					// Just use any old edict slot. This case sucks because we lose the baseline.
					return CreateEntityByName( pClassname );
				}
				else
				{
					// Cool, the slot where this entity was is free again (most likely, the entity was 
					// freed above). Now create an entity with this specific index.
					return CreateEntityByName( pClassname, ref.m_iEdict );
				}
			}
		}

	public:
		int m_iIterator; // Iterator into g_MapEntityRefs.
	};
	CHL2MPMapEntityFilter filter;
	filter.m_iIterator = g_MapEntityRefs.Head();

	// DO NOT CALL SPAWN ON info_node ENTITIES!

	MapEntity_ParseAllEntities( engine->GetMapEntitiesString(), &filter, true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CHL2MPRules::GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer )
{
	if ( !pPlayer )  // dedicated server output
	{
		return NULL;
	}

	const char *pszFormat = NULL;

	// team only
	if ( bTeamOnly == TRUE )
	{
		if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
		{
			pszFormat = "HL2MP_Chat_Spec";
		}
		else
		{
			const char *chatLocation = GetChatLocation( bTeamOnly, pPlayer );
			if ( chatLocation && *chatLocation )
			{
				pszFormat = "HL2MP_Chat_Team_Loc";
			}
			else
			{
				pszFormat = "HL2MP_Chat_Team";
			}
		}
	}
	// everyone
	else
	{
		if ( pPlayer->GetTeamNumber() != TEAM_SPECTATOR )
		{
			pszFormat = "HL2MP_Chat_All";	
		}
		else
		{
			pszFormat = "HL2MP_Chat_AllSpec";
		}
	}

	return pszFormat;
}

void CC_ForceFreeman(const CCommand& args)
{
	// Listenserver host or rcon access only!
	if (!UTIL_IsCommandIssuedByServerAdmin())
		return;

	if (args.ArgC() < 2)
	{
		DevMsg("%s <player name>\n", args.Arg(0));
		return;
	}

	const char* pPlayerName = args.Arg(1);

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CHL2MP_Player* player = ToHL2MPPlayer((UTIL_PlayerByIndex(i)));

		if (!player)
			continue;

		if (FNullEnt(player->edict()))
			continue;

		if (FStrEq(pPlayerName, player->GetPlayerName()))
		{
			HL2MPRules()->SetNextPlayerToBecomeFreeman(player);
			Msg("%s will become Gordon Freeman on the next round.\n", player->GetPlayerName());
			return;
		}
	}
}
static ConCommand forcefreeman("forcefreeman", CC_ForceFreeman, "Make the specified player become Gordon Freeman on the next round.\n", FCVAR_GAMEDLL | FCVAR_CHEAT);

#define HL2MP_GAMERULES_SCRIPT_FUNC( function, desc ) \
	ScriptRegisterFunctionNamed( g_pScriptVM, Script##function, #function, desc )

int		ScriptGetRoundState() { return HL2MPRules()->GetState(); }
int		ScriptGetEndGameReason() { return HL2MPRules()->GetEndGameReason(); }
bool	ScriptIsInIntermission() { return HL2MPRules()->IsIntermission(); }
bool	ScriptHasEnded() { return HL2MPRules()->HasEnded(); }
int		ScriptSoldierCount() { return HL2MPRules()->GetRemainingSoldierCount(); }
float	ScriptGetTime() { return HL2MPRules()->GetTimeSinceGameStart(); }
float	ScriptGetWaitTime() { return HL2MPRules()->GetMapRemainingTime(); }
HSCRIPT ScriptGetFreeman() { return ToHScript(HL2MPRules()->GetFreeman()); }
void ScriptSetNextPlayerToBecomeFreeman(HSCRIPT pPlayer) 
{ 
	CHL2MP_Player* pHL2MPPlayer = ScriptToEntClass< CHL2MP_Player >(pPlayer);

	if (!pHL2MPPlayer)
		return;

	return HL2MPRules()->SetNextPlayerToBecomeFreeman(pHL2MPPlayer);
}
int		ScriptGetRoundCount() { return HL2MPRules()->GetRoundCount(); }
void ScriptAddLevelDesignerPlacedObject(HSCRIPT pEntity)
{
	CBaseEntity* pEnt = ScriptToEntClass< CBaseEntity >(pEntity);

	if (!pEnt)
		return;

	return HL2MPRules()->AddLevelDesignerPlacedObject(pEnt);
}
void ScriptRemoveLevelDesignerPlacedObject(HSCRIPT pEntity)
{
	CBaseEntity* pEnt = ScriptToEntClass< CBaseEntity >(pEntity);

	if (!pEnt)
		return;

	return HL2MPRules()->RemoveLevelDesignerPlacedObject(pEnt);
}

void CHL2MPRules::RegisterScriptFunctions()
{
	HL2MP_GAMERULES_SCRIPT_FUNC(GetRoundState, "Get current round state. See Constants.EAC1RoundState");
	HL2MP_GAMERULES_SCRIPT_FUNC(GetEndGameReason, "Get end of game reason. See Constants.EAC1EndReason");
	HL2MP_GAMERULES_SCRIPT_FUNC(IsInIntermission, "Are we in the intermission state?");
	HL2MP_GAMERULES_SCRIPT_FUNC(HasEnded, "Did the game end?");
	HL2MP_GAMERULES_SCRIPT_FUNC(SoldierCount, "Returns the number of soldiers left in-game.");
	HL2MP_GAMERULES_SCRIPT_FUNC(GetTime, "Returns the time after the game started.");
	HL2MP_GAMERULES_SCRIPT_FUNC(GetWaitTime, "Returns the time set before a round starts or during an intermission.");
	HL2MP_GAMERULES_SCRIPT_FUNC(GetFreeman, "Returns the CHL2MP_Player chosen as Gordon Freeman.");
	HL2MP_GAMERULES_SCRIPT_FUNC(SetNextPlayerToBecomeFreeman, "Sets a new CHL2MP_Player to become Gordon Freeman.");
	HL2MP_GAMERULES_SCRIPT_FUNC(GetRoundCount, "Get the round count.");
	HL2MP_GAMERULES_SCRIPT_FUNC(AddLevelDesignerPlacedObject, "Adds an entity to the respawn list.");
	HL2MP_GAMERULES_SCRIPT_FUNC(RemoveLevelDesignerPlacedObject, "Removes an entity from the respawn list.");
}

#endif
