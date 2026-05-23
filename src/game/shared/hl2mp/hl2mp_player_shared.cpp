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

//-----------------------------------------------------------------------------
// Consider the weapon's built-in accuracy, this character's proficiency with
// the weapon, and the status of the target. Use this information to determine
// how accurately to shoot at the target.
//-----------------------------------------------------------------------------
Vector CHL2MP_Player::GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget )
{
	if ( pWeapon )
		return pWeapon->GetBulletSpread( WEAPON_PROFICIENCY_PERFECT );
	
	return VECTOR_CONE_15DEGREES;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : step - 
//			fvol - 
//			force - force sound to play
//-----------------------------------------------------------------------------
void CHL2MP_Player::PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force )
{
	if ( gpGlobals->maxClients > 1 && !sv_footsteps.GetFloat() )
		return;

#if defined( CLIENT_DLL )
	// during prediction play footstep sounds only once
	if ( !prediction->IsFirstTimePredicted() )
		return;
#endif

	if ( GetFlags() & FL_DUCKING )
		return;

	m_Local.m_nStepside = !m_Local.m_nStepside;

	char szStepSound[128];

	if ( m_Local.m_nStepside )
	{
		Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.RunFootstepLeft", g_ppszPlayerSoundPrefixNames[m_iPlayerSoundType] );
	}
	else
	{
		Q_snprintf( szStepSound, sizeof( szStepSound ), "%s.RunFootstepRight", g_ppszPlayerSoundPrefixNames[m_iPlayerSoundType] );
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
	ep.m_nChannel = CHAN_BODY;
	ep.m_pSoundName = params.soundname;
	ep.m_flVolume = fvol;
	ep.m_SoundLevel = params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound( filter, entindex(), ep );
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