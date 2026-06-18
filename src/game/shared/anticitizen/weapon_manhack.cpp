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
	#include "npc_manhack.h"
#endif

#include "weapon_ar2.h"
#include "effect_dispatch_data.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MANHACK_TIMER	2.5f //Seconds

#define MANHACK_PAUSED_NO			0
#define MANHACK_PAUSED_PRIMARY		1
#define MANHACK_PAUSED_SECONDARY	2

#define MANHACK_RADIUS	4.0f // inches

#define MANHACK_DAMAGE_RADIUS 250.0f

#ifdef CLIENT_DLL
#define CWeaponManhack C_WeaponManhack
#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponManhack: public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponManhack, CBaseHL2MPCombatWeapon );
public:

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponManhack();

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

#ifndef CLIENT_DLL
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif

	void	ThrowGrenade( CBasePlayer *pPlayer );
	bool	IsPrimed( bool ) { return ( m_AttackPaused != 0 );	}
	
private:
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	CNetworkVar( bool,	m_bRedraw );	//Draw the weapon again after throwing a grenade
	
	CNetworkVar( int,	m_AttackPaused );
	CNetworkVar( bool,	m_fDrawbackFinished );

	CWeaponManhack( const CWeaponManhack & );

	DECLARE_ACTTABLE();
};

acttable_t	CWeaponManhack::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_MANHAC,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_MANHAC,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_MANHAC,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_MANHAC,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MANHAC,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_MANHAC,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_MANHAC,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_MANHAC,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_MANHAC,					false },

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
};

IMPLEMENT_ACTTABLE(CWeaponManhack);

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponManhack, DT_WeaponManhack )

BEGIN_NETWORK_TABLE( CWeaponManhack, DT_WeaponManhack )

#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bRedraw ) ),
	RecvPropBool( RECVINFO( m_fDrawbackFinished ) ),
	RecvPropInt( RECVINFO( m_AttackPaused ) ),
#else
	SendPropBool( SENDINFO( m_bRedraw ) ),
	SendPropBool( SENDINFO( m_fDrawbackFinished ) ),
	SendPropInt( SENDINFO( m_AttackPaused ) ),
#endif
	
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponManhack )
	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_fDrawbackFinished, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_AttackPaused, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_manhack, CWeaponManhack );
PRECACHE_WEAPON_REGISTER( weapon_manhack );

CWeaponManhack::CWeaponManhack( void ) :
	CBaseHL2MPCombatWeapon()
{
	m_bRedraw = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponManhack::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "npc_manhack" );
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
void CWeaponManhack::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	bool fThrewGrenade = false;

	switch( pEvent->event )
	{
		case EVENT_WEAPON_SEQUENCE_FINISHED:
			m_fDrawbackFinished = true;
			break;

		case EVENT_WEAPON_THROW:
			ThrowGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}

#define RETHROW_DELAY	0.5
	if( fThrewGrenade )
	{
		m_flNextPrimaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flTimeWeaponIdle = FLT_MAX; //NOTE: This is set once the animation has finished up!

		// Make a sound designed to scare snipers back into their holes!
		CBaseCombatCharacter* pOwner = GetOwner();

		if (pOwner)
		{
			Vector vecSrc = pOwner->Weapon_ShootPosition();
			Vector	vecDir;

			AngleVectors(pOwner->EyeAngles(), &vecDir);

			trace_t tr;

			UTIL_TraceLine(vecSrc, vecSrc + vecDir * 1024, MASK_SOLID_BRUSHONLY, pOwner, COLLISION_GROUP_NONE, &tr);

			CSoundEnt::InsertSound(SOUND_DANGER_SNIPERONLY, tr.endpos, 384, 0.2, pOwner);
		}
	}
}

#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponManhack::Deploy( void )
{
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	CHL2MP_Player* pOwner = ToHL2MPPlayer(GetOwner());

	if (pOwner)
	{
		if (pOwner->GetAmmoCount(GetPrimaryAmmoType()) == 1)
		{
			pOwner->SetBodygroup(1, 0);
		}
	}

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponManhack::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	CHL2MP_Player* pOwner = ToHL2MPPlayer(GetOwner());

	if (pOwner)
	{
		if (pOwner->GetAmmoCount(GetPrimaryAmmoType()) == 1)
		{
			pOwner->SetBodygroup(1, 1);
		}
	}

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponManhack::Reload( void )
{
	if ( !HasPrimaryAmmo() )
		return false;

	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bRedraw = false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponManhack::PrimaryAttack( void )
{
	if ( m_bRedraw )
		return;

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
	{ 
		return;
	}

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );;

	if ( !pPlayer )
		return;

	// Note that this is a primary attack and prepare the grenade attack to pause.
	m_AttackPaused = MANHACK_PAUSED_PRIMARY;
	SendWeaponAnim( ACT_VM_PULLBACK_HIGH );
	
	// Put both of these off indefinitely. We do not know how long
	// the player will hold the grenade.
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextPrimaryAttack = FLT_MAX;

	// If I'm now out of ammo, switch away
	if ( !HasPrimaryAmmo() )
	{
		pPlayer->SwitchToNextBestWeapon( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponManhack::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponManhack::ItemPostFrame( void )
{
	if( m_fDrawbackFinished )
	{
		CHL2MP_Player* pOwner = ToHL2MPPlayer(GetOwner());

		if (pOwner)
		{
			switch( m_AttackPaused )
			{
			case MANHACK_PAUSED_PRIMARY:
				if( !(pOwner->m_nButtons & IN_ATTACK) )
				{
					SendWeaponAnim( ACT_VM_THROW );
					pOwner->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);
					m_fDrawbackFinished = false;
				}
				break;

			case MANHACK_PAUSED_SECONDARY:
				if( !(pOwner->m_nButtons & IN_ATTACK2) )
				{
					//See if we're ducking
					if ( pOwner->m_nButtons & IN_DUCK )
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_SECONDARYATTACK );
					}
					else
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_HAULBACK );
					}

					//Tony; the grenade really should have a secondary anim. but it doesn't on the player.
					pOwner->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);

					m_fDrawbackFinished = false;
				}
				break;

			default:
				break;
			}
		}
	}

	BaseClass::ItemPostFrame();

	if ( m_bRedraw )
	{
		if ( IsViewModelSequenceFinished() )
		{
			Reload();

			CHL2MP_Player* pHL2MPPlayer = ToHL2MPPlayer(GetOwner());

			if (pHL2MPPlayer)
			{
				if (pHL2MPPlayer->GetPlayerClass() != CLS_FREEMAN)
				{
					pHL2MPPlayer->SwitchToNextBestWeapon(this);
				}
			}
		}
	}
}

	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
void CWeaponManhack::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(MANHACK_RADIUS+2,MANHACK_RADIUS+2,MANHACK_RADIUS+2), Vector(MANHACK_RADIUS+2,MANHACK_RADIUS+2,MANHACK_RADIUS+2), 
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
void CWeaponManhack::ThrowGrenade( CBasePlayer *pPlayer )
{
	CHL2MP_Player* pHL2MPPlayer = ToHL2MPPlayer(pPlayer);

#ifndef CLIENT_DLL
	CNPC_Manhack* pManhack = dynamic_cast<CNPC_Manhack*>(CreateEntityByName("npc_manhack"));
	if (pManhack)
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
		vecThrow += vForward * 650;

		AngularImpulse angImp = AngularImpulse(600, random->RandomInt(-1200, 1200), 0);

		pManhack->SetName(AllocPooledString("spawnedManhack"));
		pManhack->SetOwnerEntity(pPlayer);
		pManhack->SetLocalOrigin(vecSrc);
		pManhack->SetLocalAngles(vec3_angle);
		pManhack->AddSpawnFlags((SF_MANHACK_PACKED_UP | SF_NPC_FADE_CORPSE));
		pManhack->Spawn();
		// not needed, but allows the manhack to work properly when playing as freeman.
		pManhack->ChangeTeam(pPlayer->GetTeamNumber());

		IPhysicsObject* pPhysicsObject = pManhack->VPhysicsGetObject();
		if (pPhysicsObject)
		{
			pPhysicsObject->AddVelocity(&vecThrow, &angImp);
		}
	}

	if (pHL2MPPlayer)
	{
		pHL2MPPlayer->SpeakSentence("DEPLOY_MANHACK");
	}
#endif

	m_bRedraw = true;
	m_iPrimaryAttacks++;

	WeaponSound( SINGLE );
	
	// player "shoot" animation
	if (pHL2MPPlayer)
	{
		pHL2MPPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);
	}

#ifdef GAME_DLL
	pPlayer->OnMyWeaponFired( this );
#endif
}