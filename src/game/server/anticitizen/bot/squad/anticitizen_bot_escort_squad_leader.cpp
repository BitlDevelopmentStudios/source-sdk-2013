//========= Copyright Valve Corporation, All rights reserved. ============//
// anticitizen_bot_escort_squad_leader.cpp
// Escort the squad leader to their destination
// Michael Booth, Octoboer 2011

#include "cbase.h"

#include "bot/hl2mp_bot.h"
#include "anticitizen/bot/squad/anticitizen_bot_escort_squad_leader.h"

ConVar anticitizen_bot_squad_escort_range( "anticitizen_bot_squad_escort_range", "500", FCVAR_CHEAT );
ConVar anticitizen_bot_formation_debug( "anticitizen_bot_formation_debug", "0", FCVAR_CHEAT );

class CTraceFilterIgnoreTeammates : public CTraceFilterSimple
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS(CTraceFilterIgnoreTeammates, CTraceFilterSimple);

	CTraceFilterIgnoreTeammates(const IHandleEntity* passentity, int collisionGroup, int iIgnoreTeam)
		: CTraceFilterSimple(passentity, collisionGroup), m_iIgnoreTeam(iIgnoreTeam)
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity* pServerEntity, int contentsMask)
	{
		CBaseEntity* pEntity = EntityFromEntityHandle(pServerEntity);

		if ((pEntity->IsPlayer() || pEntity->IsCombatItem()) && (pEntity->GetTeamNumber() == m_iIgnoreTeam || m_iIgnoreTeam == TEAM_ANY))
		{
			return false;
		}

		return BaseClass::ShouldHitEntity(pServerEntity, contentsMask);
	}

	int m_iIgnoreTeam;
};

//---------------------------------------------------------------------------------------------
CAnticitizenBotEscortSquadLeader::CAnticitizenBotEscortSquadLeader( Action< CHL2MPBot > *actionToDoAfterSquadDisbands ) // : m_path( ChasePath::LEAD_SUBJECT )
{
	m_actionToDoAfterSquadDisbands = actionToDoAfterSquadDisbands;
	m_formationPath.SetGoalTolerance( 0.0f );
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CAnticitizenBotEscortSquadLeader::OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction )
{
	m_formationForward = vec3_origin;

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot > CAnticitizenBotEscortSquadLeader::Update( CHL2MPBot *me, float interval )
{
	if ( interval <= 0.0f )
	{
		return Continue();
	}

	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	if ( threat && threat->IsVisibleRecently() )
	{
		// prepare to fight
		me->EquipBestWeaponForThreat( threat );
	}

	CHL2MPBotSquad *squad = me->GetSquad();
	if ( !squad )
	{
		if ( m_actionToDoAfterSquadDisbands )
		{
			return ChangeTo( m_actionToDoAfterSquadDisbands, "Not in a Squad" );
		}

		return Done( "Not in a Squad" );
	}

	// we need to update every tick to smoothly move in formation
	me->FlagForUpdate();

	CHL2MPBot *leader = squad->GetLeader();
	if ( !leader || !leader->IsAlive() )
	{
		me->LeaveSquad();

		if ( m_actionToDoAfterSquadDisbands )
		{
			return ChangeTo( m_actionToDoAfterSquadDisbands, "Squad leader is dead" );
		}

		return Done( "Squad leader is dead" );
	}

	CUtlVector< CHL2MPBot * > rawMemberVector;
	squad->CollectMembers( &rawMemberVector );

	const PathFollower *leaderPath = leader->GetCurrentPath();
	if ( !leaderPath || !leaderPath->GetCurrentGoal() )
	{
		// no path, no formation
		me->SetSquadFormationError( 0.0f );
		me->SetBrokenFormation( false );
		return Continue();
	}

	const Path::Segment *leaderSegment = leaderPath->GetCurrentGoal();

	Vector leaderForward = leaderSegment->pos - leader->GetAbsOrigin();

	// if the leader is very close to the goal, use the next goal to ensure 
	// the forward vector stays forward
	const float atGoal = 25.0f;
	if ( leaderForward.IsLengthLessThan( atGoal ) )
	{
		const Path::Segment *nextSegment = leaderPath->NextSegment( leaderSegment );
		if ( nextSegment )
		{
			leaderForward = nextSegment->pos - leader->GetAbsOrigin();
		}
	}

	leaderForward.NormalizeInPlace();

	if ( m_formationForward.IsZero() )
	{
		m_formationForward = leaderForward;
	}
	else
	{
		// limit rate of change of leader forward vector to keep formation coherent
		float maxRotation = 30.0f;	// degrees/second

		float leaderForwardYaw = UTIL_VecToYaw( leaderForward );
		float formationYaw = UTIL_VecToYaw( m_formationForward );

		float angleDiff = UTIL_AngleDiff( leaderForwardYaw, formationYaw );

		float deltaYaw = maxRotation * interval;

		if ( angleDiff < -deltaYaw )
		{
			formationYaw -= deltaYaw;
		}
		else if ( angleDiff > deltaYaw )
		{
			formationYaw += deltaYaw;
		}
		else
		{
			formationYaw += angleDiff;
		}

		FastSinCos( formationYaw * M_PI / 180.0f, &m_formationForward.y, &m_formationForward.x );
		m_formationForward.z = 0.0f;
	}


	const float maxSeparationAngle = 30.0f * M_PI / 180.0f;
	
	float formationRadius = 125.0f;
	if ( squad->GetFormationSize() > 0.0f )
	{
		formationRadius = squad->GetFormationSize();
	}

	Vector myFormationSpot;
	Vector formationForward = vec3_origin;
	float s, c;

	// where am I in the roster
	int which;
	for( which=0; which<rawMemberVector.Count(); ++which )
	{
		if ( me->IsSelf(rawMemberVector[which] ) )
		{
			break;
		}
	}

	// subtract one since the leader is always first
	--which;

	// my formation spot is assigned via my position in the roster array
	int slot = ( which + 1 ) /2;

	float formationAngle = slot * maxSeparationAngle;

	if ( which & 0x1 )
	{
		formationAngle = -formationAngle;
	}

	FastSinCos( formationAngle, &s, &c );
	formationForward.x = m_formationForward.x * c - m_formationForward.y * s;
	formationForward.y = m_formationForward.y * c + m_formationForward.x * s;

	myFormationSpot = leader->GetAbsOrigin() + formationRadius * formationForward;

	trace_t result;
	CTraceFilterIgnoreTeammates filter( me, COLLISION_GROUP_NONE, me->GetTeamNumber() );
	UTIL_TraceLine( leader->GetAbsOrigin() + Vector( 0, 0, HalfHumanHeight ), myFormationSpot + Vector( 0, 0, HalfHumanHeight ), MASK_PLAYERSOLID, &filter, &result );

	if ( result.DidHitWorld() )
	{
		myFormationSpot = result.endpos - Vector( 0, 0, HalfHumanHeight ) + 0.6f * me->GetBodyInterface()->GetHullWidth() * result.plane.normal;
	}

	if (anticitizen_bot_formation_debug.GetBool() )
	{
		NDebugOverlay::Circle( myFormationSpot, 16.0f, 0, 255, 0, 255, true, 0.1f );

		CFmtStr msg;
		NDebugOverlay::Text( myFormationSpot, msg.sprintf( "%d", which ), false, 0.1f );
	}

	// match speed with leader if I'm at/near my formation position
	Vector to = myFormationSpot - me->GetAbsOrigin();
	float error = to.Length2D();
	const float maxError = 100.0f;	// 50

	float normalizedError = 1.0f;
	if ( error < maxError )
	{
		normalizedError = error / maxError;
	}

	// this error term is used in CTFPlayer::TeamFortress_CalculateMaxSpeed() to 
	// modulate our speed
	// 0 = in position (no error)
	// 1 = far out of position (max error)
	me->SetSquadFormationError( normalizedError );
	
	// move to my formation spot
	if ( error < 50.0f )
	{
		// if we're ahead of where we want to be, just wait
		if ( DotProduct( to, formationForward ) > 0.0f )
		{
			// very close - just directly approach to avoid pathing jaggies
			me->GetLocomotionInterface()->Approach( myFormationSpot );
		}
		else
		{
			// we're in position
			me->SetSquadFormationError( 0.0f );
		}
	}
	else
	{
		if ( m_pathTimer.IsElapsed() )
		{
			m_pathTimer.Start( RandomFloat( 0.1f, 0.2f ) );

			me->SetBrokenFormation( false );

			CHL2MPBotPathCost cost( me, FASTEST_ROUTE );
			if ( m_formationPath.Compute( me, myFormationSpot, cost ) == false )
			{
				// no path back to formation
				me->SetBrokenFormation( true );
			}

			// if we have a long path to get back in formation, we've broken ranks
			const float tooFar = 750.0f;
			if ( m_formationPath.GetLength() > tooFar )
			{
				me->SetBrokenFormation( true );
			}
		}

		m_formationPath.Update( me );
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
void CAnticitizenBotEscortSquadLeader::OnEnd( CHL2MPBot *me, Action< CHL2MPBot > *nextAction )
{
}


//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot > CAnticitizenBotWaitForOutOfPositionSquadMember::OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction )
{
	m_waitTimer.Start( 2.0f );

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot > CAnticitizenBotWaitForOutOfPositionSquadMember::Update( CHL2MPBot *me, float interval )
{
	if ( m_waitTimer.IsElapsed() )
	{
		return Done( "Timeout" );
	}

	if ( !me->IsInASquad() || !me->GetSquad()->IsLeader( me ) )
	{
		return Done( "No squad" );
	}

	if ( me->GetSquad()->IsInFormation() )
	{
		// Everyone is in position
		return Done( "Everyone is in formation. Moving on." );
	}

	return Continue();
}
