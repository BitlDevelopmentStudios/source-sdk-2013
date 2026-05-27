//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "hl2mp_player.h"
#include "globalstate.h"
#include "game.h"
#include "gamerules.h"
#include "hl2mp_player_shared.h"
#include "predicted_viewmodel.h"
#include "in_buttons.h"
#include "hl2mp_gamerules.h"
#include "KeyValues.h"
#include "team.h"
#include "weapon_hl2mpbase.h"
#include "grenade_satchel.h"
#include "eventqueue.h"
#include "gamestats.h"
#include "tier0/vprof.h"
#include "bone_setup.h"
#include "ammodef.h"
#include "NextBot.h"

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int g_iLastCitizenModel = 0;
int g_iLastCombineModel = 0;

CBaseEntity	 *g_pLastCombineSpawn = NULL;
CBaseEntity	 *g_pLastRebelSpawn = NULL;
extern CBaseEntity				*g_pLastSpawn;

ConVar hl2mp_spawn_frag_fallback_radius( "hl2mp_spawn_frag_fallback_radius", "48", FCVAR_NONE, "If no spawns are available, kill players with this radius to allow new players to spawn." );

#define HL2MP_COMMAND_MAX_RATE 0.3

#define CYCLELATCH_UPDATE_INTERVAL	0.2f

void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );

LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

LINK_ENTITY_TO_CLASS( info_player_combine, CPointEntity );
LINK_ENTITY_TO_CLASS( info_player_rebel, CPointEntity );

extern void SendProxy_Origin(const SendProp* pProp, const void* pStruct, const void* pData, DVariant* pOut, int iElement, int objectID);

//Tony; this should ideally be added to dt_send.cpp
extern void* SendProxy_SendNonLocalDataTable(const SendProp* pProp, const void* pStruct, const void* pVarData, CSendProxyRecipients* pRecipients, int objectID);
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_SendNonLocalDataTable);

BEGIN_SEND_TABLE_NOBASE(CHL2MP_Player, DT_HL2MPLocalPlayerExclusive)
	// send a hi-res origin to the local player for use in prediction
	SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_NOSCALE | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin),
	SendPropFloat(SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f),
	//	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE(CHL2MP_Player, DT_HL2MPNonLocalPlayerExclusive)
	// send a lo-res origin to other players
	SendPropVector(SENDINFO(m_vecOrigin), -1, SPROP_COORD_MP_LOWPRECISION | SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin),
	SendPropFloat(SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f),
	SendPropAngle(SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN),
	// Only need to latch cycle for other players
	// If you increase the number of bits networked, make sure to also modify the code below and in the client class.
	SendPropInt(SENDINFO(m_cycleLatch), 4, SPROP_UNSIGNED),
END_SEND_TABLE()

IMPLEMENT_SERVERCLASS_ST(CHL2MP_Player, DT_HL2MP_Player)
	SendPropExclude( "DT_BaseEntity", "m_vecOrigin" ),

	// misyl:
	// m_flMaxspeed is fully predicted by the client and the client's
	// maxspeed is sent in the user message.
	// Other games like DOD, etc don't use this var at all and just fully
	// predict in GameMovement, but the HL2 codebase doesn't do that and modifies this
	// on the player.
	// So, just never send it, and don't predict it on the client either.
	SendPropExclude( "DT_BasePlayer", "m_flMaxspeed" ),

	// Data that only gets sent to the local player
	SendPropDataTable( "hl2mplocaldata", 0, &REFERENCE_SEND_TABLE( DT_HL2MPLocalPlayerExclusive ), SendProxy_SendLocalDataTable ),

	// Data that gets sent to all other players
	SendPropDataTable( "hl2mpnonlocaldata", 0, &REFERENCE_SEND_TABLE( DT_HL2MPNonLocalPlayerExclusive ), SendProxy_SendNonLocalDataTable ),

	SendPropEHandle( SENDINFO( m_hRagdoll ) ),
	SendPropInt( SENDINFO( m_iSpawnInterpCounter), 4 ),
	SendPropInt( SENDINFO( m_iPlayerSoundType), 3 ),

	SendPropInt(SENDINFO(m_iLives) ),

	SendPropInt(SENDINFO(m_iPlayerClass), 4),

	SendPropFloat(SENDINFO(m_flNormalSpeed)),
	SendPropFloat(SENDINFO(m_flSprintSpeed)),
	
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude("DT_BaseAnimating", "m_flPlaybackRate"),
	SendPropExclude("DT_BaseAnimating", "m_nSequence"),
	SendPropExclude("DT_BaseEntity", "m_angRotation"),
	SendPropExclude("DT_BaseAnimatingOverlay", "overlay_vars"),

	SendPropExclude("DT_BaseEntity", "m_vecOrigin"),

	// playeranimstate and clientside animation takes care of these on the client
	SendPropExclude("DT_ServerAnimationData", "m_flCycle"),
	SendPropExclude("DT_AnimTimeMustBeFirst", "m_flAnimTime"),

	SendPropExclude("DT_BaseFlex", "m_flexWeight"),
	SendPropExclude("DT_BaseFlex", "m_blinktoggle"),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

	// Data that only gets sent to the local player
	SendPropDataTable("hl2mplocaldata", 0, &REFERENCE_SEND_TABLE(DT_HL2MPLocalPlayerExclusive), SendProxy_SendLocalDataTable),

	// Data that gets sent to all other players
	SendPropDataTable("hl2mpnonlocaldata", 0, &REFERENCE_SEND_TABLE(DT_HL2MPNonLocalPlayerExclusive), SendProxy_SendNonLocalDataTable),

	SendPropEHandle(SENDINFO(m_hRagdoll)),
	SendPropInt(SENDINFO(m_iSpawnInterpCounter), 4),
	SendPropInt(SENDINFO(m_iPlayerSoundType), 3),
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CHL2MP_Player, CHL2_Player, "Half-Life 2: Deathmatch Player" )
END_SCRIPTDESC();

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

CHL2MP_Player::CHL2MP_Player()
{
	//Tony; create our player animation state.
	m_PlayerAnimState = CreateHL2MPPlayerAnimState(this);
	UseClientSideAnimation();

	m_angEyeAngles.Init();

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	m_iSpawnInterpCounter = 0;

    m_bEnterObserver = false;
	m_bReady = false;

	m_bInitialSpawn = true;

	m_iPlayerClass = CLS_INVALID;
	m_bChosenClass = false;

	m_iLives = -1;

	m_cycleLatch = 0;
	m_cycleLatchTimer.Invalidate();

	m_grenadeReloadTimer.Invalidate();
	m_ballReloadTimer.Invalidate();
	m_hackReloadTimer.Invalidate();

	BaseClass::ChangeTeam( 0 );
	
	//UseClientSideAnimation();
}

CHL2MP_Player::~CHL2MP_Player( void )
{
	m_PlayerAnimState->Release();
}

void CHL2MP_Player::UpdateOnRemove( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void CHL2MP_Player::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel ( "sprites/glow01.vmt" );

	PrecacheFootStepSounds();
	PrecacheADSSounds();

	PrecacheScriptSound( "NPC_MetroPolice.Die" );
	PrecacheScriptSound( "NPC_CombineS.Die" );
	PrecacheScriptSound( "NPC_Citizen.die" );
}

void CHL2MP_Player::GiveAllItems( void )
{
	EquipSuit();

	CBasePlayer::GiveAmmo(255, "Pistol");
	CBasePlayer::GiveAmmo(255, "AR2");
	CBasePlayer::GiveAmmo(5, "AR2AltFire");
	CBasePlayer::GiveAmmo(255, "SMG1");
	CBasePlayer::GiveAmmo(1, "smg1_grenade");
	CBasePlayer::GiveAmmo(255, "Buckshot");
	CBasePlayer::GiveAmmo(32, "357");
	CBasePlayer::GiveAmmo(3, "rpg_round");
	CBasePlayer::GiveAmmo(16, "XBowBolt");

	CBasePlayer::GiveAmmo(1, "grenade");
	CBasePlayer::GiveAmmo(2, "slam");

	GiveNamedItem("weapon_crowbar");
	GiveNamedItem("weapon_stunstick");
	GiveNamedItem("weapon_pistol");
	GiveNamedItem("weapon_357");

	GiveNamedItem("weapon_smg1");
	GiveNamedItem("weapon_ar2");

	GiveNamedItem("weapon_shotgun");
	GiveNamedItem("weapon_frag");

	GiveNamedItem("weapon_crossbow");

	GiveNamedItem("weapon_rpg");

	GiveNamedItem("weapon_slam");

	GiveNamedItem("weapon_physcannon");
}

void CHL2MP_Player::GiveAllWeapons(void)
{
	CBasePlayer::GiveAmmo(255, "Pistol");
	CBasePlayer::GiveAmmo(255, "AR2");
	CBasePlayer::GiveAmmo(5, "AR2AltFire");
	CBasePlayer::GiveAmmo(255, "SMG1");
	CBasePlayer::GiveAmmo(1, "smg1_grenade");
	CBasePlayer::GiveAmmo(255, "Buckshot");
	CBasePlayer::GiveAmmo(32, "357");
	CBasePlayer::GiveAmmo(3, "rpg_round");
	CBasePlayer::GiveAmmo(16, "XBowBolt");

	CBasePlayer::GiveAmmo(1, "grenade");

	GiveNamedItem("weapon_crowbar");
	GiveNamedItem("weapon_pistol");
	GiveNamedItem("weapon_357");

	GiveNamedItem("weapon_smg1");
	GiveNamedItem("weapon_ar2");

	GiveNamedItem("weapon_shotgun");
	GiveNamedItem("weapon_frag");

	GiveNamedItem("weapon_crossbow");

	GiveNamedItem("weapon_rpg");

	GiveNamedItem("weapon_physcannon");

	const char* szDefaultWeaponName = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_defaultweapon");

	CBaseCombatWeapon* pDefaultWeapon = Weapon_OwnsThisType(szDefaultWeaponName);

	if (pDefaultWeapon)
	{
		Weapon_Switch(pDefaultWeapon);
	}
	else
	{
		Weapon_Switch(Weapon_OwnsThisType("weapon_physcannon"));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets HL2 specific defaults.
//-----------------------------------------------------------------------------
void CHL2MP_Player::Spawn(void)
{
	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	BaseClass::Spawn();

	if (!m_bChosenClass)
	{
		ChangeTeam(TEAM_SPECTATOR);
	}
	else
	{
		if (m_bInitialSpawn)
		{
			m_bInitialSpawn = false;
		}

		LoadClass(GetPlayerClass());
	}
	
	if ( !IsObserver() )
	{
		pl.deadflag = false;
		RemoveSolidFlags( FSOLID_NOT_SOLID );

		RemoveEffects( EF_NODRAW );
	}

	m_nRenderFX = kRenderNormal;

	m_flNextPainSoundTime = 0;
	m_Local.m_iHideHUD = 0;
	
	AddFlag(FL_ONGROUND); // set the player on the ground at the start of the round.

	m_impactEnergyScale = HL2MPPLAYER_PHYSDAMAGE_SCALE;

	if ( HL2MPRules()->IsIntermission() )
	{
		AddFlag( FL_FROZEN );
	}
	else
	{
		RemoveFlag( FL_FROZEN );
	}

	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	m_Local.m_bDucked = false;

	SetPlayerUnderwater(false);

	m_bReady = false;

	m_cycleLatchTimer.Start(CYCLELATCH_UPDATE_INTERVAL);

	//Tony; do the spawn animevent
	DoAnimationEvent(PLAYERANIMEVENT_SPAWN);

	if (GetTeamNumber() != TEAM_SPECTATOR)
	{
		StopObserverMode();
	}
	else
	{
		// Ms - If we are spectating then go roaming
		StartObserverMode(OBS_MODE_ROAMING);
	}
}

ConVar hl2mp_allow_pickup( "hl2mp_allow_pickup", "0", FCVAR_GAMEDLL );

void CHL2MP_Player::PickupObject( CBaseEntity* pObject, bool bLimitMassAndSize )
{
	if ( !hl2mp_allow_pickup.GetBool() )
		return;

	return BaseClass::PickupObject( pObject, bLimitMassAndSize );
}

void CHL2MP_Player::SetupPlayerSoundsByModel( const char *pModelName )
{
	if ( Q_stristr( pModelName, "models/human") )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_CITIZEN;
	}
	else if ( Q_stristr(pModelName, "police" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_METROPOLICE;
	}
	else if ( Q_stristr(pModelName, "combine" ) )
	{
		m_iPlayerSoundType = (int)PLAYER_SOUNDS_COMBINESOLDIER;
	}
}

bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	return bRet;
}

void CHL2MP_Player::PreThink( void )
{
	BaseClass::PreThink();
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
}

extern ConVar sk_resource_regen_time;

void CHL2MP_Player::PostThink( void )
{
	BaseClass::PostThink();
	
	if ( GetFlags() & FL_DUCKING )
	{
		SetCollisionBounds( VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX );
	}

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles(angles);

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();
	m_PlayerAnimState->Update(m_angEyeAngles[YAW], m_angEyeAngles[PITCH]);

	if ((GetPlayerClass() > CLS_INVALID))
	{
		const CAnticitizen_FilePlayerClassInfo_t& info = GetPlayerClassInfo();

		if (info.iGrenades == -1)
		{
			if (GetAmmoCount("grenade") < 1)
			{
				// If it's been longer than three seconds, reload
				if (m_grenadeReloadTimer.HasElapsedSinceStart())
				{
					// Just load the clip with no animations
					CBasePlayer::GiveAmmo(1, "grenade");
					m_grenadeReloadTimer.Invalidate();
				}
				else
				{
					if (!m_grenadeReloadTimer.HasStarted())
					{
						m_grenadeReloadTimer.Start(sk_resource_regen_time.GetFloat());
					}
				}
			}
		}

		if (info.iCombineBalls == -1)
		{
			if (GetAmmoCount("AR2AltFire") < 1)
			{
				// If it's been longer than three seconds, reload
				if (m_ballReloadTimer.HasElapsedSinceStart())
				{
					// Just load the clip with no animations
					CBasePlayer::GiveAmmo(1, "AR2AltFire");
					m_ballReloadTimer.Invalidate();
				}
				else
				{
					if (!m_ballReloadTimer.HasStarted())
					{
						m_ballReloadTimer.Start(sk_resource_regen_time.GetFloat());
					}
				}
			}
		}

		if (info.iManhacks == -1)
		{
			if (GetAmmoCount("Manhacks") < 1)
			{
				// If it's been longer than three seconds, reload
				if (m_hackReloadTimer.HasElapsedSinceStart())
				{
					// Just load the clip with no animations
					SetBodygroup(1, 1);
					CBasePlayer::GiveAmmo(1, "Manhacks");
					m_hackReloadTimer.Invalidate();
				}
				else
				{
					SetBodygroup(1, 0);

					if (!m_hackReloadTimer.HasStarted())
					{
						m_hackReloadTimer.Start(sk_resource_regen_time.GetFloat());
					}
				}
			}
		}
	}
	
	if (IsAlive() && m_cycleLatchTimer.IsElapsed())
	{
		m_cycleLatchTimer.Start(CYCLELATCH_UPDATE_INTERVAL);
		// Compress the cycle into 4 bits. Can represent 0.0625 in steps which is enough.
		m_cycleLatch.GetForModify() = 16 * GetCycle();
	}
}

void CHL2MP_Player::PlayerDeathThink()
{
	if( !IsObserver() )
	{
		BaseClass::PlayerDeathThink();
	}
}

void CHL2MP_Player::FireBullets ( const FireBulletsInfo_t &info )
{
	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation( this, this->GetCurrentCommand() );

	FireBulletsInfo_t modinfo = info;

	CWeaponHL2MPBase *pWeapon = dynamic_cast<CWeaponHL2MPBase *>( GetActiveWeapon() );

	if ( pWeapon )
	{
		modinfo.m_iPlayerDamage = modinfo.m_flDamage = pWeapon->GetHL2MPWpnData().m_iPlayerDamage;
	}

	BaseClass::FireBullets( modinfo );

	// Move other players back to history positions based on local player's lag
	lagcompensation->FinishLagCompensation( this );

	if ( pWeapon )
		this->OnMyWeaponFired( pWeapon );
}

void CHL2MP_Player::OnMyWeaponFired( CBaseCombatWeapon* weapon )
{
	BaseClass::OnMyWeaponFired( weapon );

	TheNextBots().OnWeaponFired( this, weapon );
}

extern ConVar sv_maxunlag;

bool CHL2MP_Player::WantsLagCompensationOnEntity( const CBasePlayer *pPlayer, const CUserCmd *pCmd, const CBitVec<MAX_EDICTS> *pEntityTransmitBits ) const
{
	// If this entity hasn't been transmitted to us and acked, then don't bother lag compensating it.
	if ( pEntityTransmitBits && !pEntityTransmitBits->Get( pPlayer->entindex() ) )
		return false;

	const Vector &vMyOrigin = GetAbsOrigin();
	const Vector &vHisOrigin = pPlayer->GetAbsOrigin();

	// get max distance player could have moved within max lag compensation time, 
	// multiply by 1.5 to to avoid "dead zones"  (sqrt(2) would be the exact value)
	float maxDistance = 1.5 * pPlayer->MaxSpeed() * sv_maxunlag.GetFloat();

	// If the player is within this distance, lag compensate them in case they're running past us.
	if ( vHisOrigin.DistTo( vMyOrigin ) < maxDistance )
		return true;

	// If their origin is not within a 45 degree cone in front of us, no need to lag compensate.
	Vector vForward;
	AngleVectors( pCmd->viewangles, &vForward );
	
	Vector vDiff = vHisOrigin - vMyOrigin;
	VectorNormalize( vDiff );

	float flCosAngle = 0.707107f;	// 45 degree angle
	if ( vForward.Dot( vDiff ) < flCosAngle )
		return false;

	return true;
}

Activity CHL2MP_Player::TranslateTeamActivity( Activity ActToTranslate )
{
	if ( m_iModelType == TEAM_COMBINE )
		 return ActToTranslate;
	
	if ( ActToTranslate == ACT_RUN )
		 return ACT_RUN_AIM_AGITATED;

	if ( ActToTranslate == ACT_IDLE )
		 return ACT_IDLE_AIM_AGITATED;

	if ( ActToTranslate == ACT_WALK )
		 return ACT_WALK_AIM_AGITATED;

	return ActToTranslate;
}

extern ConVar hl2_normspeed;

extern int	gEvilImpulse101;
//-----------------------------------------------------------------------------
// Purpose: Player reacts to bumping a weapon. 
// Input  : pWeapon - the weapon that the player bumped into.
// Output : Returns true if player picked up the weapon
//-----------------------------------------------------------------------------
bool CHL2MP_Player::BumpWeapon( CBaseCombatWeapon *pWeapon )
{
	CBaseCombatCharacter *pOwner = pWeapon->GetOwner();

	// Can I have this weapon type?
	if ( !IsAllowedToPickupWeapons() )
		return false;

	if ( pOwner || !Weapon_CanUse( pWeapon ) || !g_pGameRules->CanHavePlayerItem( this, pWeapon ) )
	{
		if ( gEvilImpulse101 )
		{
			UTIL_Remove( pWeapon );
		}
		return false;
	}

	// Don't let the player fetch weapons through walls (use MASK_SOLID so that you can't pickup through windows)
	if( !pWeapon->FVisible( this, MASK_SOLID ) && !(GetFlags() & FL_NOTARGET) )
	{
		return false;
	}

	bool bOwnsWeaponAlready = !!Weapon_OwnsThisType( pWeapon->GetClassname(), pWeapon->GetSubType());

	if ( bOwnsWeaponAlready == true ) 
	{
		//If we have room for the ammo, then "take" the weapon too.
		 if ( Weapon_EquipAmmoOnly( pWeapon ) )
		 {
			 pWeapon->CheckRespawn();

			 UTIL_Remove( pWeapon );
			 return true;
		 }
		 else
		 {
			 return false;
		 }
	}

	pWeapon->CheckRespawn();
	Weapon_Equip( pWeapon );

	return true;
}

void CHL2MP_Player::ChangeTeam( int iTeam )
{
	BaseClass::ChangeTeam( iTeam );

	if ( iTeam == TEAM_SPECTATOR )
	{
		RemoveAllItems( true );
		State_Transition( STATE_OBSERVER_MODE );
	}
	else
	{
		StopObserverMode();
		State_Transition(STATE_ACTIVE);
	}
}

void CHL2MP_Player::LoadClass(int iClass)
{
	if (!IsAllowedToPickupWeapons())
	{
		SetPreventWeaponPickup(false);
	}

	if (GetPlayerClass() > CLS_INVALID)
	{
		const CAnticitizen_FilePlayerClassInfo_t& pPlayerClassInfo = GetPlayerClassInfo();

		if (pPlayerClassInfo.m_szPlayerModel[0])
		{
			PrecacheModel(pPlayerClassInfo.m_szPlayerModel);
			SetModel(pPlayerClassInfo.m_szPlayerModel);
			SetupPlayerSoundsByModel(pPlayerClassInfo.m_szPlayerModel);
		}

		if (pPlayerClassInfo.m_szCArmModel[0])
		{
			PrecacheModel(pPlayerClassInfo.m_szCArmModel);

			CBaseViewModel* pHands = GetViewModel(1);

			if (pHands)
			{
				pHands->SetModel(pPlayerClassInfo.m_szCArmModel);

				if (pPlayerClassInfo.iCArmSkin > -1)
				{
					pHands->SetSkin(pPlayerClassInfo.iCArmSkin);
				}
			}
		}

		if (pPlayerClassInfo.iHealth > 0)
		{
			SetHealth(pPlayerClassInfo.iHealth);
			SetMaxHealth(pPlayerClassInfo.iHealth);
		}

		if (pPlayerClassInfo.iLives > 0)
		{
			SetLifeCount(pPlayerClassInfo.iLives);
		}

		if (pPlayerClassInfo.bSuit)
		{
			EquipSuit();

			if (pPlayerClassInfo.iSuitArmor > 0)
			{
				SetArmorValue(pPlayerClassInfo.iSuitArmor);
			}
		}

		if (pPlayerClassInfo.bAllWeapons)
		{
			GiveAllWeapons();
		}
		else
		{
			CBasePlayer::GiveAmmo(255, "Pistol");
			CBasePlayer::GiveAmmo(255, "AR2");
			CBasePlayer::GiveAmmo(255, "SMG1");
			CBasePlayer::GiveAmmo(255, "Buckshot");

			if (pPlayerClassInfo.szPrimaryWeapon[0])
			{
				GiveNamedItem(pPlayerClassInfo.szPrimaryWeapon);
			}

			if (pPlayerClassInfo.szSecondaryWeapon[0])
			{
				GiveNamedItem(pPlayerClassInfo.szSecondaryWeapon);
			}

			if (pPlayerClassInfo.szMeleeWeapon[0])
			{
				GiveNamedItem(pPlayerClassInfo.szMeleeWeapon);
			}

			if (pPlayerClassInfo.iGrenades == -1 || pPlayerClassInfo.iGrenades > 0)
			{
				GiveNamedItem("weapon_frag");

				if (pPlayerClassInfo.iGrenades > 1)
				{
					CBasePlayer::GiveAmmo((pPlayerClassInfo.iGrenades - 1), "grenade");
				}
			}

			if (pPlayerClassInfo.iCombineBalls == -1 || pPlayerClassInfo.iCombineBalls > 0)
			{
				if (pPlayerClassInfo.iGrenades > 1)
				{
					CBasePlayer::GiveAmmo(pPlayerClassInfo.iCombineBalls, "AR2AltFire");
				}
				else if (pPlayerClassInfo.iCombineBalls == -1)
				{
					CBasePlayer::GiveAmmo(1, "AR2AltFire");
				}
			}

			if (pPlayerClassInfo.iManhacks == -1 || pPlayerClassInfo.iManhacks > 0)
			{
				SetBodygroup(1, 1);
				GiveNamedItem("weapon_manhack");

				if (pPlayerClassInfo.iManhacks > 1)
				{
					CBasePlayer::GiveAmmo(pPlayerClassInfo.iManhacks, "Manhacks");
				}
			}

			// switch to our primary instead of the last weapon we were given.
			if (pPlayerClassInfo.szPrimaryWeapon[0])
			{
				Weapon_Switch(Weapon_OwnsThisType(pPlayerClassInfo.szPrimaryWeapon));
			}

			SetPreventWeaponPickup(true);
		}

		if (pPlayerClassInfo.flNormSpeed > 0)
		{
			m_flNormalSpeed = pPlayerClassInfo.flNormSpeed;
		}

		if (pPlayerClassInfo.flSprintSpeed > 0)
		{
			m_flSprintSpeed = pPlayerClassInfo.flSprintSpeed;
		}

		if (pPlayerClassInfo.iSentenceVoice > VOICE_TYPE_NONE)
		{
			if (pPlayerClassInfo.iSentenceVoice == VOICE_TYPE_METROPOLICE)
			{
				m_Sentences.Init(this, "NPC_Metropolice.PlayerSentenceParameters");
			}
			else if (pPlayerClassInfo.iSentenceVoice == VOICE_TYPE_SOLDIER)
			{
				m_Sentences.Init(this, "NPC_Combine.PlayerSentenceParameters");
			}
		}
	}
}

const char* CHL2MP_Player::SentenceForConcept(int iConcept, int iVoiceMode)
{
	int concept = iConcept;

	if (concept == MP_SENTENCE_LOST_GROUP)
	{
		int choice = random->RandomInt(0, 1);

		if (choice == 1)
		{
			concept = MP_SENTENCE_LOST_LONG;
		}
		else
		{
			concept = MP_SENTENCE_LOST_SHORT;
		}
	}
	else if (concept == MP_SENTENCE_ALERT_GROUP)
	{
		if (iVoiceMode == VOICE_TYPE_METROPOLICE)
		{
			concept = MP_SENTENCE_GO_ALERT;
		}
		else if (iVoiceMode == VOICE_TYPE_SOLDIER)
		{
			int choice = random->RandomInt(0, 1);

			if (choice == 1)
			{
				concept = MP_SENTENCE_GO_ALERT;
			}
			else
			{
				concept = MP_SENTENCE_COMBINE_ALERT;
			}
		}
	}

	switch (concept)
	{
		case MP_SENTENCE_COMBINE_ANNOUNCE:
			return "ANNOUNCE";
		case MP_SENTENCE_COMBINE_ASSAULT:
			return "ASSAULT";
		case MP_SENTENCE_FLANK:
			return "FLANK";
		case MP_SENTENCE_GO_ALERT:
			return "GO_ALERT";
		case MP_SENTENCE_LOST_LONG:
			return "LOST_LONG";
		case MP_SENTENCE_LOST_SHORT:
			return "LOST_SHORT";
		case MP_SENTENCE_REFIND_ENEMY:
			return "REFIND_ENEMY";
		case MP_SENTENCE_DANGER:
			return "DANGER";
		case MP_SENTENCE_COMBINE_ALERT:
			return "ALERT";
		case MP_SENTENCE_IDLE:
			if (iVoiceMode == VOICE_TYPE_METROPOLICE)
				return "IDLE_CR";
			else if (iVoiceMode == VOICE_TYPE_SOLDIER)
				return "IDLE";
		case MP_SENTENCE_QUEST:
			if (iVoiceMode == VOICE_TYPE_METROPOLICE)
				return "IDLE_QUEST_CR";
			else if (iVoiceMode == VOICE_TYPE_SOLDIER)
				return "QUEST";
		case MP_SENTENCE_ANSWER:
			if (iVoiceMode == VOICE_TYPE_METROPOLICE)
				return "IDLE_ANSWER_CR";
			else if (iVoiceMode == VOICE_TYPE_SOLDIER)
				return "ANSWER";
		case MP_SENTENCE_CLEAR:
			if (iVoiceMode == VOICE_TYPE_METROPOLICE)
				return "IDLE_CLEAR_CR";
			else if (iVoiceMode == VOICE_TYPE_SOLDIER)
				return "CLEAR";
		case MP_SENTENCE_CHECK:
			if (iVoiceMode == VOICE_TYPE_METROPOLICE)
				return "IDLE_CHECK_CR";
			else if (iVoiceMode == VOICE_TYPE_SOLDIER)
				return "CHECK";
		default:
			break;
	}

	return "";
}

void CHL2MP_Player::SpeakSentence(const char* pSentence, SentencePriority_t nSoundPriority, SentenceCriteria_t nCriteria)
{
	if (GetPlayerClass() > CLS_INVALID)
	{
		const CAnticitizen_FilePlayerClassInfo_t& pPlayerClassInfo = GetPlayerClassInfo();

		if (pPlayerClassInfo.iSentenceVoice > VOICE_TYPE_NONE)
		{
			const char* szPrefix = "";

			if (pPlayerClassInfo.iSentenceVoice == VOICE_TYPE_METROPOLICE)
			{
				szPrefix = "METROPOLICE_";
			}
			else if (pPlayerClassInfo.iSentenceVoice == VOICE_TYPE_SOLDIER)
			{
				szPrefix = "COMBINE_";
			}

			char szStepSound[128];

			Q_snprintf(szStepSound, sizeof(szStepSound), "%s%s", szPrefix, pSentence);

			m_Sentences.Speak(szStepSound);
		}
	}
}

void CC_Debug_Sentences(const CCommand& args)
{
	CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_GetCommandClient());

	if (pPlayer)
	{
		pPlayer->SpeakSentence(args[1]);
	}
}
static ConCommand player_sentence("player_sentence", CC_Debug_Sentences, "Say something\n");

bool CHL2MP_Player::HandleCommand_JoinTeam( int team )
{
	if ( !GetGlobalTeam( team ) || team == 0 )
	{
		Warning( "HandleCommand_JoinTeam( %d ) - invalid team index.\n", team );
		return false;
	}

	if (team == GetTeamNumber())
	{
		return false;
	}

	if ( team == TEAM_SPECTATOR )
	{
		// Prevent this is the cvar is set
		if ( !mp_allowspectators.GetInt() && !IsHLTV() )
		{
			ClientPrint( this, HUD_PRINTCENTER, "#Cannot_Be_Spectator" );
			return false;
		}

		ChangeTeam( TEAM_SPECTATOR );

		return true;
	}

#ifndef DEBUG
	if (GetPlayerClass() == CLS_FREEMAN)
	{
		Warning("Cannot join another team as Freeman.\n");
		return true;
	}
#endif // !DEBUG

	// Switch their actual team...
	ChangeTeam( team );

	return true;
}

bool CHL2MP_Player::HandleCommand_JoinClass(int iclass)
{
	if (!g_Anticitizen_PR)
		return false;

	if (iclass < 0 || iclass >= g_Anticitizen_PR->GetNumPlayerClasses())
	{
		Warning("HandleCommand_JoinClass( %d ) - invalid class index.\n", iclass);
		return false;
	}

	if (iclass == GetPlayerClass())
	{
		return false;
	}

#ifndef DEBUG
	if (iclass == CLS_FREEMAN)
	{
		Warning("Freeman classes are managed by the game.\n");
		return false;
	}
#endif // !DEBUG

#ifdef DEBUG
	if (iclass == CLS_FREEMAN)
	{
		ChangeTeam(TEAM_FREEMAN);
	}
	else if (GetTeamNumber() != TEAM_COMBINE)
#else
	if (GetTeamNumber() != TEAM_COMBINE)
#endif // !DEBUG
	{
		ChangeTeam(TEAM_COMBINE);
	}

	// Switch their actual team...
	RemoveAllItems(true);
	SetPlayerClass(iclass);
	m_bChosenClass = true;
	Spawn();

	return true;
}

bool CHL2MP_Player::ClientCommand( const CCommand &args )
{
	if ( FStrEq( args[0], "spectate" ) )
	{
		if ( ShouldRunRateLimitedCommand( args ) )
		{
			// instantly join spectators
			HandleCommand_JoinTeam( TEAM_SPECTATOR );	
		}
		return true;
	}
	else if (FStrEq(args[0], "joinclass"))
	{
		if (args.ArgC() < 2)
		{
			Warning("Player sent bad joinclass syntax\n");
		}

#ifndef DEBUG
		if (GetPlayerClass() == CLS_FREEMAN)
		{
			Warning("Cannot join another class as Freeman.\n");
			return true;
		}
#endif // !DEBUG

		if (ShouldRunRateLimitedCommand(args))
		{
			int iClass = atoi(args[1]);
			HandleCommand_JoinClass(iClass);
		}
		return true;
	}

	return BaseClass::ClientCommand( args );
}

void CHL2MP_Player::CheatImpulseCommands( int iImpulse )
{
	switch ( iImpulse )
	{
		case 101:
			{
				if( sv_cheats->GetBool() )
				{
					GiveAllItems();
				}
			}
			break;

		default:
			BaseClass::CheatImpulseCommands( iImpulse );
	}
}

bool CHL2MP_Player::ShouldRunRateLimitedCommand( const CCommand &args )
{
	int i = m_RateLimitLastCommandTimes.Find( args[0] );
	if ( i == m_RateLimitLastCommandTimes.InvalidIndex() )
	{
		m_RateLimitLastCommandTimes.Insert( args[0], gpGlobals->curtime );
		return true;
	}
	else if ( (gpGlobals->curtime - m_RateLimitLastCommandTimes[i]) < HL2MP_COMMAND_MAX_RATE )
	{
		// Too fast.
		return false;
	}
	else
	{
		m_RateLimitLastCommandTimes[i] = gpGlobals->curtime;
		return true;
	}
}

void CHL2MP_Player::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

bool CHL2MP_Player::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

// -------------------------------------------------------------------------------- //
// Ragdoll entities.
// -------------------------------------------------------------------------------- //

class CHL2MPRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CHL2MPRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );
};

LINK_ENTITY_TO_CLASS( hl2mp_ragdoll, CHL2MPRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CHL2MPRagdoll, DT_HL2MPRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) )
END_SEND_TABLE()


void CHL2MP_Player::CreateRagdollEntity( void )
{
	if ( m_hRagdoll )
	{
		UTIL_RemoveImmediate( m_hRagdoll );
		m_hRagdoll = NULL;
	}

	// If we already have a ragdoll, don't make another one.
	CHL2MPRagdoll *pRagdoll = dynamic_cast< CHL2MPRagdoll* >( m_hRagdoll.Get() );
	
	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CHL2MPRagdoll* >( CreateEntityByName( "hl2mp_ragdoll" ) );
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = m_vecTotalBulletForce;
		pRagdoll->SetAbsOrigin( GetAbsOrigin() );
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int CHL2MP_Player::FlashlightIsOn( void )
{
	return IsEffectActive( EF_DIMLIGHT );
}

extern ConVar flashlight;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOn( void )
{
	if( flashlight.GetInt() > 0 && IsAlive() )
	{
		AddEffects( EF_DIMLIGHT );
		EmitSound( "HL2Player.FlashlightOn" );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHL2MP_Player::FlashlightTurnOff( void )
{
	RemoveEffects( EF_DIMLIGHT );
	
	if( IsAlive() )
	{
		EmitSound( "HL2Player.FlashlightOff" );
	}
}

void CHL2MP_Player::Weapon_Drop( CBaseCombatWeapon *pWeapon, const Vector *pvecTarget, const Vector *pVelocity )
{
	//Drop a grenade if it's primed.
	if ( GetActiveWeapon() )
	{
		CBaseCombatWeapon *pGrenade = Weapon_OwnsThisType("weapon_frag");

		if ( GetActiveWeapon() == pGrenade )
		{
			if ( ( m_nButtons & IN_ATTACK ) || (m_nButtons & IN_ATTACK2) )
			{
				DropPrimedFragGrenade( this, pGrenade );
				return;
			}
		}
	}

	BaseClass::Weapon_Drop( pWeapon, pvecTarget, pVelocity );
}

int CHL2MP_Player::GetMaxAmmo( int iAmmoIndex ) const
{
	if ( iAmmoIndex == -1 )
		return 0;

	if ( GetAmmoDef()->MaxCarry( iAmmoIndex ) == INFINITE_AMMO )
		return 999;

	return GetAmmoDef()->MaxCarry( iAmmoIndex );
}

void CHL2MP_Player::DetonateTripmines( void )
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname( pEntity, "npc_satchel" )) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() == this )
		{
			g_EventQueue.AddEvent( pSatchel, "Explode", 0.20, this, this );
		}
	}

	// Play sound for pressing the detonator
	EmitSound( "Weapon_SLAM.SatchelDetonate" );
}

void CHL2MP_Player::Event_Killed( const CTakeDamageInfo &info )
{
	//update damage info with our accumulated physics force
	CTakeDamageInfo subinfo = info;
	subinfo.SetDamageForce( m_vecTotalBulletForce );

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.
	// Ms - Spectators don't have corpes
	if (GetTeamNumber() != TEAM_SPECTATOR)
		CreateRagdollEntity();

	DetonateTripmines();

	BaseClass::Event_Killed( subinfo );

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		if ( m_hRagdoll )
		{
			m_hRagdoll->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL );
		}
	}

	CBaseEntity *pAttacker = info.GetAttacker();

	// Check for the attacker being in team Unassigned to account
	// for non-player attackers, which are in this team by default.
	// In TDM, should only happen with deaths to non-player causes.
	if (pAttacker && !pAttacker->InSameTeam(this) && pAttacker->GetTeamNumber() != TEAM_UNASSIGNED)
	{
		pAttacker->GetTeam()->AddScore(1);
	}
	else
	{
		GetTeam()->AddScore(-1);
	}

	FlashlightTurnOff();

	m_lifeState = LIFE_DEAD;

	RemoveEffects( EF_NODRAW );	// still draw player body
	StopZooming();
}

int CHL2MP_Player::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	//return here if the player is in the respawn grace period vs. slams.
	if ( gpGlobals->curtime < m_flSlamProtectTime &&  (inputInfo.GetDamageType() == DMG_BLAST ) )
		return 0;

	m_vecTotalBulletForce += inputInfo.GetDamageForce();
	
	gamestats->Event_PlayerDamage( this, inputInfo );

	return BaseClass::OnTakeDamage( inputInfo );
}

int CHL2MP_Player::OnTakeDamage_Alive(const CTakeDamageInfo& info)
{
	if (GetPlayerClass() > CLS_INVALID)
	{
		const CAnticitizen_FilePlayerClassInfo_t& pPlayerClassInfo = GetPlayerClassInfo();

		if (pPlayerClassInfo.iSentenceVoice > VOICE_TYPE_NONE)
		{
			PainSound(info);

			if ((info.GetDamageType() & DMG_SLASH) && hl2_episodic.GetBool())
			{
				if (m_afPhysicsFlags & PFLAG_USING)
				{
					// Stop the player using a rotating button for a short time if hit by a creature's melee attack.
					// This is for the antlion burrow-corking training in EP1 (sjb).
					SuspendUse(0.5f);
				}
			}

			return BaseClass::BaseClass::OnTakeDamage_Alive(info);;
		}
	}

	// Call the base class implementation
	return BaseClass::OnTakeDamage_Alive(info);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHL2MP_Player::PainSound(const CTakeDamageInfo& info)
{
	// Ms - Spectators can't scream
	if (GetTeamNumber() == TEAM_SPECTATOR)
		return;

	if (GetPlayerClass() > CLS_INVALID)
	{
		const CAnticitizen_FilePlayerClassInfo_t& pPlayerClassInfo = GetPlayerClassInfo();

		if (pPlayerClassInfo.iSentenceVoice > VOICE_TYPE_NONE)
		{
			if (gpGlobals->curtime < m_flNextPainSoundTime)
				return;

			float flHeavy = 0.45f;
			float flLight = 0.75f;

			float healthRatio = (float)GetHealth() / (float)GetMaxHealth();
			if (healthRatio > 0.0f)
			{
				const char* pSentenceName = "";

				if (pPlayerClassInfo.iSentenceVoice == VOICE_TYPE_METROPOLICE)
				{
					pSentenceName = "METROPOLICE_PAIN";
					if (healthRatio < flHeavy)
					{
						pSentenceName = "METROPOLICE_PAIN_HEAVY";
					}
					else if (healthRatio > flLight)
					{
						pSentenceName = "METROPOLICE_PAIN_LIGHT";
					}
				}
				else if (pPlayerClassInfo.iSentenceVoice == VOICE_TYPE_SOLDIER)
				{
					pSentenceName = "COMBINE_PAIN";
					if (healthRatio < flHeavy)
					{
						pSentenceName = "COMBINE_COVER";
					}
					else if (healthRatio > flLight)
					{
						pSentenceName = "COMBINE_TAUNT";
					}
				}

				// This causes it to speak it no matter what; doesn't bother with setting sounds.
				// Use m_Sentences.Speak because we have custom logic here.
				m_Sentences.Speak(pSentenceName, SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
				m_flNextPainSoundTime = gpGlobals->curtime + 1;
			}
		}
	}
}

int CHL2MP_Player::GetVoiceMode(void)
{
	const CAnticitizen_FilePlayerClassInfo_t& pPlayerClassInfo = GetPlayerClassInfo();
	return pPlayerClassInfo.iSentenceVoice;
}

void CHL2MP_Player::DeathSound( const CTakeDamageInfo &info )
{
	// Ms - Spectators can't scream
	if (GetTeamNumber() == TEAM_SPECTATOR)
		return;
	
	if ( m_hRagdoll && m_hRagdoll->GetBaseAnimating()->IsDissolving() )
		 return;

	if (GetPlayerClass() > CLS_INVALID)
	{
		const CAnticitizen_FilePlayerClassInfo_t& pPlayerClassInfo = GetPlayerClassInfo();

		if (pPlayerClassInfo.iSentenceVoice > VOICE_TYPE_NONE)
		{
			SpeakSentence("DIE");
		}
		else
		{
			// freeman death sounds.
			BaseClass::DeathSound(info);
		}
	}
	else
	{
		char szStepSound[128];

		Q_snprintf(szStepSound, sizeof(szStepSound), "%s.Die", GetPlayerModelSoundPrefix());

		const char* pModelName = STRING(GetModelName());

		CSoundParameters params;
		if (GetParametersForSound(szStepSound, params, pModelName) == false)
			return;

		Vector vecOrigin = GetAbsOrigin();

		CRecipientFilter filter;
		filter.AddRecipientsByPAS(vecOrigin);

		EmitSound_t ep;
		ep.m_nChannel = params.channel;
		ep.m_pSoundName = params.soundname;
		ep.m_flVolume = params.volume;
		ep.m_SoundLevel = params.soundlevel;
		ep.m_nFlags = 0;
		ep.m_nPitch = params.pitch;
		ep.m_pOrigin = &vecOrigin;

		EmitSound(filter, entindex(), ep);
	}
}

CBaseEntity* CHL2MP_Player::EntSelectSpawnPoint( void )
{
	CBaseEntity *pSpot = NULL;
	CBaseEntity *pLastSpawnPoint = g_pLastSpawn;
	edict_t		*player = edict();
	const char *pSpawnpointName = "info_player_deathmatch";

	if (GetTeamNumber() == TEAM_COMBINE)
	{
		pSpawnpointName = "info_player_combine";
		pLastSpawnPoint = g_pLastCombineSpawn;
	}
	else if (GetTeamNumber() == TEAM_FREEMAN)
	{
		pSpawnpointName = "info_player_rebel";
		pLastSpawnPoint = g_pLastRebelSpawn;
	}

	if (gEntList.FindEntityByClassname(NULL, pSpawnpointName) == NULL)
	{
		pSpawnpointName = "info_player_deathmatch";
		pLastSpawnPoint = g_pLastSpawn;
	}

	pSpot = pLastSpawnPoint;
	// Randomize the start spot
	for ( int i = random->RandomInt(1,5); i > 0; i-- )
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	if ( !pSpot )  // skip over the null point
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );

	CBaseEntity *pFirstSpot = pSpot;

	do 
	{
		if ( pSpot )
		{
			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, pSpawnpointName );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	if ( pSpot )
	{
		CBaseEntity *ent = NULL;
		for ( CEntitySphereQuery sphere( pSpot->GetAbsOrigin(), hl2mp_spawn_frag_fallback_radius.GetFloat() ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
		{
			// if ent is a client, kill em (unless they are ourselves)
			if ( ent->IsPlayer() && !(ent->edict() == player) )
				ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
		}
		goto ReturnSpot;
	}

	if ( !pSpot  )
	{
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_player_start" );

		if ( pSpot )
			goto ReturnSpot;
	}

ReturnSpot:

	if (GetTeamNumber() == TEAM_COMBINE)
	{
		g_pLastCombineSpawn = pSpot;
	}
	else if (GetTeamNumber() == TEAM_FREEMAN)
	{
		g_pLastRebelSpawn = pSpot;
	}

	g_pLastSpawn = pSpot;

	m_flSlamProtectTime = gpGlobals->curtime + 0.5;

	return pSpot;
} 


CON_COMMAND( timeleft, "prints the time remaining in the match" )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_GetCommandClient() );

	int iTimeRemaining = (int)HL2MPRules()->GetMapRemainingTime();
    
	if ( iTimeRemaining == 0 )
	{
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "This game has no timelimit." );
		}
		else
		{
			Msg( "* No Time Limit *\n" );
		}
	}
	else
	{
		int iMinutes, iSeconds;
		iMinutes = iTimeRemaining / 60;
		iSeconds = iTimeRemaining % 60;

		char minutes[8];
		char seconds[8];

		Q_snprintf( minutes, sizeof(minutes), "%d", iMinutes );
		Q_snprintf( seconds, sizeof(seconds), "%2.2d", iSeconds );

		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, "Time left in map: %s1:%s2", minutes, seconds );
		}
		else
		{
			Msg( "Time Remaining:  %s:%s\n", minutes, seconds );
		}
	}	
}


void CHL2MP_Player::Reset()
{	
	ResetDeathCount();
	ResetFragCount();
}

bool CHL2MP_Player::IsReady()
{
	return m_bReady;
}

void CHL2MP_Player::SetReady( bool bReady )
{
	m_bReady = bReady;
}

void CHL2MP_Player::CheckChatText( char *p, int bufsize )
{
	BaseClass::CheckChatText(p, bufsize);

	//Look for escape sequences and replace

	char *buf = new char[bufsize];
	int pos = 0;

	// Parse say text for escape sequences
	for ( char *pSrc = p; pSrc != NULL && *pSrc != 0 && pos < bufsize-1; pSrc++ )
	{
		// copy each char across
		buf[pos] = *pSrc;
		pos++;
	}

	buf[pos] = '\0';

	// copy buf back into p
	Q_strncpy( p, buf, bufsize );

	delete[] buf;	

	const char *pReadyCheck = p;

	HL2MPRules()->CheckChatForReadySignal( this, pReadyCheck );
}

void CHL2MP_Player::State_Transition( HL2MPPlayerState newState )
{
	State_Leave();
	State_Enter( newState );
}


void CHL2MP_Player::State_Enter( HL2MPPlayerState newState )
{
	m_iPlayerState = newState;
	m_pCurStateInfo = State_LookupInfo( newState );

	// Initialize the new state.
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnEnterState )
		(this->*m_pCurStateInfo->pfnEnterState)();
}


void CHL2MP_Player::State_Leave()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnLeaveState )
	{
		(this->*m_pCurStateInfo->pfnLeaveState)();
	}
}


void CHL2MP_Player::State_PreThink()
{
	if ( m_pCurStateInfo && m_pCurStateInfo->pfnPreThink )
	{
		(this->*m_pCurStateInfo->pfnPreThink)();
	}
}


CHL2MPPlayerStateInfo *CHL2MP_Player::State_LookupInfo( HL2MPPlayerState state )
{
	// This table MUST match the 
	static CHL2MPPlayerStateInfo playerStateInfos[] =
	{
		{ STATE_ACTIVE,			"STATE_ACTIVE",			&CHL2MP_Player::State_Enter_ACTIVE, NULL, &CHL2MP_Player::State_PreThink_ACTIVE },
		{ STATE_OBSERVER_MODE,	"STATE_OBSERVER_MODE",	&CHL2MP_Player::State_Enter_OBSERVER_MODE,	NULL, &CHL2MP_Player::State_PreThink_OBSERVER_MODE }
	};

	for ( int i=0; i < ARRAYSIZE( playerStateInfos ); i++ )
	{
		if ( playerStateInfos[i].m_iPlayerState == state )
			return &playerStateInfos[i];
	}

	return NULL;
}

bool CHL2MP_Player::StartObserverMode(int mode)
{
	//we only want to go into observer mode if the player asked to, not on a death timeout
	if ( m_bEnterObserver == true )
	{
		VPhysicsDestroyObject();
		return BaseClass::StartObserverMode( mode );
	}
	return false;
}

void CHL2MP_Player::StopObserverMode()
{
	m_bEnterObserver = false;
	BaseClass::StopObserverMode();
}

void CHL2MP_Player::State_Enter_OBSERVER_MODE()
{
	int observerMode = m_iObserverLastMode;
	if ( IsNetClient() )
	{
		const char *pIdealMode = engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_spec_mode" );
		if ( pIdealMode )
		{
			observerMode = atoi( pIdealMode );
			if ( observerMode <= OBS_MODE_FIXED || observerMode > OBS_MODE_ROAMING )
			{
				observerMode = m_iObserverLastMode;
			}
		}
	}
	m_bEnterObserver = true;
	StartObserverMode( observerMode );
}

void CHL2MP_Player::State_PreThink_OBSERVER_MODE()
{
	// Make sure nobody has changed any of our state.
	//	Assert( GetMoveType() == MOVETYPE_FLY );
	Assert( m_takedamage == DAMAGE_NO );
	Assert( IsSolidFlagSet( FSOLID_NOT_SOLID ) );
	//	Assert( IsEffectActive( EF_NODRAW ) );

	// Must be dead.
	Assert( m_lifeState == LIFE_DEAD );
	Assert( pl.deadflag );
}


void CHL2MP_Player::State_Enter_ACTIVE()
{
	SetMoveType( MOVETYPE_WALK );
	
	// md 8/15/07 - They'll get set back to solid when they actually respawn. If we set them solid now and mp_forcerespawn
	// is false, then they'll be spectating but blocking live players from moving.
	// RemoveSolidFlags( FSOLID_NOT_SOLID );
	
	m_Local.m_iHideHUD = 0;
}


void CHL2MP_Player::State_PreThink_ACTIVE()
{
	//we don't really need to do anything here. 
	//This state_prethink structure came over from CS:S and was doing an assert check that fails the way hl2dm handles death
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHL2MP_Player::CanHearAndReadChatFrom( CBasePlayer *pPlayer )
{
	// can always hear the console unless we're ignoring all chat
	if ( !pPlayer )
		return false;

	return true;
}

//-----------------------------------------------------------------------------------------------------
// Return true if the given threat is aiming in our direction
bool CHL2MP_Player::IsThreatAimingTowardMe( CBaseEntity* threat, float cosTolerance ) const
{
	CHL2MP_Player* player = ToHL2MPPlayer( threat );
	Vector to = GetAbsOrigin() - threat->GetAbsOrigin();
	Vector forward;

	if ( player == NULL )
	{
		return false;
	}

	// is the player pointing at me?
	player->EyeVectors( &forward );

	if ( DotProduct( to, forward ) > cosTolerance )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------------------------------
// Return true if the given threat is aiming in our direction and firing its weapon
bool CHL2MP_Player::IsThreatFiringAtMe( CBaseEntity* threat ) const
{
	if ( IsThreatAimingTowardMe( threat ) )
	{
		CHL2MP_Player* player = ToHL2MPPlayer( threat );

		if ( player )
		{
			return player->IsFiringWeapon();
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: multiplayer does not do autoaiming.
//-----------------------------------------------------------------------------
Vector CHL2MP_Player::GetAutoaimVector(float flScale)
{
	//No Autoaim
	Vector	forward;
	AngleVectors(EyeAngles() + m_Local.m_vecPunchAngle, &forward);
	return	forward;
}

//-----------------------------------------------------------------------------
// Purpose: Do nothing multiplayer_animstate takes care of animation.
// Input  : playerAnim - 
//-----------------------------------------------------------------------------
void CHL2MP_Player::SetAnimation(PLAYER_ANIM playerAnim)
{
	return;
}

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //
class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS(CTEPlayerAnimEvent, CBaseTempEntity);
	DECLARE_SERVERCLASS();

	CTEPlayerAnimEvent(const char* name) : CBaseTempEntity(name)
	{
	}

	CNetworkHandle(CBasePlayer, m_hPlayer);
	CNetworkVar(int, m_iEvent);
	CNetworkVar(int, m_nData);
};

IMPLEMENT_SERVERCLASS_ST_NOBASE(CTEPlayerAnimEvent, DT_TEPlayerAnimEvent)
SendPropEHandle(SENDINFO(m_hPlayer)),
SendPropInt(SENDINFO(m_iEvent), Q_log2(PLAYERANIMEVENT_COUNT) + 1, SPROP_UNSIGNED),
SendPropInt(SENDINFO(m_nData), 32)
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent("PlayerAnimEvent");

void TE_PlayerAnimEvent(CBasePlayer* pPlayer, PlayerAnimEvent_t event, int nData)
{
	CPVSFilter filter((const Vector&)pPlayer->EyePosition());

	//Tony; use prediction rules.
	filter.UsePredictionRules();

	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.m_nData = nData;
	g_TEPlayerAnimEvent.Create(filter, 0);
}


void CHL2MP_Player::DoAnimationEvent(PlayerAnimEvent_t event, int nData)
{
	m_PlayerAnimState->DoAnimationEvent(event, nData);
	TE_PlayerAnimEvent(this, event, nData);	// Send to any clients who can see this guy.
}

//-----------------------------------------------------------------------------
// Purpose: Override setup bones so that is uses the render angles from
//			the HL2MP animation state to setup the hitboxes.
//-----------------------------------------------------------------------------
void CHL2MP_Player::SetupBones(matrix3x4_t* pBoneToWorld, int boneMask)
{
	VPROF_BUDGET("CHL2MP_Player::SetupBones", VPROF_BUDGETGROUP_SERVER_ANIM);

	// Get the studio header.
	Assert(GetModelPtr());
	CStudioHdr* pStudioHdr = GetModelPtr();
	if (!pStudioHdr)
		return;

	Vector pos[MAXSTUDIOBONES];
	Quaternion q[MAXSTUDIOBONES];

	// Adjust hit boxes based on IK driven offset.
	Vector adjOrigin = GetAbsOrigin() + Vector(0, 0, m_flEstIkOffset);

	// FIXME: pass this into Studio_BuildMatrices to skip transforms
	CBoneBitList boneComputed;
	if (m_pIk)
	{
		m_iIKCounter++;
		m_pIk->Init(pStudioHdr, GetAbsAngles(), adjOrigin, gpGlobals->curtime, m_iIKCounter, boneMask);
		GetSkeleton(pStudioHdr, pos, q, boneMask);

		m_pIk->UpdateTargets(pos, q, pBoneToWorld, boneComputed);
		CalculateIKLocks(gpGlobals->curtime);
		m_pIk->SolveDependencies(pos, q, pBoneToWorld, boneComputed);
	}
	else
	{
		GetSkeleton(pStudioHdr, pos, q, boneMask);
	}

	CBaseAnimating* pParent = dynamic_cast<CBaseAnimating*>(GetMoveParent());
	if (pParent)
	{
		// We're doing bone merging, so do special stuff here.
		CBoneCache* pParentCache = pParent->GetBoneCache();
		if (pParentCache)
		{
			BuildMatricesWithBoneMerge(
				pStudioHdr,
				m_PlayerAnimState->GetRenderAngles(),
				adjOrigin,
				pos,
				q,
				pBoneToWorld,
				pParent,
				pParentCache);

			return;
		}
	}

	Studio_BuildMatrices(
		pStudioHdr,
		m_PlayerAnimState->GetRenderAngles(),
		adjOrigin,
		pos,
		q,
		-1,
		GetModelScale(), // Scaling
		pBoneToWorld,
		boneMask);
}