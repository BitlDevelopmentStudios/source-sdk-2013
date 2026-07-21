//========= Copyright Valve Corporation, All rights reserved. ============//
// tf_bot_sniper_attack.h
// Attack a threat as a Sniper
// Michael Booth, February 2009

#include "cbase.h"
#include "hl2mp_player.h"
#include "hl2mp_gamerules.h"
#include "bot/hl2mp_bot.h"
#include "bot/sniper/anticitizen_bot_sniper_attack.h"
#include "bot/behavior/hl2mp_bot_melee_attack.h"
#include "bot/behavior/hl2mp_bot_retreat_to_cover.h"
#include "weapon_sniperrifle.h"

#include "nav_mesh.h"

extern ConVar hl2mp_bot_path_lookahead_range;

ConVar anticitizen_bot_sniper_flee_range( "tf_bot_sniper_flee_range", "400", FCVAR_CHEAT, "If threat is closer than this, retreat" );
ConVar anticitizen_bot_sniper_melee_range( "tf_bot_sniper_melee_range", "200", FCVAR_CHEAT, "If threat is closer than this, attack with melee weapon" );
ConVar anticitizen_bot_sniper_linger_time( "tf_bot_sniper_linger_time", "5", FCVAR_CHEAT, "How long Sniper will wait around after losing his target before giving up" );


//---------------------------------------------------------------------------------------------
bool CHL2MPBotSniperAttack::IsPossible( CHL2MPBot *me )
{
	return (me->GetPlayerClass() == CLS_COMBINE_ASSASSIN) && me->GetVisionInterface()->GetPrimaryKnownThreat() && me->GetVisionInterface()->GetPrimaryKnownThreat()->IsVisibleRecently();
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperAttack::OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction )
{
	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperAttack::Update( CHL2MPBot *me, float interval )
{
	// switch to our sniper rifle
	CBaseCombatWeapon *myGun = me->Weapon_OwnsThisType("weapon_sniperrifle");
	if ( myGun)
	{
		me->Weapon_Switch( myGun );
	}

	if (me->GetAmmoCount(myGun->GetPrimaryAmmoType()) < SNIPER_CHARGE_DRAIN)
	{
		return SuspendFor(new CHL2MPBotRetreatToCover, "Our weapon is empty");
	}

	// shoot at bad guys
	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();

	if ( threat && !threat->GetEntity()->IsAlive() )
	{
		// he's dead
		threat = NULL;
	}

	if ( threat == NULL || !threat->IsVisibleInFOVNow() )
	{
		if ( m_lingerTimer.IsElapsed() )
		{
			if ( me->IsCurrentWeaponZoomed())
			{
				return Continue();
			}

			return Done( "No threat for awhile" );
		}

		return Continue();
	}

	//me->EquipBestWeaponForThreat( threat );

	if ( me->IsDistanceBetweenLessThan( threat->GetLastKnownPosition(), anticitizen_bot_sniper_flee_range.GetFloat() ) )
	{
		return SuspendFor( new CHL2MPBotRetreatToCover, "Retreating from nearby enemy" );
	}

	if ( me->GetTimeSinceLastInjury() < 1.0f )
	{
		return SuspendFor( new CHL2MPBotRetreatToCover, "Retreating due to injury" );
	}

	// we have a target
	m_lingerTimer.Start( RandomFloat( 0.75f, 1.25f ) * anticitizen_bot_sniper_linger_time.GetFloat() );

	if (!me->IsCurrentWeaponZoomed())
	{
		me->PressAltFireButton();
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
void CHL2MPBotSniperAttack::OnEnd( CHL2MPBot *me, Action< CHL2MPBot > *nextAction )
{
	if ( me->IsCurrentWeaponZoomed())
	{
		// we're leaving to do something else - unzoom
		me->PressAltFireButton();
	}
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperAttack::OnSuspend( CHL2MPBot *me, Action< CHL2MPBot > *interruptingAction )
{
	if ( me->IsCurrentWeaponZoomed())
	{
		// we're leaving to do something else - unzoom
		me->PressAltFireButton();
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperAttack::OnResume( CHL2MPBot *me, Action< CHL2MPBot > *interruptingAction )
{
	return Continue();
}


//---------------------------------------------------------------------------------------------
// given a subject, return the world space position we should aim at
Vector CHL2MPBotSniperAttack::SelectTargetPoint( const INextBot *me, const CBaseCombatCharacter *subject ) const
{
	VPROF_BUDGET( "CHL2MPBotSniperAttack::SelectTargetPoint", "NextBot" );

	Vector visibleSpot;

	trace_t result;
	NextBotTraceFilterIgnoreActors filter( subject, COLLISION_GROUP_NONE );

	// head, then chest, then feet for the Sniper

	// headshot seems to be a bit higher that EyePosition()
	Vector subjectHeadPos( subject->EyePosition() );
	subjectHeadPos.z += 1.0f;

	UTIL_TraceLine( me->GetBodyInterface()->GetEyePosition(), subjectHeadPos, MASK_BLOCKLOS_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE, &filter, &result );
	if ( result.DidHit() )
	{
		UTIL_TraceLine( me->GetBodyInterface()->GetEyePosition(), subject->WorldSpaceCenter(), MASK_BLOCKLOS_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE, &filter, &result );

		if ( result.DidHit() )
		{
			UTIL_TraceLine( me->GetBodyInterface()->GetEyePosition(), subject->GetAbsOrigin(), MASK_BLOCKLOS_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE, &filter, &result );
		}
	}

	// even if they aren't visible, we have no way to communicate that out, so pick a reasonable spot
	return result.endpos;
}


//---------------------------------------------------------------------------------------------
bool CHL2MPBotSniperAttack::IsImmediateThreat( const CBaseCombatCharacter *subject, const CKnownEntity *threat ) const
{
	if ( subject->InSameTeam( threat->GetEntity() ) )
		return false;

	if ( !threat->GetEntity()->IsAlive() )
		return false;

	const float hiddenAwhile = 3.0f;
	if ( !threat->WasEverVisible() || threat->GetTimeSinceLastSeen() > hiddenAwhile )
		return false;

	CHL2MP_Player *player = ToHL2MPPlayer( threat->GetEntity() );

	Vector to = subject->GetAbsOrigin() - threat->GetLastKnownPosition();
	float threatRange = to.NormalizeInPlace();

	if (player->GetPlayerClass() == CLS_FREEMAN)
	{
		// always try to kill these guys first
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------
// return the more dangerous of the two threats to 'subject', or NULL if we have no opinion
const CKnownEntity *CHL2MPBotSniperAttack::SelectMoreDangerousThreat( const INextBot *me, 
																   const CBaseCombatCharacter *subject,
																   const CKnownEntity *threat1, 
																   const CKnownEntity *threat2 ) const
{
	if ( threat1 && threat2 )
	{
		bool isImmediateThreat1 = IsImmediateThreat( subject, threat1 );
		bool isImmediateThreat2 = IsImmediateThreat( subject, threat2 );

		if ( isImmediateThreat1 && !isImmediateThreat2 )
		{
			return threat1;
		}
		else if ( !isImmediateThreat1 && isImmediateThreat2 )
		{
			return threat2;
		}
	}

	// both or neither are immediate threats - no preference
	return NULL;
}
