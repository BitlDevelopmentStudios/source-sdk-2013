#ifndef SERVERADMIN_COMMAND_TF_H
#define SERVERADMIN_COMMAND_TF_H

#include "admin\base_serveradmin.h"
#include "filesystem.h"
#include "hl2mp_player.h"
#include "hl2mp_gamerules.h"

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
		AdminReply(replySource, pAdmin, "0 = Metropolice, 1 = Combine Soldier, 2 = Combine Shotgunner, 3 = Combine Elite, 4 = Combine Assassin");
		return;
	}

	const char* partialName = args.Arg(2);
	int classIndex = atoi(args.Arg(3));

	if (classIndex < CLS_FIRST_COMBINE_CLASS || classIndex > CLS_LAST_COMBINE_CLASS)
	{
		AdminReply(replySource, pAdmin, "Invalid class index. Class index must be between 0 and 4. To set yourself or a target to Gordon Freeman, use sa forcefreeman or sa freeman.");
		return;
	}

	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget))
		return;

	const char* className = g_Anticitizen_PR->GetPlayerClassInfo(classIndex).m_szPlayerClassName;

	auto MovePlayerToClass = [classIndex](CHL2MP_Player* pPlayer)
		{
			pPlayer->HandleCommand_JoinClass(classIndex);
		};

	CUtlString logDetails, chatMessage;

	if (pTarget)
	{
		CHL2MP_Player* pAnticitizenTarget = ToHL2MPPlayer(pTarget);

		if (pAnticitizenTarget)
		{
			int iCurClass = pAnticitizenTarget->GetPlayerClass();
			if (iCurClass == classIndex)
			{
				AdminReply(replySource, pAdmin, "Player %s is already on class %s.", pTarget->GetPlayerName(), className);
				return;
			}

			MovePlayerToClass(pAnticitizenTarget);

			CBase_Admin::LogAction(pAdmin, pAnticitizenTarget, "moved", UTIL_VarArgs("to class %s", className));

			CUtlString classMessage;

			if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
			{
				classMessage.Format("Console moved player %s to class %s.", pAnticitizenTarget->GetPlayerName(), className);
			}
			else
			{
				classMessage.Format("Admin %s moved player %s to class %s.", pAdmin->GetPlayerName(), pAnticitizenTarget->GetPlayerName(), className);
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
				CHL2MP_Player* pAnticitizenTarget = ToHL2MPPlayer(pPlayer);

				if (pAnticitizenTarget)
				{
					// Skip players already on the desired class
					int iCurClass = pAnticitizenTarget->GetPlayerClass();
					if (iCurClass == classIndex)
					{
						continue;
					}

					MovePlayerToClass(pAnticitizenTarget);
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

static void ForceFreemanCommand(const CCommand& args)
{
	CBasePlayer* pAdmin = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pAdmin);

	if (!pAdmin && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (args.ArgC() < 3)
	{
		AdminReply(replySource, pAdmin, "Usage: sa forcefreeman <name|#userID>");
		return;
	}

	const char* partialName = args.Arg(2);
	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget, true))
		return;

	if (targetPlayers.Count() > 1)
	{
		AdminReply(replySource, pAdmin, "This command only works for one player.");
		return;
	}
	else
	{
		if (targetPlayers.Count() == 1)
		{
			pTarget = targetPlayers[0];
		}
	}

	if (pTarget)
	{
		CHL2MP_Player* pAnticitizenTarget = ToHL2MPPlayer(pTarget);

		if (pAnticitizenTarget)
		{
			HL2MPRules()->SetNextPlayerToBecomeFreeman(pAnticitizenTarget);

			CBase_Admin::LogAction(pAdmin, pAnticitizenTarget, "Gordon Freeman set for", "");

			if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
			{
				Msg("Console set Gordon Freeman for player %s.\n", pAnticitizenTarget->GetPlayerName());
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
					"Console set Gordon Freeman for %s\n",
					pAnticitizenTarget->GetPlayerName()
				));
			}
			else
			{
				UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
					"Admin %s set Gordon Freeman for %s\n",
					pAdmin ? pAdmin->GetPlayerName() : "Console", pAnticitizenTarget->GetPlayerName()
				));
			}
		}
	}
}

#define ANTICITIZEN_COMMAND_MODULE_NAME "ANTICITIZEN ONE Commands"

static void LoadAnticitizenCommandModule()
{
	REGISTER_ADMIN_COMMAND(ANTICITIZEN_COMMAND_MODULE_NAME, "class", true, NULL, "<name|#userID> <class index> -> Move a player to another class", "f", ClassPlayerCommand);
	REGISTER_ADMIN_COMMAND(ANTICITIZEN_COMMAND_MODULE_NAME, "forcefreeman", true, NULL, "<name|#userID> -> Set a player to play as Gordon Freeman in the next round.", "f", ForceFreemanCommand);
	REGISTER_ADMIN_COMMAND(ANTICITIZEN_COMMAND_MODULE_NAME, "freeman", true, NULL, "<name|#userID> -> Set a player to play as Gordon Freeman in the next round. Alias of forcefreeman", "f", ForceFreemanCommand);
}

#endif // SERVERADMIN_COMMAND_TF_H