//========= Copyright Valve Corporation, All rights reserved. ============//

#ifndef HL2MP_BOT_MELEE_ATTACK_H
#define HL2MP_BOT_MELEE_ATTACK_H

#include "Path/NextBotChasePath.h"

#define SNIPER_CHARGE_LEVEL_TO_WAIT_FOR_NORMAL 75
#define SNIPER_CHARGE_LEVEL_TO_WAIT_FOR_HARD 100

class CHL2MPBotSniperPistolAttack : public Action< CHL2MPBot >
{
public:
	CHL2MPBotSniperPistolAttack(void);

	virtual ActionResult< CHL2MPBot >	OnStart( CHL2MPBot *me, Action< CHL2MPBot > *priorAction );
	virtual ActionResult< CHL2MPBot >	Update( CHL2MPBot *me, float interval );

	virtual const char *GetName( void ) const	{ return "SniperPistolAttack"; };

private:
	bool m_LowAmmo;
	ChasePath m_path;
};

#endif // HL2MP_BOT_MELEE_ATTACK_H
