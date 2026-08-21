//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2MP_GAMERULES_H
#define HL2MP_GAMERULES_H
#pragma once

#include "gamerules.h"
#include "hl2_gamerules.h"
#include "gamevars_shared.h"

#ifndef CLIENT_DLL
#include "hl2mp_player.h"
#else
#include "c_hl2mp_player.h"
#endif

#define VEC_CROUCH_TRACE_MIN	HL2MPRules()->GetHL2MPViewVectors()->m_vCrouchTraceMin
#define VEC_CROUCH_TRACE_MAX	HL2MPRules()->GetHL2MPViewVectors()->m_vCrouchTraceMax

enum
{
	TEAM_COMBINE = 2,
	TEAM_FREEMAN,
};

enum
{
	STATE_PREROUND,
	STATE_PLAYING,
	STATE_COMPLETION
};

enum
{
	GAME_NOT_ENDED,
	GAME_END_NOTENOUGHPLAYERS,
	GAME_END_FREEMANDEAD,
	GAME_END_NOMORESOLDIERS
};

enum
{
	TIMERSTATE_NONE,
	TIMERSTATE_ROUNDSTART,
	TIMERSTATE_GAMESTART,
	TIMERSTATE_RESTART,
	TIMERSTATE_CHANGELEVEL,
};

#ifdef CLIENT_DLL
	#define CHL2MPRules C_HL2MPRules
	#define CHL2MPGameRulesProxy C_HL2MPGameRulesProxy
#endif

class CHL2MPGameRulesProxy : public CHalfLife2Proxy
{
public:
	DECLARE_CLASS( CHL2MPGameRulesProxy, CHalfLife2Proxy );
	DECLARE_NETWORKCLASS();
};

class HL2MPViewVectors : public CViewVectors
{
public:
	HL2MPViewVectors( 
		Vector vView,
		Vector vHullMin,
		Vector vHullMax,
		Vector vDuckHullMin,
		Vector vDuckHullMax,
		Vector vDuckView,
		Vector vObsHullMin,
		Vector vObsHullMax,
		Vector vDeadViewHeight,
		Vector vCrouchTraceMin,
		Vector vCrouchTraceMax ) :
			CViewVectors( 
				vView,
				vHullMin,
				vHullMax,
				vDuckHullMin,
				vDuckHullMax,
				vDuckView,
				vObsHullMin,
				vObsHullMax,
				vDeadViewHeight )
	{
		m_vCrouchTraceMin = vCrouchTraceMin;
		m_vCrouchTraceMax = vCrouchTraceMax;
	}

	Vector m_vCrouchTraceMin;
	Vector m_vCrouchTraceMax;	
};

class CHL2MPRules : public CHalfLife2
{
public:
	DECLARE_CLASS( CHL2MPRules, CHalfLife2 );

#ifdef CLIENT_DLL

	DECLARE_CLIENTCLASS_NOBASE(); // This makes datatables able to access our private vars.

#else

	DECLARE_SERVERCLASS_NOBASE(); // This makes datatables able to access our private vars.
#endif
	
	CHL2MPRules();
	virtual ~CHL2MPRules();

	virtual void Precache( void );
	virtual bool ShouldCollide( int collisionGroup0, int collisionGroup1 );
	virtual bool ClientCommand( CBaseEntity *pEdict, const CCommand &args );

	virtual float FlWeaponRespawnTime( CBaseCombatWeapon *pWeapon );
	virtual float FlWeaponTryRespawn( CBaseCombatWeapon *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBaseCombatWeapon *pWeapon );
	virtual int WeaponShouldRespawn( CBaseCombatWeapon *pWeapon );
	virtual int GetRemainingSoldierCount(void);
	virtual int GetSoldierCount(void);
	virtual int GetSpectatorCount(void);
	virtual void CheckLastMemberLeft(void);
	virtual void SelectFreeman(void);
	virtual void ReassignSpectators(void);
	virtual bool IsFreemanAlive(void);
	virtual int CheckCanEndGame(void);
	virtual void Think( void );
	virtual void CreateStandardEntities( void );
	virtual void ClientSettingsChanged( CBasePlayer *pPlayer );
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual void GoToIntermission( void );
	virtual void LeaveIntermission(void);
	virtual void DeathNotice( CBasePlayer *pVictim, const CTakeDamageInfo &info );
	virtual const char *GetGameDescription( void );
	// derive this function if you mod uses encrypted weapon info files
	virtual const unsigned char *GetEncryptionKey( void ) { return (unsigned char *)"x9Ke0BY7"; }
	virtual const CViewVectors* GetViewVectors() const;
	const HL2MPViewVectors* GetHL2MPViewVectors() const;

	void CleanUpMap();
	void RestartGame(bool gameend = false);

	int GetState(void) { return m_iRoundState; }
	int GetEndGameReason(void) { return m_iGameEndReason; }
	int HasEnded(void) { return m_bJustEnded; }
	int GetRoundCount(void) { return m_iCurrentRound; }
	bool IsInFinalRound(void) { return m_bFinalRound; }
	bool LastPlayerAnnounced(void) { return m_bLastSquadMemberAnnounced; }

	virtual CHL2MP_Player* GetFreeman(void);
	float GetFreemanHealthFraction(void);
	int GetFreemanHealth(void);
	int GetFreemanMaxHealth(void);
	
#ifndef CLIENT_DLL
	virtual Vector VecItemRespawnSpot( CItem *pItem );
	virtual QAngle VecItemRespawnAngles( CItem *pItem );
	virtual float	FlItemRespawnTime( CItem *pItem );
	virtual bool	CanHavePlayerItem( CBasePlayer *pPlayer, CBaseCombatWeapon *pItem );
	virtual bool FShouldSwitchWeapon( CBasePlayer *pPlayer, CBaseCombatWeapon *pWeapon );
	virtual void GetTaggedConVarList(KeyValues* pCvarTagList);

	void	AddLevelDesignerPlacedObject( CBaseEntity *pEntity );
	void	RemoveLevelDesignerPlacedObject( CBaseEntity *pEntity );
	void	ManageObjectRelocation( void );
	const char *GetChatFormat( bool bTeamOnly, CBasePlayer *pPlayer );

	void SetNextPlayerToBecomeFreeman(CHL2MP_Player* pPlayer) 
	{ 
		pNextPlayerToBecomeFreeman = pPlayer; 
	}

	CHL2MP_Player* GetNextPlayerToBecomeFreeman(void) 
	{
		return pNextPlayerToBecomeFreeman;
	}

	void PlayerSpawn(CBasePlayer* pPlayer);
	void OnNavMeshLoad(void);

	void SendHudMessage(CBasePlayer* pToPlayer, const char* text, float flDuration = 5.0f);
	void SendHudMessage(CBasePlayer* pToPlayer, string_t text, float flDuration = 5.0f);

	void RegisterScriptFunctions() override;
#endif

	bool IsOfficialMap( void );

	virtual void ClientDisconnected( edict_t *pClient );

	bool IsIntermission( void );

	void PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info );

	
	bool	IsTeamplay( void ) { return true;	}

	virtual bool IsConnectedUserInfoChangeAllowed( CBasePlayer *pPlayer );

	bool	MegaPhyscannonActive(void) { return false; }

	float GetMapRemainingTime();

	float GetTimeSinceGameStart() { return (gpGlobals->curtime - m_flTimeSinceGameStart); }

	bool HasRoundStarted() { return (m_flGameStartTime < gpGlobals->curtime); }

	int GetTimerState() { return m_iTimerType; }

	void Announce(bool gameend = false);
	void AwardGameEndAchievements();
	int GetFreemanBulletsShot();
	
private:
	
	CNetworkVar( bool, m_bFinalRound );
	CNetworkVar( bool, m_bIsInIntermission );
	CNetworkVar( float, m_flGameStartTime );
	CNetworkVar( float, m_flTimeSinceGameStart );
	CNetworkVar( float, m_flGameEndTime );
	CNetworkVar( int, m_iTimerType );
	CNetworkVar(int, m_iSoldiers);
	CNetworkVar(int, m_iRoundState);
	CNetworkVar(int, m_iCurrentRound);
	CNetworkVar(int, m_iGameEndReason);
	CUtlVector<EHANDLE> m_hRespawnableItemsAndWeapons;
	bool m_bCompleteReset;
	bool m_bHasMinPlayersToStart;
	bool m_bReassignSpectators;
	bool m_bStartedStartClock;
	bool m_bJustEnded;
	bool m_bLastSquadMemberAnnounced;
	bool m_bAnnouncedGameStart;
	bool m_bAnnouncedGameEnd;
	bool m_bGaveGameEndAchievements;
	bool m_bSentGameEndEvent;
	bool m_bStrippedFlags;

#ifndef CLIENT_DLL
	//may be disasterous
	CHL2MP_Player* pFreeman;
	CHL2MP_Player* pNextPlayerToBecomeFreeman;
	bool m_bChangelevelDone;
	uint64 m_uiFreemanID;
	uint64 m_uiLastFreemanID;
	int m_iNumTimesFreemanIDShowedUpIFuckingHateThis;
#endif
};

inline CHL2MPRules* HL2MPRules()
{
	return static_cast<CHL2MPRules*>(g_pGameRules);
}

#endif //HL2MP_GAMERULES_H
