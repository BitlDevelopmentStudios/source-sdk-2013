//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================


#include "cbase.h"
#include "achievements_anticitizen.h"

#ifdef CLIENT_DLL

#include "achievementmgr.h"
#include "baseachievement.h"
#include "hl2mp_player_shared.h"
#include "c_hl2mp_player.h"

CAchievementMgr g_AchievementMgrAnticitizen;	// global achievement mgr for HL2

class CAchievementKillFreeman : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1);
	}

	//earned through entity/s itself
};
DECLARE_ACHIEVEMENT(CAchievementKillFreeman, ACHIEVEMENT_ANTICITIZEN_KILL_FREEMAN, "ANTICITIZEN_KILL_FREEMAN", 10);

class CAchievementKillCombine : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1);
	}

	//earned through entity/s itself
};
DECLARE_ACHIEVEMENT(CAchievementKillCombine, ACHIEVEMENT_ANTICITIZEN_KILL_COMBINE, "ANTICITIZEN_KILL_COMBINE", 10);

class CAchievementKillFreemanInLessTime : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1);
	}

	//earned through entity/s itself
};
DECLARE_ACHIEVEMENT(CAchievementKillFreemanInLessTime, ACHIEVEMENT_ANTICITIZEN_KILL_FREEMAN_LESSTIME, "ANTICITIZEN_KILL_FREEMAN_LESSTIME", 10);

class CAchievementKillCombineWithCrowbar : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL | ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS);
		SetGoal(25);
	}

	virtual void Event_EntityKilled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, IGameEvent* event)
	{
		if (!pVictim || !pVictim->IsPlayer())
			return;

		if (pAttacker == C_BasePlayer::GetLocalPlayer())
		{
			// no friendly fire kills
			if (pVictim->GetTeamNumber() != pAttacker->GetTeamNumber())
			{
				CHL2MP_Player* pHL2MPAttacker = ToHL2MPPlayer(pAttacker);
				if (pHL2MPAttacker && (pHL2MPAttacker->GetPlayerClass() == CLS_FREEMAN))
				{
					CBaseCombatWeapon* pDefaultWeapon = pHL2MPAttacker->Weapon_OwnsThisType("weapon_crowbar");

					if (pDefaultWeapon == pHL2MPAttacker->GetActiveWeapon())
					{
						// we killed a soldier
						IncrementCount();
					}
				}
			}
		}
	}
};
DECLARE_ACHIEVEMENT(CAchievementKillCombineWithCrowbar, ACHIEVEMENT_ANTICITIZEN_KILL_COMBINE_CROWBAR, "ANTICITIZEN_KILL_COMBINE_CROWBAR", 10);

class CAchievementKillCombineGravityGun : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1);
	}

	//earned through entity/s itself
};
DECLARE_ACHIEVEMENT(CAchievementKillCombineGravityGun, ACHIEVEMENT_ANTICITIZEN_KILL_COMBINE_GRAVITYGUN, "ANTICITIZEN_KILL_COMBINE_GRAVITYGUN", 10);
#endif // GAME_DLL
