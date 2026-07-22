//========= Copyright Valve Corporation, All rights reserved. ============//

#include "cbase.h"
#include "bot/hl2mp_bot.h"
#include "bot/sniper/anticitizen_bot_sniper_pistol_attack.h"
#include "hl2mp/weapon_hl2mpbasebasebludgeon.h"
#include "bot/behavior/hl2mp_bot_get_ammo.h"
#include "weapon_sniperrifle.h"

#include "nav_mesh.h"

extern ConVar hl2mp_bot_path_lookahead_range;

//---------------------------------------------------------------------------------------------
CHL2MPBotSniperPistolAttack::CHL2MPBotSniperPistolAttack(void)
{
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperPistolAttack::OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction )
{
	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CHL2MPBot >	CHL2MPBotSniperPistolAttack::Update( CHL2MPBot *me, float interval )
{
	// bash the bad guys
	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();

	if ( threat == NULL )
	{
		return Done( "No threat" );
	}

	CBaseCombatWeapon* pSniperWeapon = me->Weapon_OwnsThisType("weapon_sniperrifle");
	if (me->GetAmmoCount(pSniperWeapon->GetPrimaryAmmoType()) >= SNIPER_CHARGE_DRAIN)
	{
		me->PopRequiredWeapon();
		return Done("Sniper is charged");
	}

	// switch to our melee weapon
	CBaseHL2MPCombatWeapon* pPistolWeapon = (CBaseHL2MPCombatWeapon*)me->Weapon_OwnsThisType("weapon_dualpistols");

	if ( !pPistolWeapon)
	{
		// misyl: TF nextbot is missing this check... Interesting.
		return Done( "Don't have a pistol weapon!" );
	}

	me->PushRequiredWeapon(pPistolWeapon);

	// actual head aiming is handled elsewhere

	// just keep swinging
	me->PressFireButton();

	// chase them down
	CHL2MPBotPathCost cost( me, FASTEST_ROUTE );
	m_path.Update( me, threat->GetEntity(), cost );

	return Continue();
}
