//========= Copyright Valve Corporation, All rights reserved. ============//
// hl2mp_bot_use_item.h
// Equip and consume an item
// Michael Booth, July 2011

#include "cbase.h"
#include "hl2mp/weapon_hl2mpbasehlmpcombatweapon.h"
#include "bot/hl2mp_bot.h"
#include "bot/behavior/hl2mp_bot_use_item.h"

//---------------------------------------------------------------------------------------------
CHL2MPBotUseItem::CHL2MPBotUseItem(CBaseHL2MPCombatWeapon* item)
{
	m_item = item;
}

//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotUseItem::OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction )
{
	// force-equip the item we're going to use
	me->PushRequiredWeapon( m_item );

	m_cooldownTimer.Start( m_item->m_flNextPrimaryAttack - gpGlobals->curtime + 0.25f );

	return Continue();
}

//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotUseItem::Update( CHL2MPBot *me, float interval )
{
	if ( m_item == NULL )
	{
		return Done( "NULL item" );
	}

	CBaseHL2MPCombatWeapon *myCurrentWeapon = (CBaseHL2MPCombatWeapon*)me->GetActiveWeapon();

	if ( !myCurrentWeapon )
	{
		return Done( "NULL weapon" );
	}

	if ( m_cooldownTimer.HasStarted() )
	{
		if ( m_cooldownTimer.IsElapsed() )
		{
			// use it
			me->PressFireButton();
			m_cooldownTimer.Invalidate();
		}
	}
	else // used
	{
		return Done("Item used");
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
void CHL2MPBotUseItem::OnEnd( CHL2MPBot *me, Action< CHL2MPBot > *nextAction )
{
	me->PopRequiredWeapon();
}

