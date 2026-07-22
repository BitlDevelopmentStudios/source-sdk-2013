
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_sniperrifle.h"

extern ConVar sk_max_sniper_round;

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSniperRifle, DT_WeaponSniperRifle )

BEGIN_NETWORK_TABLE( CWeaponSniperRifle, DT_WeaponSniperRifle )
#ifdef CLIENT_DLL
	RecvPropBool(RECVINFO(m_bInZoom)),
	RecvPropFloat(RECVINFO(m_flRechargeTime)),
	RecvPropBool(RECVINFO(m_bLaserOn)),
#else
	SendPropBool(SENDINFO(m_bInZoom)),
	SendPropFloat(SENDINFO(m_flRechargeTime)),
	SendPropBool(SENDINFO(m_bLaserOn)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponSniperRifle)
DEFINE_PRED_FIELD(m_bInZoom, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flRechargeTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_bLaserOn, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_sniperrifle, CWeaponSniperRifle );
PRECACHE_WEAPON_REGISTER( weapon_sniperrifle );

acttable_t CWeaponSniperRifle::m_acttable[] = 
{
	/*
	{ ACT_MP_STAND_IDLE,				ACT_HL2AC_IDLE_SNIPER,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2AC_CROUCH_SNIPER,				false },

	{ ACT_MP_RUN,						ACT_HL2AC_RUN_SNIPER,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2AC_WALK_CROUCH_SNIPER,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK_SNIPER,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK_SNIPER,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2AC_GESTURE_RELOAD_SNIPER,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2AC_GESTURE_RELOAD_SNIPER,		false },

	{ ACT_MP_JUMP,						ACT_HL2AC_JUMP_SNIPER,					false },
	*/

	// looks better with the ar2 anims
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_AR2,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_AR2,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_AR2,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_AR2,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_AR2,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_AR2,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_AR2,					false },

	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_SNIPER_RIFLE,		false },
};

IMPLEMENT_ACTTABLE( CWeaponSniperRifle );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponSniperRifle::CWeaponSniperRifle( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
	m_fMaxRange1 = SNIPER_RANGE;
	m_flRechargeTime = 0;
	m_bInZoom = false;
	m_bLaserOn = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponSniperRifle::~CWeaponSniperRifle()
{
#ifndef CLIENT_DLL
	if (m_hLaserDot)
	{
		UTIL_Remove(m_hLaserDot);
		m_hLaserDot = NULL;
	}
#endif
}

void CWeaponSniperRifle::Precache(void)
{
	PrecacheParticleSystem("hunter_muzzle_flash");
	PrecacheModel("sprites/blueglow1.vmt");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::SecondaryAttack(void)
{
	//NOTENOTE: The zooming is handled by the post/busy frames
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CHL2MP_Player* pPlayer = ToHL2MPPlayer(GetOwner());

	if ( !pPlayer )
	{
		return;
	}

	int iAmmoCount = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);

	if (!HasPrimaryAmmo() || iAmmoCount < SNIPER_CHARGE_DRAIN)
	{
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.2f;
		return;
	}

	m_iPrimaryAttacks++;
	WeaponSound( SINGLE );

	if (GetPlayerOwner() && !m_bInZoom)
	{
		DispatchParticleEffect("hunter_muzzle_flash", PATTACH_POINT_FOLLOW, GetPlayerOwner()->GetViewModel(), "muzzle", false);
	}
	else
	{
		DispatchParticleEffect("hunter_muzzle_flash", PATTACH_POINT_FOLLOW, this, "muzzle", false);
	}

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration() + 1.5f;
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration() + 1.5f;

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );	

	CreateMuzzleSmokeEffect();

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;

	if (iAmmoCount < sk_max_sniper_round.GetInt())
	{
		info.m_flDamageScale = 0.50f;
	}

	// Fire the bullets, and force the first shot to be perfectly accuracy
	//pPlayer->FireBullets( info );
#ifndef CLIENT_DLL
	FireActualBullet(info, SNIPER_BULLET_SPEED, GetTracerType(), true, true, "AR2Impact");
#endif // CLIENT_DLL

#ifdef CLIENT_DLL
	//Disorient the player
	if ( prediction->IsFirstTimePredicted() )
	{
		QAngle angles;
		engine->GetViewAngles( angles );
		angles.x += random->RandomInt( -1, 1 );
		angles.y += random->RandomInt( -1, 1 );
		angles.z += 0.0f;
		engine->SetViewAngles( angles );
	}
#endif // CLIENT_DLL

	// enough for 4 shots
	pPlayer->RemoveAmmo(SNIPER_CHARGE_DRAIN, m_iPrimaryAmmoType);
	// don't immediately start charging.
	m_flRechargeTime = gpGlobals->curtime + SNIPER_RECHARGE_TIME;

	pPlayer->ViewPunch( QAngle( -8, random->RandomFloat( -2, 2 ), 0 ) );
#ifndef CLIENT_DLL
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner());
#endif // CLIENT_DLL

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); 
	}
}

bool CWeaponSniperRifle::ShouldBeep(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer)
	{
		int iAmmoCount = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);
		return ((iAmmoCount < sk_max_sniper_round.GetInt()) && 
			   (((iAmmoCount % SNIPER_CHARGE_DRAIN) == 0) || (iAmmoCount == (sk_max_sniper_round.GetInt() - 1))));
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::CheckZoomToggle(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer->m_afButtonPressed & IN_ATTACK2)
	{
		ToggleZoom();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::ItemBusyFrame(void)
{
	// Allow zoom toggling even when we're reloading
	CheckZoomToggle();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSwitchingTo - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponSniperRifle::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	TurnOff();

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::Drop(const Vector& vecVelocity)
{
	TurnOff();

	BaseClass::Drop(vecVelocity);
}

void CWeaponSniperRifle::TurnOff(void)
{
	if (m_bInZoom)
	{
		ToggleZoom();
	}

	LaserOff();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::ToggleZoom(void)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	if (!m_bInZoom)
	{
		WeaponSound(SPECIAL1);
	}
	else
	{
		WeaponSound(SPECIAL2);
	}

#ifndef CLIENT_DLL
	color32 lightBlue = { 0, 100, 255, 32 };
	float flZoomTime = 0.5f;

	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, flZoomTime))
		{
			m_bInZoom = false;
			UTIL_ScreenFade(pPlayer, lightBlue, flZoomTime, 0, (FFADE_IN | FFADE_PURGE));
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 20, flZoomTime))
		{
			m_bInZoom = true;
			UTIL_ScreenFade(pPlayer, lightBlue, flZoomTime, 0, (FFADE_OUT | FFADE_PURGE | FFADE_STAYOUT));
		}
	}
#endif
}

void CWeaponSniperRifle::Charge(int iState)
{
	CHL2MP_Player* pPlayer = ToHL2MPPlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

#ifndef CLIENT_DLL
	int iAmmoCount = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);

	if (iAmmoCount < sk_max_sniper_round.GetInt())
	{
		if (m_flRechargeTime < gpGlobals->curtime)
		{
			pPlayer->GiveAmmo(1, m_iPrimaryAmmoType, true);

			switch (iState)
			{
				case CHARGE_STATE_ACTIVE:
				{
					m_flRechargeTime = gpGlobals->curtime + SNIPER_RECHARGE_TIME;
					break;
				}

				case CHARGE_STATE_ZOOMED:
				{
					m_flRechargeTime = gpGlobals->curtime + SNIPER_RECHARGE_ZOOMED_TIME;
					break;
				}

				case CHARGE_STATE_HOLSTERED:
				{
					m_flRechargeTime = gpGlobals->curtime + SNIPER_RECHARGE_HOLSTERED_TIME;
					break;
				}
			}

			if (ShouldBeep())
			{
				pPlayer->EmitSound("Weapon_SniperRifle.ChargeProgress");
			}
		}
	}
#endif
}

void CWeaponSniperRifle::ItemPostFrame(void)
{
	CHL2MP_Player* pPlayer = ToHL2MPPlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	// Allow zoom toggling
	CheckZoomToggle();

	if (m_bInZoom)
	{
		LaserOn();
	}
	else
	{
		LaserOff();
	}

	Charge(m_bInZoom ? CHARGE_STATE_ZOOMED : CHARGE_STATE_ACTIVE);

	BaseClass::ItemPostFrame();
}

void CWeaponSniperRifle::HolsterThink(void)
{
	CHL2MP_Player* pPlayer = ToHL2MPPlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	Charge(CHARGE_STATE_HOLSTERED);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::LaserOff(void)
{
	m_bLaserOn = false;

#ifndef CLIENT_DLL
	// Kill the dot completely
	if (m_hLaserDot)
	{
		EnableLaserDot(m_hLaserDot, false);
		UTIL_Remove(m_hLaserDot);
		m_hLaserDot = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::LaserOn(void)
{
#ifndef CLIENT_DLL
	CHL2MP_Player* pPlayer = ToHL2MPPlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	// The beam is backwards, sortof. The endpoint is the sniper. This is
	// so that the beam can be tapered to very thin where it emits from the sniper.
	// 
	// Set up the vectors and traceline
	trace_t tr;
	Vector vecStart, vecStop, vecDir;

	// Get the angles
	AngleVectors(pPlayer->EyeAngles(), &vecDir);

	Vector	vForward, vRight, vUp;

	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	Vector	muzzlePoint = pPlayer->Weapon_ShootPosition();

	// Get the vectors
	vecStart = muzzlePoint;
	vecStop = vecStart + vecDir * m_fMaxRange1;

	// Do the TraceLine
	UTIL_TraceLine(vecStart, vecStop, MASK_ALL, this, COLLISION_GROUP_NONE, &tr);

	if (m_hLaserDot == NULL)
	{
		m_hLaserDot = CreateLaserDotEx(GetAbsOrigin(), this, false, 1);
	}

	SetLaserDotPostition(m_hLaserDot, (tr.endpos + (tr.plane.normal * 1.0f)), tr.plane.normal);
	EnableLaserDot(m_hLaserDot, true);
#endif

	m_bLaserOn = true;
}
