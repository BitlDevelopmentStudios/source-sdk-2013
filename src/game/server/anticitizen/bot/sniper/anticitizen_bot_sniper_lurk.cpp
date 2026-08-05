//========= Copyright Valve Corporation, All rights reserved. ============//
// anticitizen_bot_sniper_lurk.h
// Move into position and wait for victims
// Michael Booth, October 2009

#include "cbase.h"
#include "hl2mp_player.h"

#include "bot/hl2mp_bot.h"
#include "bot/sniper/anticitizen_bot_sniper_lurk.h"
#include "bot/sniper/anticitizen_bot_sniper_pistol_attack.h"
#include "bot/behavior/hl2mp_bot_retreat_to_cover.h"
#include "bot/behavior/hl2mp_bot_attack.h"
#include "bot/behavior/hl2mp_bot_seek_and_destroy.h"
#include "weapon_sniperrifle.h"

#include "nav_mesh.h"

extern float SkewedRandomValue( void );

ConVar anticitizen_bot_sniper_pistol_range("anticitizen_bot_sniper_pistol_range", "500", FCVAR_CHEAT, "If threat is closer than this, attack with pistol");
ConVar anticitizen_bot_sniper_patience_duration( "anticitizen_bot_sniper_patience_duration", "10", FCVAR_CHEAT, "How long a Sniper bot will wait without seeing an enemy before picking a new spot" );
ConVar anticitizen_bot_sniper_target_linger_duration( "anticitizen_bot_sniper_target_linger_duration", "2", FCVAR_CHEAT, "How long a Sniper bot will keep toward at a target it just lost sight of" );
ConVar anticitizen_bot_sniper_allow_opportunistic( "anticitizen_bot_sniper_allow_opportunistic", "1", FCVAR_NONE, "If set, Snipers will stop on their way to their preferred lurking spot to snipe at opportunistic targets" );
ConVar anticitizen_bot_sniper_linger_time("anticitizen_bot_sniper_linger_time", "5", FCVAR_CHEAT, "How long Sniper will wait around after losing his target before giving up");

//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperLurk::OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction )
{
	m_boredTimer.Start( RandomFloat( 0.9f, 1.1f ) * anticitizen_bot_sniper_patience_duration.GetFloat() );

	m_homePosition = me->GetAbsOrigin();
	m_isHomePositionValid = false;
	m_isAtHome = false;
	m_failCount = 0;

	//m_isOpportunistic = anticitizen_bot_sniper_allow_opportunistic.GetBool();

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperLurk::Update( CHL2MPBot *me, float interval )
{
	// aim at bad guys
	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();

	if (threat)
	{
		// continuously search for good sniping spots
		FindHidingSpot(me);
	}
	else
	{
		return SuspendFor(new CHL2MPBotSeekAndDestroy, "Assassin is hunting");
	}

	if ( threat && !threat->GetEntity()->IsAlive() )
	{
		// he's dead
		threat = NULL;
	}

	if ( threat && me->GetIntentionInterface()->ShouldAttack( me, threat ) == ANSWER_NO )
	{
		threat = NULL;
	}

	if ( threat && threat->IsVisibleInFOVNow() )
	{
		m_failCount = 0;

		CWeaponSniperRifle* pSniperWeapon = (CWeaponSniperRifle*)me->Weapon_OwnsThisType("weapon_sniperrifle");
		if (pSniperWeapon && (pSniperWeapon == me->GetActiveWeapon()) && !pSniperWeapon->IsReady())
		{
			return SuspendFor( new CHL2MPBotSniperPistolAttack, "Pistol attacking nearby threat" );
		}
	}

	bool isSightingRifle = false;

	if ( threat && 
		 threat->GetTimeSinceLastSeen() < anticitizen_bot_sniper_target_linger_duration.GetFloat() &&
		 me->IsLineOfFireClear( threat->GetEntity() ) )
	{
		// we see something...
		if ( m_isOpportunistic )
		{
			// switch to our sniper rifle
			CBaseHL2MPCombatWeapon* myGun = (CBaseHL2MPCombatWeapon*)me->Weapon_OwnsThisType("weapon_sniperrifle");
			if (myGun)
			{
				me->PopRequiredWeapon();
				me->PushRequiredWeapon(myGun);
			}

			isSightingRifle = true;
			m_boredTimer.Reset();

			if ( !m_isHomePositionValid )
			{
				// make this our opportunistic home for awhile
				m_homePosition = me->GetAbsOrigin();
				m_boredTimer.Start( RandomFloat( 0.9f, 1.1f ) * anticitizen_bot_sniper_patience_duration.GetFloat() );
			}
		}
		else
		{
			// switch to our SMG and fire while we run
			CBaseHL2MPCombatWeapon* myOtherGun = (CBaseHL2MPCombatWeapon*)me->Weapon_OwnsThisType("weapon_dualpistols");
			if (myOtherGun)
			{
				me->PopRequiredWeapon();
				me->PushRequiredWeapon(myOtherGun);
			}
		}
	}

	const float homeRange = 25.0f; // 100.0f;
	m_isAtHome = ( me->GetAbsOrigin() - m_homePosition ).AsVector2D().IsLengthLessThan( homeRange );

	if ( m_isAtHome )
	{
		isSightingRifle = true;

		// once we've reached a good home spot, opportunistically attack from there
		m_isOpportunistic = anticitizen_bot_sniper_allow_opportunistic.GetBool();

		if ( m_boredTimer.IsElapsed() )
		{
			++m_failCount;

			if (FindHidingSpot( me ) )
			{
				m_boredTimer.Start( RandomFloat( 0.9f, 1.1f ) * anticitizen_bot_sniper_patience_duration.GetFloat() );
			}
			else
			{
				// try again soon
				m_boredTimer.Start( 1.0f );
			}
		}
	}
	else
	{
		// not yet at home - can't start to be bored
		m_boredTimer.Reset();
	}

	if ( isSightingRifle )
	{
		CBaseHL2MPCombatWeapon* myGun = (CBaseHL2MPCombatWeapon*)me->Weapon_OwnsThisType("weapon_sniperrifle");
		if (myGun)
		{
			me->PopRequiredWeapon();
			me->PushRequiredWeapon(myGun);
			me->Weapon_Switch(myGun);
			if (threat && threat->IsVisibleInFOVNow() && !me->IsCurrentWeaponZoomed())
			{
				// zoom in and stand still
				me->PressAltFireButton();
			}
		}
	}
	else 
	{
		// move to our home position
		if ( m_repathTimer.IsElapsed() )
		{
			m_repathTimer.Start( RandomFloat( 1.0f, 2.0f ) );

			CHL2MPBotPathCost cost( me, SAFEST_ROUTE );
			m_path.Compute( me, m_homePosition, cost );
		}

		m_path.Update( me );
		
		if (me->IsCurrentWeaponZoomed())
		{
			me->PressAltFireButton();
		}
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
void CHL2MPBotSniperLurk::OnEnd( CHL2MPBot *me, Action< CHL2MPBot > *nextAction )
{
	if (me->IsCurrentWeaponZoomed())
	{
		// we're leaving to do something else - unzoom
		me->PressAltFireButton();
	}
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperLurk::OnSuspend( CHL2MPBot *me, Action< CHL2MPBot > *interruptingAction )
{
	if (me->IsCurrentWeaponZoomed())
	{
		// we're leaving to do something else - unzoom
		me->PressAltFireButton();
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperLurk::OnResume( CHL2MPBot *me, Action< CHL2MPBot > *interruptingAction )
{
	m_repathTimer.Invalidate();

	// we probably just fetched some health because the enemy shot us - pick a new place to lurk
	FindHidingSpot( me );

	return Continue();
}

//---------------------------------------------------------------------------------------------
QueryResultType CHL2MPBotSniperLurk::ShouldAttack( const INextBot *bot, const CKnownEntity *them ) const
{
	// take the shot if you've got it
	return ANSWER_YES;
}


//---------------------------------------------------------------------------------------------
QueryResultType CHL2MPBotSniperLurk::ShouldRetreat( const INextBot *me ) const
{
	return ANSWER_UNDEFINED;
}

//---------------------------------------------------------------------------------------------
// Return the more dangerous of the two threats to 'subject', or NULL if we have no opinion
const CKnownEntity *CHL2MPBotSniperLurk::SelectMoreDangerousThreat( const INextBot *meBot, 
																 const CBaseCombatCharacter *subject,
																 const CKnownEntity *threat1, 
																 const CKnownEntity *threat2 ) const
{
	// Use normal threat selection
	return NULL;
}

struct IncursionEntry_t
{
	CNavArea* area;
};

//---------------------------------------------------------------------------------------------
bool CHL2MPBotSniperLurk::FindHidingSpot(CHL2MPBot* me)
{
	CNavArea* myArea = me->GetLastKnownArea();
	if (!myArea)
	{
		return false;
	}

	if (!m_findHomeTimer.IsElapsed())
	{
		return false;
	}

	m_findHomeTimer.Start(RandomFloat(1.0f, 2.0f));

	m_hidingSpot = NULL;

	// find a spot to hide
	const float maxRange = 3500.0f;
	CUtlVector< CNavArea* > nearbyVector;
	CollectSurroundingAreas(&nearbyVector, me->GetLastKnownArea(), maxRange,
		500.0f, 500.0f);

	CUtlVector< IncursionEntry_t > hidingSpotVector;

	for (int i = 0; i < nearbyVector.Count(); ++i)
	{
		CNavArea* area = nearbyVector[i];

		if (area->GetHidingSpots()->Count() <= 0)
		{
			continue;
		}

		IncursionEntry_t entry = { area };
		hidingSpotVector.AddToTail(entry);
	}

	if (hidingSpotVector.Count() <= 0)
	{
		return false;
	}

	// penetrate as far as we can
	int which = RandomInt(0, hidingSpotVector.Count() / 2);
	CNavArea* whichArea = hidingSpotVector[which].area;

	const HidingSpotVector* hidingSpots = whichArea->GetHidingSpots();

	if (hidingSpots->Count() > 0)
	{
		m_hidingSpot = hidingSpots->Element(RandomInt(0, hidingSpots->Count() - 1));

		m_isHomePositionValid = true;
		m_homePosition = m_hidingSpot->GetPosition();
		return true;
	}

	m_isHomePositionValid = false;
	m_homePosition = me->GetAbsOrigin();

	return true;
}