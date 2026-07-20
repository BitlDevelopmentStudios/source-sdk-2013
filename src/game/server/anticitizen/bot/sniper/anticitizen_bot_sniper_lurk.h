//========= Copyright Valve Corporation, All rights reserved. ============//
// tf_bot_sniper_lurk.h
// Move into position and wait for victims
// Michael Booth, October 2009

#ifndef HL2MP_BOT_SNIPER_LURK_H
#define HL2MP_BOT_SNIPER_LURK_H

#include "Path/NextBotPathFollow.h"

class CHL2MPBotSniperLurk : public Action< CHL2MPBot >
{
public:
	virtual ActionResult< CHL2MPBot >	OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction );
	virtual ActionResult< CHL2MPBot >	Update( CHL2MPBot *me, float interval );
	virtual void					OnEnd( CHL2MPBot *me, Action< CHL2MPBot > *nextAction );
	virtual ActionResult< CHL2MPBot >	OnSuspend( CHL2MPBot *me, Action< CHL2MPBot > *interruptingAction );
	virtual ActionResult< CHL2MPBot >	OnResume( CHL2MPBot *me, Action< CHL2MPBot > *interruptingAction );

	// Snipers choose their targets a bit differently
	virtual const CKnownEntity *	SelectMoreDangerousThreat( const INextBot *me, 
															   const CBaseCombatCharacter *subject,
															   const CKnownEntity *threat1, 
															   const CKnownEntity *threat2 ) const;	// return the more dangerous of the two threats to 'subject', or NULL if we have no opinion

	virtual QueryResultType ShouldAttack( const INextBot *me, const CKnownEntity *them ) const;	// should we attack "them"?
	virtual QueryResultType ShouldRetreat( const INextBot *me ) const;							// is it time to retreat?

	virtual const char *GetName( void ) const	{ return "SniperLurk"; };

private:
	CountdownTimer m_boredTimer;
	CountdownTimer m_repathTimer;
	PathFollower m_path;
	int m_failCount;

	Vector m_homePosition;			// where we want to snipe from
	bool m_isHomePositionValid;
	bool m_isAtHome;
	CountdownTimer m_findHomeTimer;
	bool m_isOpportunistic;

	HidingSpot* m_hidingSpot;
	bool FindHidingSpot(CHL2MPBot* me);
};

#endif // TF_BOT_SNIPER_LURK_H
