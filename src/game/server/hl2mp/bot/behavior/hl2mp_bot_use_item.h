//========= Copyright Valve Corporation, All rights reserved. ============//
// tf_bot_use_item.h
// Equip and consume an item
// Michael Booth, July 2011

#ifndef HL2MP_BOT_USE_ITEM_SECONDARY_FIRE_H
#define HL2MP_BOT_USE_ITEM_SECONDARY_FIRE_H

class CHL2MPBotUseItem : public Action< CHL2MPBot >
{
public:
	CHL2MPBotUseItem( CBaseHL2MPCombatWeapon* item, bool altFire );
	virtual ~CHL2MPBotUseItem() { }

	virtual ActionResult< CHL2MPBot >	OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction );

	virtual const char *GetName( void ) const	{ return "UseItem"; };

private:
	CHandle< CBaseHL2MPCombatWeapon > m_item;
	bool m_AltFire;
};


#endif // TF_BOT_USE_ITEM_H
