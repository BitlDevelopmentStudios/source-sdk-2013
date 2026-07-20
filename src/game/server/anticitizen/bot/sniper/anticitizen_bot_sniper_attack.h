//========= Copyright Valve Corporation, All rights reserved. ============//
// tf_bot_sniper_attack.h
// Attack a threat as a Sniper
// Michael Booth, February 2009

#ifndef TF_BOT_SNIPER_ATTACK_H
#define TF_BOT_SNIPER_ATTACK_H

#include "Path/NextBotChasePath.h"

class CHL2MPBotSniperAttack : public Action< CHL2MPBot >
{
public:
	static bool IsPossible( CHL2MPBot *me );			// return true if this Action has what it needs to perform right now

	virtual ActionResult< CHL2MPBot >	OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction );
	virtual ActionResult< CHL2MPBot >	Update( CHL2MPBot *me, float interval );
	void OnEnd( CHL2MPBot *me, Action< CHL2MPBot > *nextAction );
	virtual ActionResult< CHL2MPBot >	OnSuspend( CHL2MPBot *me, Action< CHL2MPBot > *interruptingAction );
	virtual ActionResult< CHL2MPBot >	OnResume( CHL2MPBot *me, Action< CHL2MPBot > *interruptingAction );

	virtual Vector SelectTargetPoint( const INextBot *me, const CBaseCombatCharacter *subject ) const;		// given a subject, return the world space position we should aim at

	virtual const CKnownEntity *	SelectMoreDangerousThreat( const INextBot *me, 
															   const CBaseCombatCharacter *subject,
															   const CKnownEntity *threat1, 
															   const CKnownEntity *threat2 ) const;	// return the more dangerous of the two threats to 'subject', or NULL if we have no opinion

	virtual const char *GetName( void ) const	{ return "SniperAttack"; };

private:
	CountdownTimer m_lingerTimer;

	bool IsImmediateThreat( const CBaseCombatCharacter *subject, const CKnownEntity *threat ) const;
};

#endif // TF_BOT_SNIPER_ATTACK_H
