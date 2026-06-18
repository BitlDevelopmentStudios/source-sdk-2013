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
#include "hl2mp_gamerules.h"

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

		if ((pAttacker == C_BasePlayer::GetLocalPlayer()) && (pVictim != C_BasePlayer::GetLocalPlayer()))
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

class CAchievementCombineFriendlyFire : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL | ACH_LISTEN_KILL_EVENTS | ACH_FILTER_ATTACKER_IS_PLAYER);
		SetGoal(1);
	}

	virtual void Event_EntityKilled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, IGameEvent* event)
	{
		if (!pVictim || !pVictim->IsPlayer())
			return;

		if ((pAttacker == C_BasePlayer::GetLocalPlayer()) && (pVictim != C_BasePlayer::GetLocalPlayer()))
		{
			// friendly fire kills
			if (pVictim->GetTeamNumber() == pAttacker->GetTeamNumber())
			{
				// we killed a soldier
				IncrementCount();
			}
		}
	}
};
DECLARE_ACHIEVEMENT(CAchievementCombineFriendlyFire, ACHIEVEMENT_ANTICITIZEN_KILL_COMBINE_FRIENDLYFIRE, "ANTICITIZEN_KILL_COMBINE_FRIENDLYFIRE", 1);

class CAchievementKillMilestone_Base : public CBaseAchievement
{
public:
	virtual void Setup(int goal, int attackerTeam)
	{
		SetFlags(ACH_SAVE_GLOBAL | ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS);
		SetGoal(goal);
		m_iTeamNumber = attackerTeam;
	}

	virtual void Event_EntityKilled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, IGameEvent* event)
	{
		if (!pVictim || !pVictim->IsPlayer())
			return;

		if ((pAttacker == C_BasePlayer::GetLocalPlayer()) && (pVictim != C_BasePlayer::GetLocalPlayer()))
		{
			// no friendly fire kills
			if (pVictim->GetTeamNumber() != pAttacker->GetTeamNumber())
			{
				CHL2MP_Player* pHL2MPAttacker = ToHL2MPPlayer(pAttacker);
				if (pHL2MPAttacker && (pHL2MPAttacker->GetTeamNumber() == m_iTeamNumber))
				{
					// we killed a soldier
					IncrementCount();
				}
			}
		}
	}

private:
	int m_iTeamNumber;
};

class CAchievementCombineKillMilestone1 : public CAchievementKillMilestone_Base
{
protected:
	virtual void Init()
	{
		Setup(25, TEAM_COMBINE);
	}
};
DECLARE_ACHIEVEMENT(CAchievementCombineKillMilestone1, ACHIEVEMENT_ANTICITIZEN_COMBINE_MILESTONE_1, "ANTICITIZEN_COMBINE_MILESTONE_1", 10);

class CAchievementCombineKillMilestone2 : public CAchievementKillMilestone_Base
{
protected:
	virtual void Init()
	{
		Setup(50, TEAM_COMBINE);
	}
};
DECLARE_ACHIEVEMENT(CAchievementCombineKillMilestone2, ACHIEVEMENT_ANTICITIZEN_COMBINE_MILESTONE_2, "ANTICITIZEN_COMBINE_MILESTONE_2", 10);

class CAchievementCombineKillMilestone3 : public CAchievementKillMilestone_Base
{
protected:
	virtual void Init()
	{
		Setup(75, TEAM_COMBINE);
	}
};
DECLARE_ACHIEVEMENT(CAchievementCombineKillMilestone3, ACHIEVEMENT_ANTICITIZEN_COMBINE_MILESTONE_3, "ANTICITIZEN_COMBINE_MILESTONE_3", 10);

class CAchievementFreemanKillMilestone1 : public CAchievementKillMilestone_Base
{
protected:
	virtual void Init()
	{
		Setup(25, TEAM_FREEMAN);
	}
};
DECLARE_ACHIEVEMENT(CAchievementFreemanKillMilestone1, ACHIEVEMENT_ANTICITIZEN_FREEMAN_MILESTONE_1, "ANTICITIZEN_FREEMAN_MILESTONE_1", 10);

class CAchievementFreemanKillMilestone2 : public CAchievementKillMilestone_Base
{
protected:
	virtual void Init()
	{
		Setup(50, TEAM_FREEMAN);
	}
};
DECLARE_ACHIEVEMENT(CAchievementFreemanKillMilestone2, ACHIEVEMENT_ANTICITIZEN_FREEMAN_MILESTONE_2, "ANTICITIZEN_FREEMAN_MILESTONE_2", 10);

class CAchievementFreemanKillMilestone3 : public CAchievementKillMilestone_Base
{
protected:
	virtual void Init()
	{
		Setup(75, TEAM_FREEMAN);
	}
};
DECLARE_ACHIEVEMENT(CAchievementFreemanKillMilestone3, ACHIEVEMENT_ANTICITIZEN_FREEMAN_MILESTONE_3, "ANTICITIZEN_FREEMAN_MILESTONE_3", 10);

class CAchievementWinGames : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(98);
	}

	virtual void ListenForEvents()
	{
		ListenForGameEvent("anticitizen_round_end");
	}

	void FireGameEvent_Internal(IGameEvent* event)
	{
		if (FStrEq(event->GetName(), "anticitizen_round_end"))
		{
			// Were we on the winning team?
			int iTeam = event->GetInt("team");
			if ((iTeam >= FIRST_GAME_TEAM) && (iTeam == GetLocalPlayerTeam()))
			{
				IncrementCount();
			}
		}
	}
};
DECLARE_ACHIEVEMENT(CAchievementWinGames, ACHIEVEMENT_ANTICITIZEN_GENERAL_WINGAMES, "ANTICITIZEN_GENERAL_WINGAMES", 10);

class CAchievementKill : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL | ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS);
		SetGoal(420);
	}

	virtual void Event_EntityKilled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor, IGameEvent* event)
	{
		if (!pVictim || !pVictim->IsPlayer())
			return;

		if ((pAttacker == C_BasePlayer::GetLocalPlayer()) && (pVictim != C_BasePlayer::GetLocalPlayer()))
		{
			// no friendly fire kills
			if (pVictim->GetTeamNumber() != pAttacker->GetTeamNumber())
			{
				// we killed a soldier
				IncrementCount();
			}
		}
	}
};
DECLARE_ACHIEVEMENT(CAchievementKill, ACHIEVEMENT_ANTICITIZEN_GENERAL_KILLS, "ANTICITIZEN_GENERAL_KILLS", 10);
#endif // GAME_DLL
