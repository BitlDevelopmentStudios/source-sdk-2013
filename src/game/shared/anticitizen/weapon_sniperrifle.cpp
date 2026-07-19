
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_sniperrifle.h"

#define SNIPER_BEAM_TEX "effects/bluelaser1.vmt"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSniperRifle, DT_WeaponSniperRifle )

BEGIN_NETWORK_TABLE( CWeaponSniperRifle, DT_WeaponSniperRifle )
#ifdef CLIENT_DLL
	RecvPropBool(RECVINFO(m_bInZoom)),
#else
	SendPropBool(SENDINFO(m_bInZoom)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponSniperRifle)
DEFINE_PRED_FIELD(m_bInZoom, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_sniperrifle, CWeaponSniperRifle );
PRECACHE_WEAPON_REGISTER( weapon_sniperrifle );

acttable_t CWeaponSniperRifle::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2AC_IDLE_SNIPER,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2AC_CROUCH_SNIPER,			false },

	{ ACT_MP_RUN,						ACT_HL2AC_RUN_SNIPER,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2AC_WALK_CROUCH_SNIPER,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK_SNIPER,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2AC_GESTURE_RANGE_ATTACK_SNIPER,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2AC_GESTURE_RELOAD_SNIPER,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2AC_GESTURE_RELOAD_SNIPER,		false },

	{ ACT_MP_JUMP,						ACT_HL2AC_JUMP_SNIPER,					false },

	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_SNIPER_RIFLE,				false },
};

IMPLEMENT_ACTTABLE( CWeaponSniperRifle );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponSniperRifle::CWeaponSniperRifle( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
	m_fMaxRange1 = 2048;
	m_bInZoom = false;
}

void CWeaponSniperRifle::Precache(void)
{
	PrecacheParticleSystem("hunter_muzzle_flash");
	UTIL_PrecacheOther("sniperbullet");
#ifndef CLIENT_DLL
	sHaloSprite = PrecacheModel("sprites/light_glow03.vmt");
#endif
	PrecacheModel(SNIPER_BEAM_TEX);

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

	if (!HasPrimaryAmmo())
	{
		WeaponSound(EMPTY);
		m_flNextPrimaryAttack = 0.15;
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

	// enough for 4 shots
	pPlayer->RemoveAmmo(25, m_iPrimaryAmmoType);

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );	

	CreateMuzzleSmokeEffect();

	FireBulletsInfo_t info( 1, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets( info );

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

extern ConVar sk_max_sniper_round;

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
	if (m_bInZoom)
	{
		ToggleZoom();
#ifndef CLIENT_DLL
		LaserOff();
#endif
	}

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::Drop(const Vector& vecVelocity)
{
	if (m_bInZoom)
	{
		ToggleZoom();
#ifndef CLIENT_DLL
		LaserOff();
#endif
	}

	BaseClass::Drop(vecVelocity);
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
	if (m_bInZoom)
	{
		if (pPlayer->SetFOV(this, 0, 0.5f))
		{
			m_bInZoom = false;
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 20, 0.5f))
		{
			m_bInZoom = true;
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

#ifndef CLIENT_DLL
	if (m_bInZoom)
	{
		LaserOn();
	}
	else
	{
		LaserOff();
	}

	int iAmmoCount = pPlayer->GetAmmoCount(m_iPrimaryAmmoType);

	if (iAmmoCount < sk_max_sniper_round.GetInt())
	{
		pPlayer->GiveAmmo(1, m_iPrimaryAmmoType, true);
	}
#endif

	BaseClass::ItemPostFrame();
}

void CWeaponSniperRifle::HolsterThink(void)
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
		pPlayer->GiveAmmo(1, m_iPrimaryAmmoType, true);
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::DoImpactEffect(trace_t& tr, int nDamageType)
{
	CEffectData data;

	data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
	data.m_vNormal = tr.plane.normal;

	DispatchEffect("AR2Impact", data);

	BaseClass::DoImpactEffect(tr, nDamageType);
}

#ifndef CLIENT_DLL
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CWeaponSniperRifle::LaserOff(void)
{
	if (m_pBeam)
	{
		UTIL_Remove(m_pBeam);
		m_pBeam = NULL;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define LASER_LEAD_DIST	64
void CWeaponSniperRifle::LaserOn(void)
{
	CHL2MP_Player* pPlayer = ToHL2MPPlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (!m_pBeam)
	{
		m_pBeam = CBeam::BeamCreate("effects/bluelaser1.vmt", 1.0f);
		m_pBeam->SetColor(0, 100, 255);
	}
	else
	{
		// Beam seems to be on.
		//return;
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
	vecStop = vecStart + vecDir * MAX_TRACE_LENGTH;

	// Do the TraceLine
	UTIL_TraceLine(vecStart, vecStop, MASK_ALL, this, COLLISION_GROUP_NONE, &tr);

	m_pBeam->PointEntInit(tr.endpos, this);
	m_pBeam->SetBrightness(255);
	m_pBeam->SetNoise(0);
	m_pBeam->SetWidth(1.0f);
	m_pBeam->SetEndWidth(0);
	m_pBeam->SetScrollRate(0);
	m_pBeam->SetFadeLength(0);
	m_pBeam->SetHaloTexture(sHaloSprite);
	m_pBeam->SetHaloScale(4.0f);
}
#endif
