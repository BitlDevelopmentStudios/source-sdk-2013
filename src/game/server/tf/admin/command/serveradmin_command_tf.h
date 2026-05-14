#ifndef SERVERADMIN_COMMAND_TF_H
#define SERVERADMIN_COMMAND_TF_H

#include "admin\base_serveradmin.h"
#include "filesystem.h"
#include "tf_player.h"

#ifdef BDSBASE
//-----------------------------------------------------------------------------
// Purpose: Change a player's class
//-----------------------------------------------------------------------------
static void ClassPlayerCommand(const CCommand& args)
{
	CBasePlayer* pAdmin = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pAdmin);

	if (!pAdmin && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (args.ArgC() < 4)
	{
		AdminReply(replySource, pAdmin, "Usage: sa class <name|#userID> <class index>");
		AdminReply(replySource, pAdmin, "1 = Scout, 2 = Sniper, 3 = Soldier, 4 = Demoman, 5 = Medic, 6 = Heavy, 7 = Pyro, 8 = Spy, 9 = Engineer, 10 = Civilian");
		return;
	}

	const char* partialName = args.Arg(2);
	int classIndex = atoi(args.Arg(3));

	if (classIndex < TF_FIRST_NORMAL_CLASS || classIndex > TF_LAST_NORMAL_CLASS)
	{
		AdminReply(replySource, pAdmin, "Invalid class index. Class index must be between 1 and 10.");
		return;
	}

	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget))
		return;

	const char* className = GetPlayerClassData(classIndex)->m_szClassName;

	auto MovePlayerToClass = [classIndex](CTFPlayer* pPlayer)
		{
			pPlayer->SetDesiredPlayerClassIndex(classIndex);
			IGameEvent* event = gameeventmanager->CreateEvent("player_changeclass");
			if (event)
			{
				event->SetInt("userid", pPlayer->GetUserID());
				event->SetInt("class", classIndex);

				gameeventmanager->FireEvent(event);
			}
			pPlayer->ForceRespawn();
		};

	CUtlString logDetails, chatMessage;

	if (pTarget)
	{
		CTFPlayer* pTFTarget = ToTFPlayer(pTarget);

		if (pTFTarget)
		{
			if (pTFTarget->GetPlayerClass()->GetClassIndex() == classIndex)
			{
				AdminReply(replySource, pAdmin, "Player %s is already on class %s.", pTarget->GetPlayerName(), className);
				return;
			}

			MovePlayerToClass(pTFTarget);

			CBase_Admin::LogAction(pAdmin, pTFTarget, "moved", UTIL_VarArgs("to class %s", className));

			CUtlString classMessage;

			if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
			{
				classMessage.Format("Console moved player %s to class %s.", pTFTarget->GetPlayerName(), className);
			}
			else
			{
				classMessage.Format("Admin %s moved player %s to class %s.", pAdmin->GetPlayerName(), pTFTarget->GetPlayerName(), className);
			}

			UTIL_ClientPrintAll(HUD_PRINTTALK, classMessage.Get());
		}
	}
	else
	{
		if (targetPlayers.Count() == 0)
		{
			AdminReply(replySource, pAdmin, "No players found matching the criteria.");
			return;
		}

		int movedPlayersCount = 0;

		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			CBasePlayer* pPlayer = targetPlayers[i];

			if (pPlayer)
			{
				CTFPlayer* pTFPlayer = ToTFPlayer(pPlayer);

				if (pTFPlayer)
				{
					// Skip players already on the desired class
					if (pTFPlayer->GetPlayerClass()->GetClassIndex() == classIndex)
					{
						continue;
					}

					MovePlayerToClass(pTFPlayer);
					movedPlayersCount++;
				}
			}
		}

		if (movedPlayersCount == 0)
		{
			AdminReply(replySource, pAdmin, "All selected players are already on class %s.", className);
			return;
		}

		BuildGroupTargetMessage(partialName, pAdmin, "moved", NULL, logDetails, chatMessage, false);

		CBase_Admin::LogAction(pAdmin, NULL, "moved", UTIL_VarArgs("%s to class %s", logDetails.Get(), className), partialName + 1);

		chatMessage.Append(UTIL_VarArgs(" to class %s", className));

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.", chatMessage.Get()));
	}
}

static void GiveCond(CTFPlayer* pPlayer, ETFCond eCond)
{
	if (pPlayer->m_Shared.InCond(eCond))
	{
		pPlayer->m_Shared.RemoveCond(eCond);
	}
	else
	{
		pPlayer->m_Shared.AddCond(eCond);
	}
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void ConditionPlayerCommand(const CCommand& args)
{
	CBasePlayer* pAdmin = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pAdmin);

	if (!pAdmin && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (args.ArgC() < 4)
	{
		AdminReply(replySource, pAdmin, "Usage: sa cond <name|#userID> <cond ID to add/remove>");
		return;
	}

	const char* partialName = args.Arg(2);
	int condIndex = atoi(args.Arg(3));
	ETFCond eCond = TFCondIndexToEnum(condIndex);

	if (eCond == TF_COND_INVALID)
	{
		AdminReply(replySource, pAdmin, "Invalid cond index. Class index must be between 0 and %i.", TF_COND_LAST);
		return;
	}

	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget))
		return;

	CUtlString logDetails, chatMessage;

	if (pTarget)
	{
		CTFPlayer* pTFTarget = ToTFPlayer(pTarget);

		if (pTFTarget)
		{
			GiveCond(pTFTarget, eCond);

			CBase_Admin::LogAction(pAdmin, pTFTarget, "updated condition on", UTIL_VarArgs("Condition: %i.", eCond));

			CUtlString classMessage;

			if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
			{
				classMessage.Format("Console updated condition on player %s.", pTFTarget->GetPlayerName());
			}
			else
			{
				classMessage.Format("Admin %s updated condition on player %s.", pAdmin->GetPlayerName(), pTFTarget->GetPlayerName());
			}

			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s. Condition: %i.", classMessage.Get(), eCond));
		}
	}
	else
	{
		if (targetPlayers.Count() == 0)
		{
			AdminReply(replySource, pAdmin, "No players found matching the criteria.");
			return;
		}

		int movedPlayersCount = 0;

		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			CBasePlayer* pPlayer = targetPlayers[i];

			if (pPlayer)
			{
				CTFPlayer* pTFPlayer = ToTFPlayer(pPlayer);

				if (pTFPlayer)
				{
					GiveCond(pTFPlayer, eCond);
					movedPlayersCount++;
				}
			}
		}

		if (movedPlayersCount == 0)
		{
			AdminReply(replySource, pAdmin, "All selected players have updated conditions.");
			return;
		}

		BuildGroupTargetMessage(partialName, pAdmin, "updated condition on", NULL, logDetails, chatMessage, false);

		CBase_Admin::LogAction(pAdmin, NULL, "updated condition on", UTIL_VarArgs("%s Condition: %i.", logDetails.Get(), eCond), partialName + 1);

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s. Condition: %i.", chatMessage.Get(), eCond));
	}
}

#define TF_COMMAND_MODULE_NAME "Team Fortress 2 Commands"
#if defined(QUIVER_DLL)
#define QF_COMMAND_SUB_MODULE_NAME "Quiver Fortress Commands"
#endif

static void LoadTFCommandModule()
{
	// TODO: add commands
	REGISTER_ADMIN_COMMAND(TF_COMMAND_MODULE_NAME, "class", true, NULL, "<name|#userID> <class index> -> Move a player to another class", "f", ClassPlayerCommand);
	REGISTER_ADMIN_COMMAND(TF_COMMAND_MODULE_NAME, "cond", true, NULL, "<name|#userID> <cond ID to add/remove> -> Add or remove a cond on a class.", "f", ConditionPlayerCommand);
}

#endif

#endif // SERVERADMIN_COMMAND_TF_H