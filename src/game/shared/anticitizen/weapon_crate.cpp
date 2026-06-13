//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_te_effect_dispatch.h"
#else
	#include "hl2mp_player.h"
	#include "te_effect_dispatch.h"
	#include "props.h"
#endif

#include "weapon_ar2.h"
#include "effect_dispatch_data.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CRATE_RADIUS	4.0f // inches

#ifdef CLIENT_DLL
#define CWeaponCrate C_WeaponCrate
#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponCrate: public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponCrate, CBaseHL2MPCombatWeapon );
public:

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponCrate();

	void	Precache( void );
	void	PrimaryAttack( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	ItemPostFrame( void );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

#ifndef CLIENT_DLL 
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
#endif 
	
	bool	Reload( void );

	bool	ShouldDisplayHUDHint() { return true; }
	
	Activity 	GetPrimaryAttackActivity( void );

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

	void	ThrowGrenade( CBasePlayer *pPlayer );
	
private:
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	CWeaponCrate( const CWeaponCrate & );

	DECLARE_ACTTABLE();
};

acttable_t	CWeaponCrate::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2AC_IDLE_SBOX,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2AC_CROUCH_SBOX,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SBOX,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2AC_WALK_CROUCH_SBOX,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK_SBOX,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK_SBOX,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2AC_IDLE_SBOX,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2AC_CROUCH_SBOX,		false },

	{ ACT_MP_JUMP,						ACT_HL2AC_JUMP_SBOX,					false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
};

IMPLEMENT_ACTTABLE(CWeaponCrate);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponCrate, DT_WeaponCrate )

BEGIN_NETWORK_TABLE(CWeaponCrate, DT_WeaponCrate)
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponCrate)
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_crate, CWeaponCrate );
PRECACHE_WEAPON_REGISTER( weapon_crate );

CWeaponCrate::CWeaponCrate( void ) :
	CBaseHL2MPCombatWeapon()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrate::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "item_item_crate_drop" );
#endif

	PrecacheScriptSound( "WeaponFrag.Throw" );
	PrecacheScriptSound( "WeaponFrag.Roll" );
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponCrate::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
}

#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponCrate::Deploy( void )
{
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponCrate::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponCrate::Reload( void )
{
	if ( !HasPrimaryAmmo() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponCrate::GetPrimaryAttackActivity( void )
{
	random->SetSeed((int)gpGlobals->curtime);
	bool choice = (random->RandomInt(0,1) == 1);

	return (choice ? ACT_SLAM_TRIPMINE_ATTACH2 : ACT_SLAM_TRIPMINE_ATTACH);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrate::PrimaryAttack( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
	{ 
		return;
	}

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );;

	if ( !pPlayer )
		return;

	// Note that this is a primary attack and prepare the grenade attack to pause.
	SendWeaponAnim( GetPrimaryAttackActivity() );
	
	ThrowGrenade( pPlayer );
	DecrementAmmo( pPlayer );
	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
	m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

	// If I'm now out of ammo, switch away
	if ( !HasPrimaryAmmo() )
	{
		pPlayer->SwitchToNextBestWeapon( this );
	}

	CHL2MP_Player* pHL2MPPlayer = ToHL2MPPlayer(GetOwner());

	if (pHL2MPPlayer)
	{
		if (pHL2MPPlayer->GetPlayerClass() != CLS_FREEMAN)
		{
			pHL2MPPlayer->SwitchToNextBestWeapon(this);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponCrate::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCrate::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();
}

// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
void CWeaponCrate::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(CRATE_RADIUS+2, CRATE_RADIUS+2, CRATE_RADIUS+2), Vector(CRATE_RADIUS+2, CRATE_RADIUS+2, CRATE_RADIUS+2),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() )
	{
		vecSrc = tr.endpos;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponCrate::ThrowGrenade( CBasePlayer *pPlayer )
{
#ifndef CLIENT_DLL
	CPhysicsProp* pCrate = dynamic_cast<CPhysicsProp*>(CreateEntityByName("item_item_crate_drop"));
	if (pCrate)
	{
		Vector	vecEye = pPlayer->EyePosition();
		Vector	vForward, vRight;

		pPlayer->EyeVectors(&vForward, &vRight, NULL);
		Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
		CheckThrowPosition(pPlayer, vecEye, vecSrc);
		//	vForward[0] += 0.1f;
		vForward[2] += 0.1f;

		Vector vecThrow;
		pPlayer->GetVelocity(&vecThrow, NULL);
		vecThrow += vForward * 450;

		AngularImpulse angImp = AngularImpulse(600, random->RandomInt(-1200, 1200), 0);

		pCrate->SetName(AllocPooledString("spawnedCrate"));
		//pCrate->SetOwnerEntity(pPlayer);
		pCrate->SetLocalOrigin(vecSrc);
		pCrate->SetLocalAngles(vec3_angle);
		pCrate->Spawn();

		IPhysicsObject* pPhysicsObject = pCrate->VPhysicsGetObject();
		if (pPhysicsObject)
		{
			pPhysicsObject->AddVelocity(&vecThrow, &angImp);
		}
	}

	/*CHL2MP_Player* pHL2MPPlayer = ToHL2MPPlayer(pPlayer);

	if (pHL2MPPlayer)
	{
		pHL2MPPlayer->SpeakSentence("DEPLOY_MANHACK");
	}*/
#endif

	WeaponSound( SINGLE );
	m_iPrimaryAttacks++;
	
	// player "shoot" animation
	CHL2MP_Player* pHL2MPPlayer = ToHL2MPPlayer(pPlayer);
	if (pHL2MPPlayer)
	{
		pHL2MPPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);
	}

#ifdef GAME_DLL
	pPlayer->OnMyWeaponFired( this );
#endif
}