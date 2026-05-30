//========= Copyright Valve Corporation, All rights reserved. ============//
// tf_bot_escort_squad_leader.h
// Escort the squad leader to their destination
// Michael Booth, Octoboer 2011

#ifndef TF_BOT_ESCORT_SQUAD_LEADER_H
#define TF_BOT_ESCORT_SQUAD_LEADER_H


#include "Path/NextBotPathFollow.h"

//-----------------------------------------------------------------------------
class CAnticitizenBotEscortSquadLeader : public Action< CHL2MPBot >
{
public:
	CAnticitizenBotEscortSquadLeader( Action< CHL2MPBot > *actionToDoAfterSquadDisbands = NULL );
	virtual ~CAnticitizenBotEscortSquadLeader() { }

	virtual ActionResult< CHL2MPBot >	OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction );
	virtual ActionResult< CHL2MPBot >	Update( CHL2MPBot *me, float interval );
	virtual void					OnEnd( CHL2MPBot *me, Action< CHL2MPBot > *nextAction );

	virtual const char *GetName( void ) const	{ return "EscortSquadLeader"; };

private:
	Action< CHL2MPBot > *m_actionToDoAfterSquadDisbands;

	PathFollower m_formationPath;
	CountdownTimer m_pathTimer;

	const Vector &GetFormationForwardVector( CHL2MPBot *me );
	Vector m_formationForward;
};


//-----------------------------------------------------------------------------
class CAnticitizenBotWaitForOutOfPositionSquadMember : public Action< CHL2MPBot >
{
public:
	virtual ~CAnticitizenBotWaitForOutOfPositionSquadMember() { }

	virtual ActionResult< CHL2MPBot >	OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction );
	virtual ActionResult< CHL2MPBot >	Update( CHL2MPBot *me, float interval );

	virtual const char *GetName( void ) const	{ return "WaitForOutOfPositionSquadMember"; };

private:
	CountdownTimer m_waitTimer;
};


#endif // TF_BOT_ESCORT_SQUAD_LEADER_H
