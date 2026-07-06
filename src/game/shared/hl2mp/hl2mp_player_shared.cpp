//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"

#ifdef CLIENT_DLL
#include "c_hl2mp_player.h"
#include "c_anticitizen_player_resource.h"
#include "prediction.h"
#define CRecipientFilter C_RecipientFilter
#else
#include "hl2mp_player.h"
#include "anticitizen_player_resource.h"
#endif

#include "hl2mp_gamerules.h"
#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "in_buttons.h"
#include "decals.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar hl2_normspeed;
extern ConVar hl2_sprintspeed;

#define	HL2_NORM_SPEED hl2_normspeed.GetFloat()
#define	HL2_SPRINT_SPEED hl2_sprintspeed.GetFloat()

extern ConVar sv_footsteps;

const char* pszCombineClasses[] =
{
	"metropolice",
	"combine_soldier",
	"combine_shotgunner",
	"combine_elite",
	NULL
};

const char* pszFreemanClasses[] =
{
	"freeman",
	NULL
};

const char *g_ppszPlayerSoundPrefixNames[PLAYER_SOUNDS_MAX] =
{
	"NPC_Citizen",
	"NPC_CombineS",
	"NPC_MetroPolice",
};

const char *CHL2MP_Player::GetPlayerModelSoundPrefix( void )
{
	return g_ppszPlayerSoundPrefixNames[m_iPlayerSoundType];
}

void CHL2MP_Player::PrecacheFootStepSounds( void )
{
	int iFootstepSounds = ARRAYSIZE( g_ppszPlayerSoundPrefixNames );
	int i;

	for ( i = 0; i < iFootstepSounds; ++i )
	{
		char szFootStepName[128];

		Q_snprintf( szFootStepName, sizeof( szFootStepName ), "%s.RunFootstepLeft", g_ppszPlayerSoundPrefixNames[i] );
		PrecacheScriptSound( szFootStepName );

		Q_snprintf( szFootStepName, sizeof( szFootStepName ), "%s.RunFootstepRight", g_ppszPlayerSoundPrefixNames[i] );
		PrecacheScriptSound( szFootStepName );
	}
}

void CHL2MP_Player::PrecacheADSSounds(void)
{
	int iFootstepSounds = ARRAYSIZE(g_ppszPlayerSoundPrefixNames);
	int i;

	for (i = 0; i < iFootstepSounds; ++i)
	{
		char szFootStepName[128];

		Q_snprintf(szFootStepName, sizeof(szFootStepName), "%s.ADSIn", g_ppszPlayerSoundPrefixNames[i]);
		PrecacheScriptSound(szFootStepName);

		Q_snprintf(szFootStepName, sizeof(szFootStepName), "%s.ADSOut", g_ppszPlayerSoundPrefixNames[i]);
		PrecacheScriptSound(szFootStepName);
	}
}

extern CMoveData* g_pMoveData;

//-----------------------------------------------------------------------------
// Consider the weapon's built-in accuracy, this character's proficiency with
// the weapon, and the status of the target. Use this information to determine
// how accurately to shoot at the target.
//-----------------------------------------------------------------------------
Vector CHL2MP_Player::GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget )
{
	// this is incredibly hacky.
	if (pWeapon)
	{
		int proficiency = WEAPON_PROFICIENCY_PERFECT;

		if (GetPlayerClass() != CLS_FREEMAN)
		{
			const CAnticitizen_FilePlayerClassInfo_t& info = GetPlayerClassInfo();

			if (info.iClassType < CLS_TYPE_HIGH_TIER)
			{
				float groundspeed = Vector2DLength(g_pMoveData->m_vecVelocity.AsVector2D());
				bool movingalongground = ((GetFlags() & FL_ONGROUND) && groundspeed > 0.0001f);

				if (!(GetFlags() & FL_ONGROUND))
				{
					proficiency = WEAPON_PROFICIENCY_AVERAGE;
				}
				else if (movingalongground)
				{
					proficiency = WEAPON_PROFICIENCY_GOOD;
				}
				else
				{
					proficiency = WEAPON_PROFICIENCY_VERY_GOOD;
				}

				if (pWeapon->IsIronsighted())
				{
					proficiency = WEAPON_PROFICIENCY_PERFECT;
				}
			}
			else
			{
				if (pWeapon->IsIronsighted())
				{
					proficiency = WEAPON_PROFICIENCY_PERFECT;
				}
				else
				{
					proficiency = WEAPON_PROFICIENCY_VERY_GOOD;
				}
			}
		}

		const WeaponProficiencyInfo_t* pProficiencyValues = pWeapon->GetProficiencyValues();

		if (pProficiencyValues == NULL)
		{
			DevWarning("%s has no proficiency value table!\n", pWeapon->GetClassname());

			// Weapon proficiency table. Keep this in sync with WeaponProficiency_t enum in the header!!
			static WeaponProficiencyInfo_t g_BaseWeaponProficiencyTable[] =
			{
				{ 2.50, 1.0	},
				{ 2.00, 1.0	},
				{ 1.50, 1.0	},
				{ 1.25, 1.0 },
				{ 1.00, 1.0	},
			};

			COMPILE_TIME_ASSERT(ARRAYSIZE(g_BaseWeaponProficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

			pProficiencyValues = g_BaseWeaponProficiencyTable;
		}

		float scale = (pProficiencyValues)[proficiency].spreadscale;

		return pWeapon->GetBulletSpread() * scale;
	}
	
	return VECTOR_CONE_15DEGREES;
}

void CHL2MP_Player::UpdateStepSound(surfacedata_t* psurface, const Vector& vecOrigin, const Vector& vecVelocity)
{
	bool bWalking;
	float fvol;
	Vector knee;
	Vector feet;
	float height;
	float speed;
	float velrun;
	float velwalk;
	int	fLadder;

	if (m_flStepSoundTime > 0)
	{
		m_flStepSoundTime -= 1000.0f * gpGlobals->frametime;
		if (m_flStepSoundTime < 0)
		{
			m_flStepSoundTime = 0;
		}
	}

	if (m_flStepSoundTime > 0)
		return;

	if (GetFlags() & (FL_FROZEN | FL_ATCONTROLS))
		return;

	if (GetMoveType() == MOVETYPE_NOCLIP || GetMoveType() == MOVETYPE_OBSERVER)
		return;

	if (!sv_footsteps.GetFloat())
		return;

	speed = VectorLength(vecVelocity);
	float groundspeed = Vector2DLength(vecVelocity.AsVector2D());

	// determine if we are on a ladder
	fLadder = (GetMoveType() == MOVETYPE_LADDER);

	GetStepSoundVelocities(&velwalk, &velrun);

	bool onground = (GetFlags() & FL_ONGROUND);
	bool movingalongground = (groundspeed > 0.0001f);

	bool moving_fast_enough = true;

	// To hear step sounds you must be either on a ladder or moving along the ground AND
	// You must be moving fast enough

	if (!moving_fast_enough || !(fLadder || (onground && movingalongground)))
		return;

	//	MoveHelper()->PlayerSetAnimation( PLAYER_WALK );

	bWalking = speed < velrun;

	VectorCopy(vecOrigin, knee);
	VectorCopy(vecOrigin, feet);

	height = GetPlayerMaxs()[2] - GetPlayerMins()[2];

	knee[2] = vecOrigin[2] + 0.2 * height;

	// find out what we're stepping in or on...
	if (fLadder)
	{
		psurface = GetLadderSurface(vecOrigin);
		fvol = 0.5;

		SetStepSoundTime(STEPSOUNDTIME_ON_LADDER, bWalking);
	}
#ifdef CSTRIKE_DLL
	else if (enginetrace->GetPointContents(knee) & MASK_WATER)  // we want to use the knee for Cstrike, not the waist
#else
	else if (GetWaterLevel() == WL_Waist)
#endif // CSTRIKE_DLL
	{
		static int iSkipStep = 0;

		if (iSkipStep == 0)
		{
			iSkipStep++;
			return;
		}

		if (iSkipStep++ == 3)
		{
			iSkipStep = 0;
		}
		psurface = physprops->GetSurfaceData(physprops->GetSurfaceIndex("wade"));
		fvol = 0.65;
		SetStepSoundTime(STEPSOUNDTIME_WATER_KNEE, bWalking);
	}
	else if (GetWaterLevel() == WL_Feet)
	{
		psurface = physprops->GetSurfaceData(physprops->GetSurfaceIndex("water"));
		fvol = bWalking ? 0.2 : 0.5;

		SetStepSoundTime(STEPSOUNDTIME_WATER_FOOT, bWalking);
	}
	else
	{
		if (!psurface)
			return;

		SetStepSoundTime(STEPSOUNDTIME_NORMAL, bWalking);

		switch (psurface->game.material)
		{
		default:
		case CHAR_TEX_CONCRETE:
			fvol = bWalking ? 0.2 : 0.5;
			break;

		case CHAR_TEX_METAL:
			fvol = bWalking ? 0.2 : 0.5;
			break;

		case CHAR_TEX_DIRT:
			fvol = bWalking ? 0.25 : 0.55;
			break;

		case CHAR_TEX_VENT:
			fvol = bWalking ? 0.4 : 0.7;
			break;

		case CHAR_TEX_GRATE:
			fvol = bWalking ? 0.2 : 0.5;
			break;

		case CHAR_TEX_TILE:
			fvol = bWalking ? 0.2 : 0.5;
			break;

		case CHAR_TEX_SLOSH:
			fvol = bWalking ? 0.2 : 0.5;
			break;
		}
	}

	// play the sound
	// 65% volume if ducking
	if (GetFlags() & FL_DUCKING)
	{
		fvol *= 0.65;
	}

	// if we're moving fast enough, our playersounds should change from BaseClass to ours.
	bool moving_fast_enough_sndplay = (speed >= velwalk);

	if (!moving_fast_enough_sndplay)
	{
		BaseClass::PlayStepSound(feet, psurface, fvol, false);
	}
	else
	{
		PlayStepSound(feet, psurface, fvol, false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : step - 
//			fvol - 
//			force - force sound to play
//-----------------------------------------------------------------------------
void CHL2MP_Player::PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force )
{
	if (m_iPlayerSoundType == (int)PLAYER_SOUNDS_CITIZEN)
	{
		BaseClass::PlayStepSound(vecOrigin, psurface, fvol, force);
		return;
	}

	if ( gpGlobals->maxClients > 1 && !sv_footsteps.GetFloat() )
		return;

#if defined( CLIENT_DLL )
	// during prediction play footstep sounds only once
	if ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;
#endif

	if ( GetFlags() & FL_DUCKING )
		return;

	if (!psurface)
		return;

	m_Local.m_nStepside = !m_Local.m_nStepside;

	char szStepSound[128];

	if ( m_Local.m_nStepside )
	{
		Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.RunFootstepLeft", GetPlayerModelSoundPrefix());
	}
	else
	{
		Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.RunFootstepRight", GetPlayerModelSoundPrefix());
	}

	CSoundParameters params;
	if ( GetParametersForSound( szStepSound, params, NULL ) == false )
		return;

	CRecipientFilter filter;
	filter.AddRecipientsByPAS( vecOrigin );

#ifndef CLIENT_DLL
	// im MP, server removed all players in origins PVS, these players 
	// generate the footsteps clientside
	if ( gpGlobals->maxClients > 1 )
		filter.RemoveRecipientsByPVS( vecOrigin );
#endif

	EmitSound_t ep;
	// combine and metrocop footstep sounds 
	ep.m_nChannel = CHAN_STATIC;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = fvol;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
	// play the per-material footsteps.
	BaseClass::PlayStepSound(vecOrigin, psurface, fvol, force);
}

extern ConVar sv_maxspeed;

void CHL2MP_Player::HandleSpeedChanges(CMoveData* mv)
{
	if (GetPlayerClass() > CLS_INVALID)
	{
		const CAnticitizen_FilePlayerClassInfo_t& info = GetPlayerClassInfo();

		if (IsSuitEquipped())
		{
			int nChangedButtons = mv->m_nButtons ^ mv->m_nOldButtons;

			bool bJustPressedSpeed = !!(nChangedButtons & IN_SPEED);

			const bool bWantSprint = (CanSprint() && (mv->m_nButtons & IN_SPEED));
			const bool bWantsToChangeSprinting = (m_HL2Local.m_bNewSprinting != bWantSprint) && (nChangedButtons & IN_SPEED) != 0;

			bool bSprinting = m_HL2Local.m_bNewSprinting;
			if (bWantsToChangeSprinting)
			{
				if (bWantSprint)
				{
					if (m_HL2Local.m_flSuitPower < 10.0f)
					{
						if (bJustPressedSpeed)
						{
							CPASAttenuationFilter filter(this);
							filter.UsePredictionRules();
							EmitSound(filter, entindex(), "HL2Player.SprintNoPower");
						}
					}
					else
					{
						bSprinting = true;
					}
				}
				else
				{
					bSprinting = false;
				}
			}

			if (m_HL2Local.m_flSuitPower < 0.01)
			{
				bSprinting = false;
			}

			m_HL2Local.m_bNewSprinting = bSprinting;

			if (bSprinting)
			{
				if (bJustPressedSpeed)
				{
					CPASAttenuationFilter filter(this);
					filter.UsePredictionRules();
					EmitSound(filter, entindex(), "HL2Player.SprintStart");
				}
				mv->m_flClientMaxSpeed = info.flSprintSpeed;
			}
			else
			{
				mv->m_flClientMaxSpeed = info.flNormSpeed;
			}
		}
		else
		{
			if (GetActiveWeapon() && GetActiveWeapon()->IsIronsighted())
			{
				mv->m_flClientMaxSpeed = info.flADSSpeed;
			}
			else
			{
				mv->m_flClientMaxSpeed = info.flNormSpeed;
			}
		}

		mv->m_flMaxSpeed = sv_maxspeed.GetFloat();
	}
}

extern CSuitPowerDevice SuitDeviceSprint;

void CHL2MP_Player::ReduceTimers(CMoveData* mv)
{
	if (GetPlayerClass() > CLS_INVALID)
	{
		const CAnticitizen_FilePlayerClassInfo_t& info = GetPlayerClassInfo();

		if (IsSuitEquipped())
		{
			bool bSprinting = mv->m_flClientMaxSpeed == info.flSprintSpeed;

			if (bSprinting)
			{
				SuitPower_AddDevice(SuitDeviceSprint);
			}
			else
			{
				SuitPower_RemoveDevice(SuitDeviceSprint);
			}

			SuitPower_Update();
		}
	}
}

void CHL2MP_Player::SetPlayerClass(int playerclass)
{
	m_iPlayerClass = playerclass;
}

int CHL2MP_Player::GetPlayerClass(void)
{
	return m_iPlayerClass;
}

const CAnticitizen_FilePlayerClassInfo_t& CHL2MP_Player::GetPlayerClassInfo(void)
{
	return g_Anticitizen_PR->GetPlayerClassInfo(GetPlayerClass());
}

extern ConVar hl2mp_avoidteammates;

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : collisionGroup - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHL2MP_Player::ShouldCollide(int collisionGroup, int contentsMask) const
{
	if (HL2MPRules()->IsTeamplay())
	{
		if ((collisionGroup == COLLISION_GROUP_PLAYER || collisionGroup == COLLISION_GROUP_PLAYER_MOVEMENT) && hl2mp_avoidteammates.GetBool())
		{
			switch (GetTeamNumber())
			{
			case TEAM_FREEMAN:
				if (!(contentsMask & CONTENTS_TEAM1))
					return false;
				break;

			case TEAM_COMBINE:
				if (!(contentsMask & CONTENTS_TEAM2))
					return false;
				break;
			}
		}
	}

	return BaseClass::ShouldCollide(collisionGroup, contentsMask);
}