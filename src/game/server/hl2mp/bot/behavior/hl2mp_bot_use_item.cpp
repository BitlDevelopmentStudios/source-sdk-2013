//========= Copyright Valve Corporation, All rights reserved. ============//
// hl2mp_bot_use_item.h
// Equip and consume an item
// Michael Booth, July 2011

#include "cbase.h"
#include "hl2mp/weapon_hl2mpbasehlmpcombatweapon.h"
#include "bot/hl2mp_bot.h"
#include "bot/behavior/hl2mp_bot_use_item.h"

//---------------------------------------------------------------------------------------------
CHL2MPBotUseItem::CHL2MPBotUseItem(CBaseHL2MPCombatWeapon* item, bool altFire)
{
	m_item = item;
	m_AltFire = altFire;
}

//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotUseItem::OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction )
{
	if (m_item == NULL)
	{
		return Done("NULL item");
	}

	if (me->Physcannon_GetHeldProp() != NULL)
	{
		return Done("I have a prop! :D");
	}

	// force-equip the item we're going to use
	me->PushRequiredWeapon( m_item );

	CBaseHL2MPCombatWeapon* myCurrentWeapon = (CBaseHL2MPCombatWeapon*)me->GetActiveWeapon();

	if (!myCurrentWeapon)
	{
		return Done("NULL weapon");
	}

	if (m_AltFire)
	{
		if (me->GetAmmoCount(myCurrentWeapon->GetSecondaryAmmoType()) > 0)
		{
			return Done("No weapon alt ammo");
		}
		else
		{
			if (!me->IsFreeman())
			{
				if (me->GetClassType() != CLS_TYPE_HIGH_TIER)
				{
					return Done("Cannot use secondary fire");
				}
				else if (me->m_HL2Local.m_bNewSprinting)
				{
					return Done("Cannot use secondary fire while sprinting");
				}
			}
		}
	}
	else
	{
		if (me->GetAmmoCount(myCurrentWeapon->GetPrimaryAmmoType()) > 0)
		{
			return Done("No weapon ammo");
		}
	}

	// use it
	if (m_AltFire)
	{
		me->PressAltFireButton();
	}
	else
	{
		me->PressFireButton();
	}

	me->PopRequiredWeapon();

	return Done("Item used");
}