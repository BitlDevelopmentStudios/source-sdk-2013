
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_SNIPERRIFLE_H
#define WEAPON_SNIPERRIFLE_H
#ifdef _WIN32
#pragma once
#endif

#include "npcevent.h"
#include "in_buttons.h"
#include "beam_shared.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "prediction.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "hl2mp_player.h"
	#include "te_effect_dispatch.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponSniperRifle C_WeaponSniperRifle
#endif

//-----------------------------------------------------------------------------
// CWeaponSniperRifle
//-----------------------------------------------------------------------------

class CWeaponSniperRifle : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponSniperRifle, CBaseHL2MPCombatWeapon );
public:

	CWeaponSniperRifle( void );

	void	PrimaryAttack( void );
	void	Precache(void);
	void	ItemPostFrame(void);
	void	HolsterThink(void);
	const char* GetTracerType(void) { return "AR2Tracer"; }
	void	DoImpactEffect(trace_t& tr, int nDamageType);
	void	CheckZoomToggle(void);
	virtual void	SecondaryAttack(void);
	void	ToggleZoom(void);
	virtual bool	Holster(CBaseCombatWeapon* pSwitchingTo = NULL);
	virtual void	ItemBusyFrame(void);
	void			Drop(const Vector& vecVelocity);
#ifndef CLIENT_DLL
	void	LaserOff(void);
	void	LaserOn(void);
#endif

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

private:
	CNetworkVar(bool, m_bInZoom);
#ifndef CLIENT_DLL
	int sHaloSprite;
	CBeam* m_pBeam;
#endif

private:
	
	CWeaponSniperRifle( const CWeaponSniperRifle & );
};

#endif