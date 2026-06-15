//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		The class from which all bludgeon melee
//				weapons are derived. 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifndef BASEBLUDGEONWEAPON_H
#define BASEBLUDGEONWEAPON_H

#ifdef _WIN32
#pragma once
#endif


#if defined( CLIENT_DLL )
#define CBaseHL2MPBludgeonWeapon C_BaseHL2MPBludgeonWeapon
#endif

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

class CTraceFilterIgnoreTeammates : public CTraceFilterSimple
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS(CTraceFilterIgnoreTeammates, CTraceFilterSimple);

	CTraceFilterIgnoreTeammates(const IHandleEntity* passentity, int collisionGroup, int iIgnoreTeam)
		: CTraceFilterSimple(passentity, collisionGroup), m_iIgnoreTeam(iIgnoreTeam)
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity* pServerEntity, int contentsMask)
	{
		CBaseEntity* pEntity = EntityFromEntityHandle(pServerEntity);

		if ((pEntity->IsPlayer() || pEntity->IsCombatItem()) && (pEntity->GetTeamNumber() == m_iIgnoreTeam || m_iIgnoreTeam == TEAM_ANY))
		{
			return false;
		}

		return BaseClass::ShouldHitEntity(pServerEntity, contentsMask);
	}

	int m_iIgnoreTeam;
};

//=========================================================
// CBaseHLBludgeonWeapon 
//=========================================================
class CBaseHL2MPBludgeonWeapon : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CBaseHL2MPBludgeonWeapon, CBaseHL2MPCombatWeapon );
public:
	CBaseHL2MPBludgeonWeapon();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

#ifndef CLIENT_DLL 
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_MELEE_ATTACK1; }
#endif 

	virtual	void	Spawn( void );
	virtual	void	Precache( void );
	
	//Attack functions
	virtual	void	PrimaryAttack( void );
	virtual	void	SecondaryAttack( void );
	
	virtual void	ItemPostFrame( void );

	//Functions to select animation sequences 
	virtual Activity	GetPrimaryAttackActivity( void )	{	return	ACT_VM_HITCENTER;	}
	virtual Activity	GetSecondaryAttackActivity( void )	{	return	ACT_VM_HITCENTER2;	}

	virtual	float	GetFireRate( void )								{	return	0.2f;	}
	virtual float	GetRange( void )								{	return	32.0f;	}
	virtual	float	GetDamageForActivity( Activity hitActivity )	{	return	1.0f;	}

	CBaseHL2MPBludgeonWeapon( const CBaseHL2MPBludgeonWeapon & );

	virtual void	Hit(trace_t& traceHit, Activity nHitActivity);
	virtual void	Swing(int bIsSecondary);

	bool			ImpactWater(const Vector& start, const Vector& end);
	Activity		ChooseIntersectionPointAndActivity(trace_t& hitTrace, const Vector& mins, const Vector& maxs, CBasePlayer* pOwner);

protected:
	virtual	void	ImpactEffect( trace_t &trace );
};

#endif
