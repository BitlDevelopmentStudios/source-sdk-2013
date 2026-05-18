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
#include "ammodef.h"
#include "NextBot.h"

#include "engine/IEngineSound.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"

#include "ilagcompensationmanager.h"

int g_iLastCitizenModel = 0;
int g_iLastCombineModel = 0;

CBaseEntity	 *g_pLastCombineSpawn = NULL;
CBaseEntity	 *g_pLastRebelSpawn = NULL;
extern CBaseEntity				*g_pLastSpawn;

ConVar hl2mp_spawn_frag_fallback_radius( "hl2mp_spawn_frag_fallback_radius", "48", FCVAR_NONE, "If no spawns are available, kill players with this radius to allow new players to spawn." );

#define HL2MP_COMMAND_MAX_RATE 0.3

void DropPrimedFragGrenade( CHL2MP_Player *pPlayer, CBaseCombatWeapon *pGrenade );

LINK_ENTITY_TO_CLASS( player, CHL2MP_Player );

LINK_ENTITY_TO_CLASS( info_player_combine, CPointEntity );
LINK_ENTITY_TO_CLASS( info_player_rebel, CPointEntity );

// specific to the local player
BEGIN_SEND_TABLE_NOBASE( CHL2MP_Player, DT_HL2MPLocalPlayerExclusive )
	// send a hi-res origin to the local player for use in prediction
	SendPropVectorXY(SENDINFO(m_vecOrigin),               -1, SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_OriginXY ),
	SendPropFloat   (SENDINFO_VECTORELEM(m_vecOrigin, 2), -1, SPROP_NOSCALE|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_OriginZ ),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),

END_SEND_TABLE()

// all players except the local player
BEGIN_SEND_TABLE_NOBASE( CHL2MP_Player, DT_HL2MPNonLocalPlayerExclusive )
	// send a lo-res origin to other players
	SendPropVectorXY(SENDINFO(m_vecOrigin),               -1, SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_OriginXY ),
	SendPropFloat   (SENDINFO_VECTORELEM(m_vecOrigin, 2), -1, SPROP_COORD_MP_LOWPRECISION|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_OriginZ ),

	SendPropFloat( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 8, SPROP_CHANGES_OFTEN, -90.0f, 90.0f ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, SPROP_CHANGES_OFTEN ),

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

	SendPropInt(SENDINFO(m_iPlayerClass), 4),

	SendPropFloat(SENDINFO(m_flNormalSpeed)),
	SendPropFloat(SENDINFO(m_flSprintSpeed)),

	SendPropFloat(SENDINFO(m_flStamina)),
	
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseFlex", "m_viewtarget" ),

//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),	
END_SEND_TABLE()

BEGIN_DATADESC( CHL2MP_Player )
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CHL2MP_Player, CHL2_Player, "Half-Life 2: Deathmatch Player" )
END_SCRIPTDESC();

#define HL2MPPLAYER_PHYSDAMAGE_SCALE 4.0f

#pragma warning( disable : 4355 )

CHL2MP_Player::CHL2MP_Player() : m_PlayerAnimState( this )
{
	m_angEyeAngles.Init();

	m_flNextModelChangeTime = 0.0f;
	m_flNextTeamChangeTime = 0.0f;

	m_iSpawnInterpCounter = 0;

    m_bEnterObserver = false;
	m_bReady = false;

	m_iPlayerClass = CLS_INVALID;

	BaseClass::ChangeTeam( 0 );
	
	//UseClientSideAnimation();
}

CHL2MP_Player::~CHL2MP_Player( void )
{

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
	GiveAllWeapons();
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
		LoadClass(GetPlayerClass());
	}
	
	if ( !IsObserver() )
	{
		pl.deadflag = false;
		RemoveSolidFlags( FSOLID_NOT_SOLID );

		RemoveEffects( EF_NODRAW );
	}

	SetNumAnimOverlays( 3 );
	ResetAnimation();

	m_nRenderFX = kRenderNormal;

	m_flStamina = 100.0f;
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

void CHL2MP_Player::ResetAnimation( void )
{
	if ( IsAlive() )
	{
		SetSequence ( -1 );
		SetActivity( ACT_INVALID );

		if (!GetAbsVelocity().x && !GetAbsVelocity().y)
			SetAnimation( PLAYER_IDLE );
		else if ((GetAbsVelocity().x || GetAbsVelocity().y) && ( GetFlags() & FL_ONGROUND ))
			SetAnimation( PLAYER_WALK );
		else if (GetWaterLevel() > 1)
			SetAnimation( PLAYER_WALK );
		else if ( ( GetFlags() & FL_ONGROUND ) != FL_ONGROUND)
			SetAnimation( PLAYER_JUMP );
		else
			SetAnimation( PLAYER_IDLE );
	}
}


bool CHL2MP_Player::Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex )
{
	bool bRet = BaseClass::Weapon_Switch( pWeapon, viewmodelindex );

	if ( bRet == true )
	{
		ResetAnimation();
	}

	return bRet;
}

void CHL2MP_Player::PreThink( void )
{
	QAngle vOldAngles = GetLocalAngles();
	QAngle vTempAngles = GetLocalAngles();

	vTempAngles = EyeAngles();

	if ( vTempAngles[PITCH] > 180.0f )
	{
		vTempAngles[PITCH] -= 360.0f;
	}

	SetLocalAngles( vTempAngles );

	BaseClass::PreThink();
	State_PreThink();

	//Reset bullet force accumulator, only lasts one frame
	m_vecTotalBulletForce = vec3_origin;
	SetLocalAngles( vOldAngles );
}

void CHL2MP_Player::PostThink( void )
{
	BaseClass::PostThink();
	
	if ( GetFlags() & FL_DUCKING )
	{
		SetCollisionBounds( VEC_CROUCH_TRACE_MIN, VEC_CROUCH_TRACE_MAX );
	}

	m_PlayerAnimState.Update();

	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
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

// Set the activity based on an event or current state
void CHL2MP_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	int animDesired;

	float speed;

	speed = GetAbsVelocity().Length2D();

	
	// bool bRunning = true;

	//Revisit!
/*	if ( ( m_nButtons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
	{
		if ( speed > 1.0f && speed < hl2_normspeed.GetFloat() - 20.0f )
		{
			bRunning = false;
		}
	}*/

	if ( GetFlags() & ( FL_FROZEN | FL_ATCONTROLS ) )
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	Activity idealActivity = ACT_HL2MP_RUN;

	// This could stand to be redone. Why is playerAnim abstracted from activity? (sjb)
	if ( playerAnim == PLAYER_JUMP )
	{
		idealActivity = ACT_HL2MP_JUMP;
	}
	else if ( playerAnim == PLAYER_DIE )
	{
		if ( m_lifeState == LIFE_ALIVE )
		{
			return;
		}
	}
	else if ( playerAnim == PLAYER_ATTACK1 )
	{
		if ( GetActivity( ) == ACT_HOVER	|| 
			 GetActivity( ) == ACT_SWIM		||
			 GetActivity( ) == ACT_HOP		||
			 GetActivity( ) == ACT_LEAP		||
			 GetActivity( ) == ACT_DIESIMPLE )
		{
			idealActivity = GetActivity( );
		}
		else
		{
			idealActivity = ACT_HL2MP_GESTURE_RANGE_ATTACK;
		}
	}
	else if ( playerAnim == PLAYER_RELOAD )
	{
		idealActivity = ACT_HL2MP_GESTURE_RELOAD;
	}
	else if ( playerAnim == PLAYER_IDLE || playerAnim == PLAYER_WALK )
	{
		if ( !( GetFlags() & FL_ONGROUND ) && GetActivity( ) == ACT_HL2MP_JUMP )	// Still jumping
		{
			idealActivity = GetActivity( );
		}
		/*
		else if ( GetWaterLevel() > 1 )
		{
			if ( speed == 0 )
				idealActivity = ACT_HOVER;
			else
				idealActivity = ACT_SWIM;
		}
		*/
		else
		{
			if ( GetFlags() & FL_DUCKING )
			{
				if ( speed > 0 )
				{
					idealActivity = ACT_HL2MP_WALK_CROUCH;
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE_CROUCH;
				}
			}
			else
			{
				if ( speed > 0 )
				{
					/*
					if ( bRunning == false )
					{
						idealActivity = ACT_WALK;
					}
					else
					*/
					{
						idealActivity = ACT_HL2MP_RUN;
					}
				}
				else
				{
					idealActivity = ACT_HL2MP_IDLE;
				}
			}
		}

		idealActivity = TranslateTeamActivity( idealActivity );
	}
	
	if ( idealActivity == ACT_HL2MP_GESTURE_RANGE_ATTACK )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );

		// FIXME: this seems a bit wacked
		//
		// misyl: it was and was causing a pred error every time.
		// the weapons already call SendWeaponAnim with the right activity.
		//Weapon_SetActivity( Weapon_TranslateActivity( ACT_RANGE_ATTACK1 ), 0 );

		return;
	}
	else if ( idealActivity == ACT_HL2MP_GESTURE_RELOAD )
	{
		RestartGesture( Weapon_TranslateActivity( idealActivity ) );
		return;
	}
	else
	{
		SetActivity( idealActivity );

		animDesired = SelectWeightedSequence( Weapon_TranslateActivity ( idealActivity ) );

		if (animDesired == -1)
		{
			animDesired = SelectWeightedSequence( idealActivity );

			if ( animDesired == -1 )
			{
				animDesired = 0;
			}
		}
	
		// Already using the desired animation?
		if ( GetSequence() == animDesired )
			return;

		m_flPlaybackRate = 1.0;
		ResetSequence( animDesired );
		SetCycle( 0 );
		return;
	}

	// Already using the desired animation?
	if ( GetSequence() == animDesired )
		return;

	//Msg( "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	ResetSequence( animDesired );
	SetCycle( 0 );
}


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
	if (GetPlayerClass() > CLS_INVALID)
	{
		const CAnticitizen_FilePlayerClassInfo_t& pPlayerClassInfo = GetPlayerClassInfo();

		if (pPlayerClassInfo.m_szPlayerModel[0])
		{
			PrecacheModel(pPlayerClassInfo.m_szPlayerModel);
			SetModel(pPlayerClassInfo.m_szPlayerModel);
			SetupPlayerSoundsByModel(pPlayerClassInfo.m_szPlayerModel);
		}

		if (pPlayerClassInfo.iHealth > 0)
		{
			SetHealth(pPlayerClassInfo.iHealth);
			SetMaxHealth(pPlayerClassInfo.iHealth);
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
			CBasePlayer::GiveAmmo(120, "Pistol");
			CBasePlayer::GiveAmmo(120, "AR2");
			CBasePlayer::GiveAmmo(120, "SMG1");
			CBasePlayer::GiveAmmo(120, "Buckshot");

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
				m_Sentences.Init(this, "NPC_Metropolice.SentenceParameters");
			}
			else if (pPlayerClassInfo.iSentenceVoice == VOICE_TYPE_SOLDIER)
			{
				m_Sentences.Init(this, "NPC_Combine.SentenceParameters");
			}
		}
	}
}

void CHL2MP_Player::SpeakSentence(const char* pSentence)
{
	m_Sentences.Speak(pSentence);
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

	SetNumAnimOverlays( 0 );

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
				m_Sentences.Speak(pSentenceName, SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
				m_flNextPainSoundTime = gpGlobals->curtime + 1;
			}
		}
	}
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
			if (pPlayerClassInfo.iSentenceVoice == VOICE_TYPE_METROPOLICE)
			{
				m_Sentences.Speak("METROPOLICE_DIE");
			}
			else if (pPlayerClassInfo.iSentenceVoice == VOICE_TYPE_SOLDIER)
			{
				m_Sentences.Speak("COMBINE_DIE");
			}
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
