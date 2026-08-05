
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
#include "hl2mp/weapon_rpg.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "prediction.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "hl2mp_player.h"
	#include "te_effect_dispatch.h"
	#include "actual_bullet.h"
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponSniperRifle C_WeaponSniperRifle
#endif

#define SNIPER_MIN_RANGE 512 // if this is OP, increase to 1024.
#define SNIPER_MAX_RANGE 16384
#define SNIPER_CHARGE_DRAIN 25
#define SNIPER_RECHARGE_TIME 0.25f
#define SNIPER_RECHARGE_ZOOMED_TIME 0.5f
#define SNIPER_RECHARGE_HOLSTERED_TIME 0.1f
#define SNIPER_BULLET_SPEED 6000.0f

enum ChargeState_t
{
	CHARGE_STATE_ACTIVE,
	CHARGE_STATE_ZOOMED,
	CHARGE_STATE_HOLSTERED,
};

//-----------------------------------------------------------------------------
// CWeaponSniperRifle
//-----------------------------------------------------------------------------

class CWeaponSniperRifle : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponSniperRifle, CBaseHL2MPCombatWeapon );
public:

	CWeaponSniperRifle( void );
	~CWeaponSniperRifle(void);

	void	PrimaryAttack( void );
	void	Precache(void);
	void	ItemPostFrame(void);
	void	HolsterThink(void);
	const char* GetTracerType(void) { return "AR2Tracer"; }
	void	CheckZoomToggle(void);
	virtual void	SecondaryAttack(void);
	void	ToggleZoom(void);
	virtual bool	Holster(CBaseCombatWeapon* pSwitchingTo = NULL);
	virtual void	ItemBusyFrame(void);
	void			Drop(const Vector& vecVelocity);
	bool			ShouldBeep(void);
	bool			IsReady(void);
	void			Charge(int iState);

	void	TurnOff(void);

	void	LaserOff(void);
	void	LaserOn(void);

	virtual bool IsWeaponZoomed() { return m_bInZoom; }		// Is this weapon in its 'zoomed in' mode?

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone;

		cone = VECTOR_CONE_2DEGREES;

		if (m_bInZoom)
			cone = vec3_origin;

		return cone;
	}

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

private:
	CNetworkVar(bool, m_bInZoom);
	CNetworkVar(bool, m_bLaserOn);
	CNetworkVar(float, m_flRechargeTime);

#ifndef CLIENT_DLL
	EHANDLE	m_hLaserDot;
#endif

private:
	
	CWeaponSniperRifle( const CWeaponSniperRifle & );
};

#endif