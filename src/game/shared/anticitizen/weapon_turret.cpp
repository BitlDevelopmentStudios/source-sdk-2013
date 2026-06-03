//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:	'weapon' what lets the player controll the rollerbuddy.
//
// $Revision: $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "datacache/imdlcache.h"

#ifndef CLIENT_DLL
	#include "basehlcombatweapon.h"
	#include "basecombatcharacter.h"
	#include "ai_basenpc.h"
	#include "player.h"
	#include "entitylist.h"
	#include "ndebugoverlay.h"
	#include "soundent.h"
	#include "rotorwash.h"
	#include "npc_turret_floor.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define CWeaponTurret C_WeaponTurret
#endif

#define TURRET_MAX_PLACEMENT_RANGE 192.0f
#define TURRET_MIN_PLACEMENT_RANGE 32.0f

#define	FLOOR_TURRET_MODEL			"models/combine_turrets/Floor_turret_hologram.mdl"

#ifndef CLIENT_DLL
class CTurretHologram : public CAI_BaseNPC
{
public:
	DECLARE_CLASS(CTurretHologram, CAI_BaseNPC);
	DECLARE_DATADESC();

	enum HologramStatus
	{
		TURRET_JUSTRIGHT,
		TURRET_TOOFAR,
		TURRET_INVALIDPLACEMENT
	};

	void Spawn(void)
	{
		Precache();
		BaseClass::Spawn();

		//using a wide human as our measurement hull to allow for a better spawing area
		SetHullType(HULL_WIDE_HUMAN);
		SetHullSizeNormal();

		AddEFlags(EFL_DIRTY_ABSTRANSFORM);
		SetModel(FLOOR_TURRET_MODEL);

		SetThink(&CTurretHologram::OnThink);
		SetNextThink(gpGlobals->curtime + 0.1f);
	}

	void OnThink(void)
	{
		SetNextThink(gpGlobals->curtime + 0.1f);

		switch (status)
		{
		case TURRET_JUSTRIGHT:
			clrHighlightColor = Color("#50ff50");
			break;
		case TURRET_TOOFAR:
			clrHighlightColor = Color("#ff9350");
			break;
		case TURRET_INVALIDPLACEMENT:
			clrHighlightColor = Color("#fc3a3a");
			break;
		}

		SetRenderMode(kRenderTransColor);
		SetRenderColor(clrHighlightColor.r(), clrHighlightColor.g(), clrHighlightColor.b());
	}

	void SetStatus(HologramStatus statusToReport) { status = statusToReport; }
	void Precache(void) { PrecacheModel(FLOOR_TURRET_MODEL); }
	HologramStatus GetStatus(void) { return status; }

private:
	Color clrHighlightColor;
	HologramStatus status;
};


LINK_ENTITY_TO_CLASS(turret_hologram, CTurretHologram);
PRECACHE_REGISTER(turret_hologram);

BEGIN_DATADESC(CTurretHologram)
DEFINE_FIELD(clrHighlightColor, FIELD_COLOR32),
DEFINE_THINKFUNC(OnThink),
END_DATADESC()
#endif

class CWeaponTurret : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponTurret, CBaseHL2MPCombatWeapon);

	CWeaponTurret();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	DECLARE_ACTTABLE();
	

	bool				Deploy(void);
	void				Spawn(void);
	void				Precache(void);
	void				ItemPreFrame(void);
	void				ItemPostFrame(void);

	bool				Holster(CBaseCombatWeapon* pSwitchingTo = NULL);
	void				Drop(const Vector& velocity);

	void				StartHologram(void);
	void				MoveHologram(void);
	void				StopHologram(void);

	void				SpawnTurret(void);

	float GetFireRate(void)
	{
		return 0.5f;
	}

	void				PrimaryAttack(void);
	void				SecondaryAttack(void);
	bool				Reload(void);
	void				DecrementAmmo(CBaseCombatCharacter* pOwner);

private:
#ifndef CLIENT_DLL
	CHandle<CTurretHologram>	pHologram;
#endif

#ifndef CLIENT_DLL
	float				m_flNextHologramMove;
	bool				m_bStopMovingHologram;
	float				m_flCurrentBuildRotation;
	int					m_iDesiredBuildRotations;
#endif

private:
	CWeaponTurret(const CWeaponTurret&);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponTurret, DT_WeaponTurret)

BEGIN_NETWORK_TABLE(CWeaponTurret, DT_WeaponTurret)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponTurret)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_turret, CWeaponTurret);
PRECACHE_WEAPON_REGISTER(weapon_turret);

acttable_t CWeaponTurret::m_acttable[] =
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

IMPLEMENT_ACTTABLE(CWeaponTurret);

CWeaponTurret::CWeaponTurret()
{
	m_fMinRange1 = TURRET_MIN_PLACEMENT_RANGE;
	m_fMaxRange1 = TURRET_MAX_PLACEMENT_RANGE;
#ifndef CLIENT_DLL
	m_flNextHologramMove = gpGlobals->curtime;
	m_bStopMovingHologram = false;
	m_iDesiredBuildRotations = 0;
	m_flCurrentBuildRotation = 0.0f;
	pHologram = NULL;
#endif
}

void CWeaponTurret::Spawn()
{
	BaseClass::Spawn();
	Precache();
}

bool CWeaponTurret::Deploy(void)
{
	// overriding the entire deploy function.....
	MDLCACHE_CRITICAL_SECTION();
	bool bResult = DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), GetDrawActivity(), (char*)GetAnimPrefix());

	// If we should be lowered, deploy in the lowered position
	// We have to ask the player if the last time it checked, the weapon was lowered
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		CHL2MP_Player* pPlayer = assert_cast<CHL2MP_Player*>(GetOwner());
		if (pPlayer->IsWeaponLowered())
		{
			if (SelectWeightedSequence(ACT_VM_IDLE_LOWERED) != ACTIVITY_NOT_AVAILABLE)
			{
				if (DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), ACT_VM_IDLE_LOWERED, (char*)GetAnimPrefix()))
				{
					m_bLowered = true;

					// Stomp the next attack time to fix the fact that the lower idles are long
					pPlayer->SetNextAttack(gpGlobals->curtime + 1.0);
					m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
					m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
					return true;
				}
			}
		}
	}

	m_bLowered = false;

#ifndef CLIENT_DLL
	if (!pHologram)
	{
		StartHologram();
	}
#endif

	return bResult;
}

void CWeaponTurret::Precache(void)
{
	BaseClass::Precache();
	//??????
	//UTIL_PrecacheOther("npc_turret_floor");
}

void CWeaponTurret::ItemPreFrame(void)
{
	MoveHologram();

	BaseClass::ItemPreFrame();
}

void CWeaponTurret::ItemPostFrame(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	MoveHologram();

#ifndef CLIENT_DLL
	if (m_bStopMovingHologram && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		SpawnTurret();
	}
#endif

	BaseClass::ItemPostFrame();
}

void CWeaponTurret::SpawnTurret(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

#ifndef CLIENT_DLL
	CNPC_FloorTurret* pTurret = dynamic_cast<CNPC_FloorTurret*>(CreateEntityByName("npc_turret_floor"));
	if (pTurret)
	{
		pTurret->SetName(AllocPooledString("spawnedTurret"));
		pTurret->SetOwnerEntity(pOwner);
		pTurret->Spawn();
		if (pHologram)
		{
			pTurret->Teleport(&pHologram->GetAbsOrigin(), &pHologram->GetAbsAngles(), NULL);
		}
		pTurret->Activate();
		// not needed, but allows the manhack to work properly when playing as freeman.
		pTurret->ChangeTeam(pOwner->GetTeamNumber());

		WeaponSound(SPECIAL1);
	}

	m_bStopMovingHologram = false;
#endif
	MoveHologram();

	DecrementAmmo(pOwner);
}

bool CWeaponTurret::Reload(void)
{
	WeaponIdle();
	return true;
}

void CWeaponTurret::PrimaryAttack(void)
{
	if (m_flNextPrimaryAttack > gpGlobals->curtime)
		return;

	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

#ifndef CLIENT_DLL
	if (pHologram)
	{
		if (pHologram->GetStatus() == CTurretHologram::TURRET_INVALIDPLACEMENT ||
			pHologram->GetStatus() == CTurretHologram::TURRET_TOOFAR)
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
			return;
		}

		m_bStopMovingHologram = true;
	}
#endif

	WeaponSound(SINGLE);
	SendWeaponAnim(ACT_SLAM_TRIPMINE_ATTACH);
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_flNextPrimaryAttack = gpGlobals->curtime + (SequenceDuration() * 0.3f);
	//sequence duration dictates turret spawn.
}

void CWeaponTurret::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	if (pHologram)
	{
		// rotate the build angles by 90 degrees ( final angle calculated after we network this )
		m_iDesiredBuildRotations++;
		m_iDesiredBuildRotations = m_iDesiredBuildRotations % 4;
	}
#endif

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : NULL - 
//-----------------------------------------------------------------------------
bool CWeaponTurret::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return BaseClass::Holster(pSwitchingTo);

	StopHologram();

#ifndef CLIENT_DLL
	m_iDesiredBuildRotations = 0;
	m_flCurrentBuildRotation = 0.0f;
#endif

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponTurret::Drop(const Vector& velocity)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	StopHologram();

#ifndef CLIENT_DLL
	m_iDesiredBuildRotations = 0;
	m_flCurrentBuildRotation = 0.0f;
#endif

	BaseClass::Drop(velocity);
}

void CWeaponTurret::StartHologram(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

#ifndef CLIENT_DLL
	pHologram = (CTurretHologram*)CBaseEntity::Create("turret_hologram", pOwner->GetAbsOrigin(), pOwner->GetAbsAngles(), pOwner);
	if (pHologram)
	{
		pHologram->SetOwnerEntity(pOwner);
		pHologram->Spawn();
	}

	MoveHologram();
#endif
}

void CWeaponTurret::MoveHologram(void)
{
#ifndef CLIENT_DLL
	if (m_flNextHologramMove > gpGlobals->curtime)
		return;

	if (pHologram == nullptr || pHologram == NULL)
	{
		//StartHologram will move back to us.
		StartHologram();
		return;
	}

	if (m_bStopMovingHologram)
		return;

	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (pHologram)
	{
		if (!pOwner->IsAlive())
		{
			StopHologram();
		}

		// Now attempt to drop into the world
		QAngle angles;
		trace_t tr;
		Vector forward;
		pOwner->EyeVectors(&forward);
		VectorAngles(forward, angles);
		angles.x = 0;
		angles.z = 0;
		AI_TraceLine(pOwner->EyePosition(),
			pOwner->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_NPCSOLID,
			pOwner, COLLISION_GROUP_NONE, &tr);

		if (tr.fraction != 1.0)
		{
			// Raise the end position a little up off the floor, place the npc and drop him down
			tr.endpos.z += 12;
			pHologram->SetAbsOrigin(tr.endpos);
			UTIL_DropToFloor(pHologram, MASK_NPCSOLID);
			//pHologram->SetAbsAngles(angles);

			// Calculate build angles
			QAngle vecAngles = vec3_angle;
			vecAngles.y = pOwner->EyeAngles().y;

			QAngle objAngles = vecAngles;

			SetAbsAngles(objAngles);

			float flBuildRotation = 90.0f * m_iDesiredBuildRotations;

			m_flCurrentBuildRotation = ApproachAngle(flBuildRotation, m_flCurrentBuildRotation, 250 * gpGlobals->frametime);

			objAngles.y = objAngles.y + m_flCurrentBuildRotation;

			pHologram->SetLocalAngles(objAngles);

			//pHologram->Teleport(&tr.endpos, &angles, NULL);
			UTIL_DropToFloor(pHologram, MASK_NPCSOLID);
			// Now check that this is a valid location for the new npc to be
			Vector	vUpBit = pHologram->GetAbsOrigin();
			vUpBit.z += 1;

			float enemyDelta = (pHologram->WorldSpaceCenter() - pOwner->WorldSpaceCenter()).Length();

			if (enemyDelta > TURRET_MAX_PLACEMENT_RANGE)
			{
				pHologram->SetStatus(CTurretHologram::TURRET_TOOFAR);
				return;
			}
			else if (enemyDelta < TURRET_MIN_PLACEMENT_RANGE)
			{
				pHologram->SetStatus(CTurretHologram::TURRET_INVALIDPLACEMENT);
				return;
			}

			//mins and maxs for the hologram's hull
			AI_TraceHull(pHologram->GetAbsOrigin(), vUpBit, pHologram->GetHullMins(), pHologram->GetHullMaxs(),
				MASK_NPCSOLID, pHologram, COLLISION_GROUP_NONE, &tr);

			if (tr.fraction < 1.0)
			{
				pHologram->SetStatus(CTurretHologram::TURRET_INVALIDPLACEMENT);
				return;
			}

			pHologram->SetStatus(CTurretHologram::TURRET_JUSTRIGHT);

			m_flNextHologramMove = gpGlobals->curtime + 0.01f;
		}
	}
	else
	{
		StartHologram();
	}
#endif
}

void CWeaponTurret::StopHologram(void)
{
#ifndef CLIENT_DLL
	if (pHologram == nullptr || pHologram == NULL)
		return;

	if (pHologram)
	{
		pHologram->SUB_Remove();
		pHologram = NULL;
	}
#endif
}

void CWeaponTurret::DecrementAmmo(CBaseCombatCharacter* pOwner)
{
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );

	CHL2MP_Player* pPlayer = ToHL2MPPlayer(GetOwner());

	if (pPlayer)
	{
		if (pPlayer->GetPlayerClass() != CLS_FREEMAN)
		{
			pPlayer->SwitchToNextBestWeapon(this);
		}
	}
}
