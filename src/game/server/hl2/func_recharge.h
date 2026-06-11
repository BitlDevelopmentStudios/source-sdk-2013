//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
/*

===== h_battery.cpp ========================================================

  battery-related code

*/

#ifndef FUNC_RECHARGE_H
#define FUNC_RECHARGE_H
#ifdef _WIN32
#pragma once
#endif

#include "gamerules.h"
#include "player.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "ammodef.h"

#define SF_CITADEL_RECHARGER	0x2000
#define SF_KLEINER_RECHARGER	0x4000 // Gives only 25 health

#define HEALTH_CHARGER_MODEL_NAME "models/props_combine/suit_charger001.mdl"
#define CHARGE_RATE 0.25f
#define CHARGES_PER_SECOND 1 / CHARGE_RATE
#define CITADEL_CHARGES_PER_SECOND 10 / CHARGE_RATE
#define CALLS_PER_SECOND 7.0f * CHARGES_PER_SECOND

class CRecharge : public CBaseToggle
{
public:
	DECLARE_CLASS( CRecharge, CBaseToggle );

	void Spawn( );
	bool CreateVPhysics();
	int DrawDebugTextOverlays(void);
	void Off(void);
	void Recharge(void);
	bool KeyValue( const char *szKeyName, const char *szValue );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int	ObjectCaps( void ) { return (BaseClass::ObjectCaps() | FCAP_CONTINUOUS_USE); }

	int GetJuice() const { return m_iJuice; }

private:
	void InputRecharge( inputdata_t &inputdata );
	
	float MaxJuice() const;
	void UpdateJuice( int newJuice );

	DECLARE_DATADESC();

	float	m_flNextCharge; 
	int		m_iReactivate ; // DeathMatch Delay until reactvated
	int		m_iJuice;
	int		m_iOn;			// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;
	
	int		m_nState;
	
	COutputFloat m_OutRemainingCharge;
	COutputEvent m_OnHalfEmpty;
	COutputEvent m_OnEmpty;
	COutputEvent m_OnFull;
	COutputEvent m_OnPlayerUse;
};

//NEW
class CNewRecharge : public CBaseAnimating
{
public:
	DECLARE_CLASS( CNewRecharge, CBaseAnimating );

	void Spawn( );
	bool CreateVPhysics();
	int DrawDebugTextOverlays(void);
	void Off(void);
	void Recharge(void);
	bool KeyValue( const char *szKeyName, const char *szValue );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int	ObjectCaps( void ) { return (BaseClass::ObjectCaps() | m_iCaps ); }

	void SetInitialCharge( void );

	int GetJuice() const { return m_iJuice; }

private:
	void InputRecharge( inputdata_t &inputdata );
	void InputSetCharge( inputdata_t &inputdata );
	float MaxJuice() const;
	void UpdateJuice( int newJuice );
	void Precache( void );

	DECLARE_DATADESC();

	float	m_flNextCharge; 
	int		m_iReactivate ; // DeathMatch Delay until reactvated
	int		m_iJuice;
	int		m_iOn;			// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;
	
	int		m_nState;
	int		m_iCaps;
	int		m_iMaxJuice;
	
	COutputFloat m_OutRemainingCharge;
	COutputEvent m_OnHalfEmpty;
	COutputEvent m_OnEmpty;
	COutputEvent m_OnFull;
	COutputEvent m_OnPlayerUse;

	virtual void StudioFrameAdvance ( void );
	float m_flJuice;
};

#endif