#ifndef SERVERADMIN_COMMAND_BASE_H
#define SERVERADMIN_COMMAND_BASE_H

#include "admin\base_serveradmin.h"
#include "filesystem.h"
#ifdef HL2MP
#include "hl2mp_player.h"
#elif TF_DLL
#include "tf_player.h"
#endif

#ifdef BDSBASE

extern bool bAdminMapChange;

#ifndef Q_max
#define Q_max(a, b) ((a) > (b) ? (a) : (b))
#endif

//-----------------------------------------------------------------------------
// Purpose: Admin say - Makes admin messages stand out for everyone
//-----------------------------------------------------------------------------
static void AdminSay( const CCommand &args )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource( pPlayer );

	if ( !pPlayer && replySource != ADMIN_REPLY_SERVER_CONSOLE )
	{
		Msg( "Command must be issued by a player or the server console.\n" );
		return;
	}

	if ( args.ArgC() < 3 )
	{
		AdminReply( replySource, pPlayer, "Usage: sa say <message>" );
		return;
	}

	// Assemble message
	CUtlString messageText;
	for ( int i = 2; i < args.ArgC(); ++i )
	{
		messageText.Append( args[ i ] );
		if ( i < args.ArgC() - 1 )
		{
			messageText.Append( " " );
		}
	}

	CBase_Admin::LogAction( pPlayer, NULL, "sent admin message", messageText.Get() );

	if ( replySource == ADMIN_REPLY_SERVER_CONSOLE )
	{
		UTIL_ClientPrintAll( HUD_PRINTTALK, UTIL_VarArgs( "\x04(ADMIN) Console: \x01%s\n", messageText.Get() ) );
	}
	else
	{
		UTIL_ClientPrintAll( HUD_PRINTTALK, UTIL_VarArgs( "\x04(ADMIN) %s: \x01%s\n", pPlayer->GetPlayerName(), messageText.Get() ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Admin center say - Posts a centered message like the 
//			"Node Graph Out of Date - Rebuilding" message
//-----------------------------------------------------------------------------
static void AdminCSay( const CCommand &args )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource( pPlayer );

	if ( !pPlayer && replySource != ADMIN_REPLY_SERVER_CONSOLE )
	{
		Msg( "Command must be issued by a player or the server console.\n" );
		return;
	}

	if ( args.ArgC() < 3 )
	{
		AdminReply( replySource, pPlayer, "Usage: sa csay <message>" );
		return;
	}

	// Assemble message
	CUtlString messageText;
	for ( int i = 2; i < args.ArgC(); ++i )
	{
		messageText.Append( args[ i ] );
		if ( i < args.ArgC() - 1 )
		{
			messageText.Append( " " );
		}
	}

	CBase_Admin::LogAction( pPlayer, NULL, "sent centered admin message", messageText.Get() );

	if ( replySource == ADMIN_REPLY_SERVER_CONSOLE )
	{
		UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "CONSOLE: %s\n", messageText.Get() ) );
	}
	else
	{
		UTIL_ClientPrintAll( HUD_PRINTCENTER, UTIL_VarArgs( "%s: %s\n", pPlayer->GetPlayerName(), messageText.Get() ) );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Admin chat (only other admins can see those messages)
//-----------------------------------------------------------------------------
static void AdminChat( const CCommand &args )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	bool isServerConsole = !pPlayer && UTIL_IsCommandIssuedByServerAdmin();

	if ( !pPlayer && !isServerConsole )
	{
		Msg( "Command must be issued by a player or the server console.\n" );
		return;
	}

	AdminReplySource replySource = GetCmdReplySource( pPlayer );

	if ( args.ArgC() < 3 )
	{
		AdminReply( replySource, pPlayer, "Usage: sa chat <message>" );
		return;
	}

	// Construct the message
	CUtlString messageText;
	for ( int i = 2; i < args.ArgC(); ++i )
	{
		messageText.Append( args[ i ] );
		if ( i < args.ArgC() - 1 )
		{
			messageText.Append( " " );
		}
	}

	// Format the message
	CUtlString formattedMessage;
	if ( isServerConsole )
	{
		formattedMessage = UTIL_VarArgs( "\x04(Admin Chat) Console: \x01%s\n", messageText.Get() );
	}
	else
	{
		formattedMessage = UTIL_VarArgs( "\x04(Admin Chat) %s: \x01%s\n", pPlayer->GetPlayerName(), messageText.Get() );
	}

	// Log action
	CBase_Admin::LogAction(
		pPlayer,
		NULL,
		"sent message in admin chat:",
		messageText.Get()
	);

	// Send the message to all admins with at least "b" flag
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *pLoopPlayer = UTIL_PlayerByIndex( i );
		if ( pLoopPlayer && CBase_Admin::IsPlayerAdmin( pLoopPlayer, "b" ) )
		{
			ClientPrint( pLoopPlayer, HUD_PRINTTALK, formattedMessage.Get() );
		}
	}

	if ( isServerConsole )
	{
		Msg( "(Admin Chat) Console: %s\n", messageText.Get() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Private messages
//-----------------------------------------------------------------------------
static void AdminPSay( const CCommand &args )
{
	CBasePlayer *pSender = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource( pSender );
	bool isServerConsole = ( replySource == ADMIN_REPLY_SERVER_CONSOLE );

	if ( !pSender && !isServerConsole )
	{
		Msg( "Command must be issued by a player or the server console.\n" );
		return;
	}

	if ( args.ArgC() < 4 )
	{
		AdminReply( replySource, pSender, "Usage: sa psay <name|#userID> <message>" );
		return;
	}

	const char *partialName = args.Arg( 2 );

	// Prepare target variables
	CUtlVector<CBasePlayer *> targetPlayers;
	CBasePlayer *pTarget = NULL;

	if ( !ParsePlayerTargets( pSender, replySource, partialName, targetPlayers, pTarget, false ) )
		return;

	// Assemble the message from the remaining arguments
	CUtlString messageText;
	for ( int i = 3; i < args.ArgC(); ++i )
	{
		messageText.Append( args[ i ] );
		if ( i < args.ArgC() - 1 )
		{
			messageText.Append( " " );
		}
	}

	CUtlString formattedMessage;
	if ( isServerConsole )
	{
		formattedMessage = UTIL_VarArgs( "\x04[PRIVATE] Console: \x01%s\n", messageText.Get() );
	}
	else
	{
		formattedMessage = UTIL_VarArgs( "\x04[PRIVATE] %s: \x01%s\n", pSender->GetPlayerName(), messageText.Get() );
	}

	// Send the private message to the target
	ClientPrint( pTarget, HUD_PRINTTALK, formattedMessage.Get() );

	// Show the sender the message they sent (unless the console sent it)
	if ( pTarget != pSender && !isServerConsole )
	{
		ClientPrint( pSender, HUD_PRINTTALK,
			UTIL_VarArgs( "\x04[PRIVATE] To %s: \x01%s\n", pTarget->GetPlayerName(), messageText.Get() ) );
	}
	else if ( isServerConsole )
	{
		Msg( "Private message sent to %s: %s\n", pTarget->GetPlayerName(), messageText.Get() );
	}

	// Log the action
	CBase_Admin::LogAction(
		pSender,
		pTarget,
		"sent private message to",
		messageText.Get()
	);
}

//-----------------------------------------------------------------------------
// Purpose: Reloads the admins list
//-----------------------------------------------------------------------------
static void ReloadAdminsCommand( const CCommand &args )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource( pPlayer );
	bool isServerConsole = ( replySource == ADMIN_REPLY_SERVER_CONSOLE );

	if ( !pPlayer && !isServerConsole )
	{
		Msg( "Command must be issued by a player or the server console.\n" );
		return;
	}

	CUtlMap<CUtlString, AdminData_t> newAdminMap( DefLessFunc( CUtlString ) );

	if ( !CBase_Admin::ParseAdminFile( "cfg/admin/admins.txt", newAdminMap ) )
	{
		AdminReply( replySource, pPlayer, "Failed to reload admins: Could not parse admins.txt." );
		return;
	}

	// Clear out the entire live list and start fresh
	CBase_Admin::ClearAllAdmins();

	// Rebuild g_AdminList and g_AdminMap from the parsed file
	BaseAdmin()->GetAdminMap().RemoveAll();

	for ( int i = newAdminMap.FirstInorder(); i != newAdminMap.InvalidIndex(); i = newAdminMap.NextInorder( i ) )
	{
		const CUtlString &steamID = newAdminMap.Key( i );
		AdminData_t &adminData = newAdminMap.Element( i );

		CBase_Admin *newAdmin = new CBase_Admin();
		newAdmin->Initialize( steamID.Get(), adminData.flags.Get() );
		g_AdminList.AddToTail( newAdmin );

		BaseAdmin()->GetAdminMap().Insert( steamID, adminData );  // Sync new map to global map
	}

	for ( int i = 0; i < g_AdminList.Count(); ++i )
	{
		CBase_Admin *pAdmin = g_AdminList[ i ];
		CBase_Admin::LogAction( pPlayer, NULL, "loaded admin", pAdmin->GetSteamID() );
	}

	CBase_Admin::SaveAdminCache();

	AdminReply( replySource, pPlayer, "Admins list has been reloaded." );
}

//-----------------------------------------------------------------------------
// Purpose: Help commands
//-----------------------------------------------------------------------------
static void VersionCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	if (!pPlayer && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	const char* versionString = UTIL_VarArgs("Server Admin version %s.%d", VERSION, GetBuildNumber());

	if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("===== SERVER ADMIN VERSION INFO =====\n");
		Msg("%s\n", versionString);
	}
	else
	{
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "===== SERVER ADMIN VERSION INFO =====\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, versionString);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Help instructions for the admin interface
//-----------------------------------------------------------------------------
static void HelpPlayerCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	bool isServerConsole = !pPlayer && UTIL_IsCommandIssuedByServerAdmin();

	if (!pPlayer && !isServerConsole)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (isServerConsole)
	{
		Msg("===== SERVER ADMIN USAGE =====\n"
			"\n"
			"The \"sa\" command provides access to administrative functions for managing the server.\n"
			"\n"
			"To view the list of available commands, type \"sa\" into the console. Different commands are available depending on whether you are using the server console or the client console.\n"
			"\n"
			"Please note that all commands used in the console must start with \"sa\", while commands used in the chat do not.\n"
			"\n"
			"You can target players using either part of their name or their UserID. To find this information, type \"status\" into the console to see a list of connected players and their UserIDs. Example:\n"
			"  sa ban #2 0\n"
			"The number sign (#) is required when targeting players by UserID.\n"
			"\n"
			"Most admin commands do not require quotes around player names. However, if the player's name contains spaces or if you want to match it exactly, quotes can be used. Examples:\n"
			"  sa ban Pet 0 Reason here\n"
			"  sa ban \"Peter Brev\" 0 Reason here\n"
			"  sa ban #2 0 Reason here\n"
			"\n");
		Msg("Note: Special group targets always take priority. For example, \"sa ban @all 0\" will ban all players, even if a player is named \"@all\". To target such players, use their UserID instead.\n"
			"\n"
			"You can also type a command with no arguments to see its proper usage. For example:\n"
			"  sa ban\n"
			"will print:\n"
			"  Usage: sa ban <name|#userid> <time> [reason]\n"
			"Angle brackets (< >) indicate required arguments, while square brackets ([ ]) indicate optional ones.\n"
			"\n"
			"Special group targets available:\n"
			"  @all     - All players\n"
			"  @me      - Yourself\n"
#ifdef HL2MP
			"  @blue    - Combine team\n"
			"  @red     - Rebels team\n"
#elif TF_DLL
			"  @red     - RED team\n"
			"  @blue    - BLU team\n"
#endif
			"  @!me     - Everyone except yourself\n"
			"  @alive   - All alive players\n"
			"  @dead    - All dead players\n"
			"  @bots    - All bots\n"
			"  @humans  - All human players\n"
			"\n"
			"Reply Behavior:\n"
			"If you type commands directly into the console, responses will appear in the console.\n"
			"If you type commands in chat (using ! or /), responses will appear in the chat box visible to you.\n"
			"\n"
		);

	}
	else
	{
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "===== SERVER ADMIN USAGE =====\n\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "The \"sa\" command provides access to administrative functions for managing the server.\n\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "To view the list of available commands, type \"sa\" into the console.\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "The list of commands available to you depends on whether you're using the server console or the client console.\n\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Please note that all commands used in the console must start with \"sa\", while commands used in the chat do not.\n\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "You can target players by name or UserID. Use \"status\" to list players and their UserIDs.\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Example:\n  sa ban #2 0 Reason here\nThe # is required when using UserIDs.\n\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Most commands don't require quotes around names, but they are allowed if the name has spaces or for exact matching.\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Examples:\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  sa ban Pet 0 Reason here\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  sa ban \"Peter Brev\" 0 Reason here\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  sa ban #2 0 Reason here\n\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Note: Special group targets always have priority.\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  Example: sa ban @all 0\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "This will ban all players, even if one player is named \"@all\". Use their UserID to target them directly.\n\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Type a command with no arguments to see its correct syntax. Example:\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  sa ban\nThis prints:\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  Usage: sa ban <name|#userid> <time> [reason]\n\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Special group targets:\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @all     - All players\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @me      - Yourself\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @blue    - Combine team\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @red     - Rebels team\n");
#ifdef HL2MP
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @blue    - Combine team\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @red     - Rebels team\n");
#elif TF_DLL
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @red     - RED team\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @blue    - BLU team\n");
#endif
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @!me     - Everyone except you\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @alive   - All alive players\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @dead    - All dead players\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @bots    - All bots\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  @humans  - All human players\n\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Reply Behavior:\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  Commands typed in the console will reply in the console.\n");
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "  Commands typed in chat (with ! or /) will reply in the chat box.\n\n");

	}
	return;
}

//-----------------------------------------------------------------------------
// Purpose: Toggle noclip
//-----------------------------------------------------------------------------
static void ToggleNoClipForPlayer(CBasePlayer* pTarget)
{
	if (pTarget->GetMoveType() == MOVETYPE_NOCLIP)
	{
		pTarget->SetMoveType(MOVETYPE_WALK);
	}
	else
	{
		pTarget->SetMoveType(MOVETYPE_NOCLIP);
	}
}

static void NoClipPlayerCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa noclip <name|#userID>");
		return;
	}

	const char* partialName = args.Arg(2);
	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget, true))
		return;

	if (pTarget)
	{
		ToggleNoClipForPlayer(pTarget);

		CBase_Admin::LogAction(pAdmin, pTarget, "toggled noclip for", "");

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Console toggled noclip for player %s.\n", pTarget->GetPlayerName());
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
				"Console toggled noclip for %s\n",
				pTarget->GetPlayerName()
			));
		}
		else
		{
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
				"Admin %s toggled noclip for %s\n",
				pAdmin ? pAdmin->GetPlayerName() : "Console", pTarget->GetPlayerName()
			));
		}

		return;
	}

	for (int i = 0; i < targetPlayers.Count(); i++)
	{
		if (targetPlayers[i]->IsAlive())
		{
			ToggleNoClipForPlayer(targetPlayers[i]);
		}
	}

	CUtlString logDetails, chatMessage;
	BuildGroupTargetMessage(partialName, pAdmin, "toggled noclip for", NULL, logDetails, chatMessage, false, NULL);

	CBase_Admin::LogAction(pAdmin, NULL, "toggled noclip for", logDetails.Get());

	UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.\n", chatMessage.Get()));

	if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Toggled noclip for %d player%s\n", targetPlayers.Count(), targetPlayers.Count() == 1 ? "" : "s");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Teleport to a player
//-----------------------------------------------------------------------------
static void GotoPlayerCommand(const CCommand& args)
{
	CBasePlayer* pAdmin = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pAdmin);

	if (!pAdmin)
	{
		AdminReply(replySource, pAdmin, "Command must be issued by a player.");
		return;
	}

	if (args.ArgC() < 3)
	{
		AdminReply(replySource, pAdmin, "Usage: sa goto <name|#userID>");
		return;
	}

	const char* targetPlayerInput = args.Arg(2);
	CBasePlayer* pTarget = NULL;

	if (targetPlayerInput[0] == '#')
	{
		int userID = atoi(&targetPlayerInput[1]);
		if (userID <= 0)
		{
			AdminReply(replySource, pAdmin, "Invalid UserID provided.");
			return;
		}

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pLoopPlayer = UTIL_PlayerByIndex(i);
			if (pLoopPlayer && pLoopPlayer->GetUserID() == userID)
			{
				pTarget = pLoopPlayer;
				break;
			}
		}

		if (!pTarget)
		{
			AdminReply(replySource, pAdmin, "No player found with that UserID.");
			return;
		}
	}
	else
	{
		CUtlVector<CBasePlayer*> matchingPlayers;

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pLoopPlayer = UTIL_PlayerByIndex(i);
			if (pLoopPlayer && Q_stristr(pLoopPlayer->GetPlayerName(), targetPlayerInput))
			{
				matchingPlayers.AddToTail(pLoopPlayer);
			}
		}

		if (matchingPlayers.Count() == 0)
		{
			AdminReply(replySource, pAdmin, "No players found matching that name.");
			return;
		}
		else if (matchingPlayers.Count() > 1)
		{
			AdminReply(replySource, pAdmin, "Multiple players match that partial name:");
			for (int i = 0; i < matchingPlayers.Count(); i++)
			{
				AdminReply(replySource, pAdmin, "%s", matchingPlayers[i]->GetPlayerName());
			}
			return;
		}

		pTarget = matchingPlayers[0];
	}

	if (!pTarget)
	{
		AdminReply(replySource, pAdmin, "Player not found.");
		return;
	}

	if (!pTarget->IsAlive())
	{
		AdminReply(replySource, pAdmin, "This player is currently dead.");
		return;
	}

	if (pAdmin->IsAlive())
	{
		Vector targetPosition = pTarget->GetAbsOrigin();
		targetPosition.z += 80.0f;  // Slightly above player to avoid getting stuck

		pAdmin->SetAbsOrigin(targetPosition);

		CBase_Admin::LogAction(pAdmin, pTarget, "teleported to", "");

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
			"Admin %s teleported to %s\n",
			pAdmin->GetPlayerName(), pTarget->GetPlayerName()
		));
	}
	else
	{
		AdminReply(replySource, pAdmin, "You must be alive to teleport to a player.");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Teleport players to where an admin is aiming
//-----------------------------------------------------------------------------
static void BringPlayerCommand(const CCommand& args)
{
	CBasePlayer* pAdmin = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pAdmin);

	if (!pAdmin)
	{
		AdminReply(replySource, pAdmin, "Command must be issued by a player.");
		return;
	}

	if (args.ArgC() < 3)
	{
		AdminReply(replySource, pAdmin, "Usage: sa bring <name|#userID>");
		return;
	}

	const char* targetPlayerInput = args.Arg(2);
	CBasePlayer* pTarget = NULL;

	if (Q_stricmp(targetPlayerInput, "@me") == 0)
	{
		pTarget = pAdmin;  // Admin can bring themselves (useful for debugging)
	}
	else if (targetPlayerInput[0] == '#')
	{
		int userID = atoi(&targetPlayerInput[1]);
		if (userID <= 0)
		{
			AdminReply(replySource, pAdmin, "Invalid UserID provided.");
			return;
		}

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pLoopPlayer = UTIL_PlayerByIndex(i);
			if (pLoopPlayer && pLoopPlayer->GetUserID() == userID)
			{
				pTarget = pLoopPlayer;
				break;
			}
		}

		if (!pTarget)
		{
			AdminReply(replySource, pAdmin, "No player found with that UserID.");
			return;
		}
	}
	else
	{
		CUtlVector<CBasePlayer*> matchingPlayers;

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pLoopPlayer = UTIL_PlayerByIndex(i);
			if (pLoopPlayer && Q_stristr(pLoopPlayer->GetPlayerName(), targetPlayerInput))
			{
				matchingPlayers.AddToTail(pLoopPlayer);
			}
		}

		if (matchingPlayers.Count() == 0)
		{
			AdminReply(replySource, pAdmin, "No players found matching that name.");
			return;
		}
		else if (matchingPlayers.Count() > 1)
		{
			AdminReply(replySource, pAdmin, "Multiple players match that partial name:");
			for (int i = 0; i < matchingPlayers.Count(); i++)
			{
				AdminReply(replySource, pAdmin, matchingPlayers[i]->GetPlayerName());
			}
			return;
		}

		pTarget = matchingPlayers[0];
	}

	if (pTarget)
	{
		if (pAdmin->IsAlive() && pAdmin->GetTeamNumber() != TEAM_SPECTATOR)
		{
			Vector forward;
			trace_t tr;

			pAdmin->EyeVectors(&forward);
			UTIL_TraceLine(pAdmin->EyePosition(), pAdmin->EyePosition() + forward * MAX_COORD_RANGE, MASK_SOLID, pAdmin, COLLISION_GROUP_NONE, &tr);

			Vector targetPosition = tr.endpos;
			pTarget->SetAbsOrigin(targetPosition);

			CBase_Admin::LogAction(pAdmin, pTarget, "teleported", "");

			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
				"Admin %s teleported player %s\n",
				pAdmin->GetPlayerName(), pTarget->GetPlayerName()
			));
		}
		else
		{
			AdminReply(replySource, pAdmin, "You must be alive to teleport a player.");
		}
	}
	else
	{
		AdminReply(replySource, pAdmin, "Player not found.");
	}
}

#ifdef HL2MP
#define LAST_VALID_GAME_TEAM TEAM_REBELS
#elif TF_DLL
#define LAST_VALID_GAME_TEAM TF_TEAM_BLUE
#else
#define LAST_VALID_GAME_TEAM LAST_SHARED_TEAM
#endif

//-----------------------------------------------------------------------------
// Purpose: Change a player's team
//-----------------------------------------------------------------------------
static void TeamPlayerCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa team <name|#userID> <team index>");
		return;
	}

	const char* partialName = args.Arg(2);
	int teamIndex = atoi(args.Arg(3));

	if (teamIndex < TEAM_SPECTATOR || teamIndex > LAST_VALID_GAME_TEAM)
	{
		AdminReply(replySource, pAdmin, "Invalid team index. Team index must be between 1 and 3.");
		return;
	}

	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget))
		return;

	const char* teamName = "Unassigned"; // Default (index == 0)
	if (teamIndex == TEAM_SPECTATOR)
	{
		teamName = "Spectator";
	}
#ifdef HL2MP
	else if (HL2MPRules()->IsTeamplay())
	{
		if (teamIndex == TEAM_COMBINE)
			teamName = "Combine";
		else if (teamIndex == TEAM_REBELS)
			teamName = "Rebels";
	}
#elif TF_DLL
	else
	{
		if (teamIndex == TF_TEAM_RED)
			teamName = "RED";
		else if (teamIndex == TF_TEAM_BLUE)
			teamName = "BLU";
	}
#endif

	auto MovePlayerToTeam = [teamIndex](CBasePlayer* pPlayer)
		{
			pPlayer->ChangeTeam(teamIndex);
			pPlayer->Spawn();
		};

	CUtlString logDetails, chatMessage;

	if (pTarget)
	{
		if ((pTarget->GetTeamNumber() == teamIndex) || ((teamIndex == 2 || teamIndex == 3) && pTarget->GetTeamNumber() == TEAM_UNASSIGNED))
		{
			AdminReply(replySource, pAdmin, "Player %s is already on team %s.", pTarget->GetPlayerName(), teamName);
			return;
		}

		MovePlayerToTeam(pTarget);

		CBase_Admin::LogAction(pAdmin, pTarget, "moved", UTIL_VarArgs("to team %s", teamName));

		CUtlString teamMessage;

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			teamMessage.Format("Console moved player %s to team %s.", pTarget->GetPlayerName(), teamName);
		}
		else
		{
			teamMessage.Format("Admin %s moved player %s to team %s.", pAdmin->GetPlayerName(), pTarget->GetPlayerName(), teamName);
		}

		UTIL_ClientPrintAll(HUD_PRINTTALK, teamMessage.Get());
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
				// Skip players already on the desired team
				if ((pPlayer->GetTeamNumber() == teamIndex) || ((teamIndex == 2 || teamIndex == 3) && pPlayer->GetTeamNumber() == TEAM_UNASSIGNED))
				{
					continue;
				}

				MovePlayerToTeam(pPlayer);
				movedPlayersCount++;
			}
		}

		if (movedPlayersCount == 0)
		{
			AdminReply(replySource, pAdmin, "All selected players are already on team %s.", teamName);
			return;
		}

		BuildGroupTargetMessage(partialName, pAdmin, "moved", NULL, logDetails, chatMessage, false);

		CBase_Admin::LogAction(pAdmin, NULL, "moved", UTIL_VarArgs("%s to team %s", logDetails.Get(), teamName), partialName + 1);

		chatMessage.Append(UTIL_VarArgs(" to team %s", teamName));

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.", chatMessage.Get()));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Unmute a player
//-----------------------------------------------------------------------------
static void UnMutePlayerCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa unmute <name|#userID>");
		return;
	}

	const char* partialName = args.Arg(2);
	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget))
		return;

	CUtlString logDetails, chatMessage;

	auto ExecuteUnmute = [](CBasePlayer* pTarget)
		{
			if (pTarget->IsMuted())
			{
				pTarget->SetMuted(false);
			}
		};

	if (pTarget)
	{
		if (!pTarget->IsMuted())
		{
			AdminReply(replySource, pAdmin, "Player is not muted.");
			return;
		}

		ExecuteUnmute(pTarget);

		CBase_Admin::LogAction(pAdmin, pTarget, "unmuted", "");

		PrintActionMessage(pAdmin, replySource == ADMIN_REPLY_SERVER_CONSOLE, "unmuted", pTarget->GetPlayerName(), NULL, NULL);

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Unmuted player %s\n", pTarget->GetPlayerName());
		}
	}
	else
	{
		int unmutedCount = 0;

		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			if (targetPlayers[i]->IsMuted())
			{
				ExecuteUnmute(targetPlayers[i]);
				unmutedCount++;
			}
		}

		BuildGroupTargetMessage(partialName, pAdmin, "unmuted", NULL, logDetails, chatMessage, false);

		CBase_Admin::LogAction(pAdmin, NULL, "unmuted", logDetails.Get(), partialName + 1);

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.", chatMessage.Get()));

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Unmuted %d player%s\n", unmutedCount, unmutedCount == 1 ? "" : "s");
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Mute player
//-----------------------------------------------------------------------------
static void MutePlayerCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa mute <name|#userID>");
		return;
	}

	const char* partialName = args.Arg(2);
	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget))
	{
		return;
	}

	CUtlString logDetails, chatMessage;

	auto ExecuteMute = [](CBasePlayer* pTarget)
		{
			if (!pTarget->IsMuted())
			{
				pTarget->SetMuted(true);
			}
		};

	if (pTarget)
	{
		if (pTarget->IsMuted())
		{
			AdminReply(replySource, pAdmin, "Player is already muted.");
			return;
		}

		ExecuteMute(pTarget);

		CBase_Admin::LogAction(pAdmin, pTarget, "muted", "");

		PrintActionMessage(pAdmin, replySource == ADMIN_REPLY_SERVER_CONSOLE, "muted", pTarget->GetPlayerName(), NULL, NULL);

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Muted player %s\n", pTarget->GetPlayerName());
		}
	}
	else
	{
		int mutedCount = 0;

		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			if (!targetPlayers[i]->IsMuted())
			{
				ExecuteMute(targetPlayers[i]);
				mutedCount++;
			}
		}

		BuildGroupTargetMessage(partialName, pAdmin, "muted", NULL, logDetails, chatMessage, false);

		CBase_Admin::LogAction(pAdmin, NULL, "muted", logDetails.Get(), partialName + 1);

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.", chatMessage.Get()));

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Muted %d player%s\n", mutedCount, mutedCount == 1 ? "" : "s");
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Ungag player
//-----------------------------------------------------------------------------
static void UnGagPlayerCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa ungag <name|#userID>");
		return;
	}

	const char* partialName = args.Arg(2);
	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget))
		return;

	CUtlString logDetails, chatMessage;

	auto ExecuteUnGag = [](CBasePlayer* pTarget) {
		if (pTarget->IsGagged())
		{
			pTarget->SetGagged(false);
		}
		};

	if (pTarget)
	{
		if (!pTarget->IsGagged())
		{
			AdminReply(replySource, pAdmin, "Player is not gagged.");
			return;
		}

		ExecuteUnGag(pTarget);

		CBase_Admin::LogAction(pAdmin, pTarget, "ungagged", "");

		PrintActionMessage(pAdmin, replySource == ADMIN_REPLY_SERVER_CONSOLE, "ungagged", pTarget->GetPlayerName(), NULL, NULL);

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Ungagged player %s\n", pTarget->GetPlayerName());
		}
	}
	else
	{
		int ungaggedCount = 0;
		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			if (targetPlayers[i]->IsGagged())
			{
				ExecuteUnGag(targetPlayers[i]);
				ungaggedCount++;
			}
		}

		BuildGroupTargetMessage(partialName, pAdmin, "ungagged", NULL, logDetails, chatMessage, false);

		CBase_Admin::LogAction(pAdmin, NULL, "ungagged", logDetails.Get(), partialName + 1);

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.", chatMessage.Get()));

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Ungagged %d player%s\n", ungaggedCount, ungaggedCount == 1 ? "" : "s");
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Gag player
//-----------------------------------------------------------------------------
static void GagPlayerCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa gag <name|#userID>");
		return;
	}

	const char* partialName = args.Arg(2);
	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget))
		return;

	CUtlString logDetails, chatMessage;

	auto ExecuteGag = [](CBasePlayer* pTarget)
		{
			if (!pTarget->IsGagged())
			{
				pTarget->SetGagged(true);
			}
		};

	if (pTarget)
	{
		if (pTarget->IsGagged())
		{
			AdminReply(replySource, pAdmin, "Player is already gagged.");
			return;
		}

		ExecuteGag(pTarget);

		CBase_Admin::LogAction(pAdmin, pTarget, "gagged", "");

		PrintActionMessage(pAdmin, replySource == ADMIN_REPLY_SERVER_CONSOLE, "gagged", pTarget->GetPlayerName(), NULL, NULL);

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Gagged player %s\n", pTarget->GetPlayerName());
		}
	}
	else
	{
		int gaggedCount = 0;
		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			if (!targetPlayers[i]->IsGagged())
			{
				ExecuteGag(targetPlayers[i]);
				gaggedCount++;
			}
		}

		BuildGroupTargetMessage(partialName, pAdmin, "gagged", NULL, logDetails, chatMessage, false);

		CBase_Admin::LogAction(pAdmin, NULL, "gagged", logDetails.Get(), partialName + 1);

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.", chatMessage.Get()));

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Gagged %d player%s\n", gaggedCount, gaggedCount == 1 ? "" : "s");
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Check for any human players
//-----------------------------------------------------------------------------
static bool ArePlayersInGame()
{
	// mainly for the change level admin command
	// if no player connected to the server,
	// the time remains frozen, therefore the level
	// never changes; this makes it so that if no
	// players are connected, it changes the level
	// immediately rather than after 5 seconds
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer && pPlayer->IsConnected() && !pPlayer->IsBot())
		{
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Execute files
//-----------------------------------------------------------------------------
static void ExecFileCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa exec <filename>");
		return;
	}

	const char* inputFilename = args.Arg(2);

	// Make the .cfg extension optional but if server ops add it, 
	// then take it into account to avoid a double extension
	char filename[MAX_PATH];
	if (!Q_stristr(inputFilename, ".cfg"))
	{
		Q_snprintf(filename, sizeof(filename), "%s.cfg", inputFilename);
	}
	else
	{
		Q_strncpy(filename, inputFilename, sizeof(filename));
	}

	char fullPath[MAX_PATH];
	Q_snprintf(fullPath, sizeof(fullPath), "cfg/%s", filename);

	if (!filesystem->FileExists(fullPath))
	{
		AdminReply(replySource, pAdmin, UTIL_VarArgs("Config file '%s' not found in cfg folder.", filename));
		return;
	}

	engine->ServerCommand(UTIL_VarArgs("exec %s\n", filename));

	CBase_Admin::LogAction(
		pAdmin,
		NULL,
		"executed config file",
		filename
	);

	CUtlString message;
	if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Executing config file: %s\n", filename);
	}
	else
	{
		message.Format("Admin %s executed config file: %s",
			pAdmin ? pAdmin->GetPlayerName() : "Console", filename);

		UTIL_ClientPrintAll(HUD_PRINTTALK, message.Get());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Change map
//-----------------------------------------------------------------------------
static void MapCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);
	bool isServerConsole = (replySource == ADMIN_REPLY_SERVER_CONSOLE);

	if (!pPlayer && !isServerConsole)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (args.ArgC() < 3)
	{
		AdminReply(replySource, pPlayer, "Usage: sa map <mapname>");
		return;
	}

	if (g_pGameRules->IsMapChangeOnGoing()) // Stops admins from spamming map changes
	{
		AdminReply(replySource, pPlayer, "A map change is already in progress...");
		return;
	}

	const char* partialMapName = args.Arg(2);
	CUtlVector<char*> matchingMaps;
	char* exactMatchMap = NULL;

	// Find maps
	FileFindHandle_t fileHandle;
	const char* mapPath = filesystem->FindFirst("maps/*.bsp", &fileHandle);

	while (mapPath)
	{
		char mapName[256];
		V_FileBase(mapPath, mapName, sizeof(mapName));

		if (Q_stricmp(mapName, partialMapName) == 0)
		{
			exactMatchMap = strdup(mapName);
			break;
		}

		if (Q_stristr(mapName, partialMapName))
		{
			matchingMaps.AddToTail(strdup(mapName));
		}

		mapPath = filesystem->FindNext(fileHandle);
	}
	filesystem->FindClose(fileHandle);

	if (exactMatchMap)
	{
		if (!ArePlayersInGame())
		{
			engine->ServerCommand(UTIL_VarArgs("changelevel %s\n", exactMatchMap));
		}
		else
		{
			g_pGameRules->SetScheduledMapName(exactMatchMap);
			g_pGameRules->SetMapChange(true);
			g_pGameRules->SetMapChangeOnGoing(true);
			bAdminMapChange = true;

			const char* adminName = isServerConsole ? "Console" : pPlayer->GetPlayerName();
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s is changing the map to %s in 5 seconds...\n", adminName, exactMatchMap));

			engine->ServerCommand("mp_timelimit 0\n");
			CBase_Admin::LogAction(pPlayer, NULL, "changed map", UTIL_VarArgs("to %s", exactMatchMap));
		}

		free(exactMatchMap);
		return;
	}

	if (matchingMaps.Count() == 0)
	{
		AdminReply(replySource, pPlayer, UTIL_VarArgs("No maps found matching \"%s\".", partialMapName));
		return;
	}

	if (matchingMaps.Count() == 1)
	{
		const char* selectedMap = matchingMaps[0];

		if (!ArePlayersInGame())
		{
			engine->ServerCommand(UTIL_VarArgs("changelevel %s\n", selectedMap));
		}
		else
		{
			g_pGameRules->SetScheduledMapName(selectedMap);
			g_pGameRules->SetMapChange(true);
			g_pGameRules->SetMapChangeOnGoing(true);
			bAdminMapChange = true;

			const char* adminName = isServerConsole ? "Console" : pPlayer->GetPlayerName();
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s is changing the map to %s in 5 seconds...\n", adminName, selectedMap));

			CBase_Admin::LogAction(pPlayer, NULL, "changed map", UTIL_VarArgs("to %s", selectedMap));
		}
	}
	else
	{
		AdminReply(replySource, pPlayer, "Multiple maps match the partial name:");
		for (int i = 0; i < matchingMaps.Count(); i++)
		{
			AdminReply(replySource, pPlayer, "%s", matchingMaps[i]);
		}
	}

	for (int i = 0; i < matchingMaps.Count(); i++)
	{
		free(matchingMaps[i]);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Rcon
//-----------------------------------------------------------------------------
static void RconCommand(const CCommand& args)
{
	// For rcon, we are only making this command available to players in-game
	// (meaning it will not do anything if used within the server console directly)
	// because it makes no sense to use rcon in the server console, but we are also
	// disabling the usage of all "sa" commands with "sa rcon" since if the admin has rcon,
	// they probably have root privileges already.

	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	bool isServerConsole = (replySource == ADMIN_REPLY_SERVER_CONSOLE);

	if (!pPlayer && !isServerConsole)
	{
		Msg("Command must be issued by a player\n");
		return;
	}

	if (isServerConsole)
	{
		Msg("Command must be issued by a player\n");
		return;
	}

	const char* usage = "sa rcon <command> [arguments]";
	if (args.ArgC() < 3)
	{
		AdminReply(replySource, pPlayer, "Usage: %s", usage);
		return;
	}

	const char* commandName = args.Arg(2);

	if (Q_stricmp(commandName, "sa") == 0)
	{
		AdminReply(replySource, pPlayer, "No \"sa rcon\" needed with commands starting with \"%s\"", commandName);
		return;
	}

	ConVar* pConVar = g_pCVar->FindVar(commandName);

	if (pConVar && args.ArgC() == 3)
	{
		if (!(pConVar->IsFlagSet(FCVAR_NEVER_AS_STRING)))  // String-type cvars
		{
			AdminReply(replySource, pPlayer, "Cvar %s is set to \"%s\"",
				commandName, pConVar->GetString());
		}
		else  // Numeric cvars
		{
			float value = pConVar->GetFloat();
			if (fabs(value - roundf(value)) < 0.0001f)
			{
				AdminReply(replySource, pPlayer, "Cvar %s is set to %d",
					commandName, static_cast<int>(value));
			}
			else
			{
				AdminReply(replySource, pPlayer, "Cvar %s is set to %f",
					commandName, value);
			}
		}
		return;
	}

	// Assemble command (handles `sv_tags`, etc.)
	CUtlString rconCommand;
	for (int i = 2; i < args.ArgC(); i++)
	{
		rconCommand.Append(args.Arg(i));
		if (i < args.ArgC() - 1)
		{
			rconCommand.Append(" ");
		}
	}

	engine->ServerCommand(UTIL_VarArgs("%s\n", rconCommand.Get()));

	CBase_Admin::LogAction(pPlayer, NULL, "executed rcon", rconCommand.Get());

	AdminReply(replySource, pPlayer, "Rcon command issued: %s", rconCommand.Get());
}

//-----------------------------------------------------------------------------
// Purpose: Cvar
//-----------------------------------------------------------------------------
static void CVarCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	bool isServerConsole = (replySource == ADMIN_REPLY_SERVER_CONSOLE);

	if (!pPlayer && !isServerConsole)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	const char* usage = "Usage: sa cvar <cvarname> [newvalue]";
	if (args.ArgC() < 3)
	{
		AdminReply(replySource, pPlayer, "%s", usage);
		return;
	}

	const char* cvarName = args.Arg(2);
	ConVar* pConVar = cvar->FindVar(cvarName);

	if (!pConVar)
	{
		AdminReply(replySource, pPlayer, "Cvar %s not found.", cvarName);
		return;
	}

	bool requiresCheatFlag = pConVar->IsFlagSet(FCVAR_CHEAT);
	if (requiresCheatFlag && pPlayer && !CBase_Admin::IsPlayerAdmin(pPlayer, "n"))
	{
		AdminReply(replySource, pPlayer, "You do not have permission to change cheat-protected cvars.");
		return;
	}

	// Show the admin the value if they're not changing it
	if (args.ArgC() == 3)
	{
		if (pConVar->IsFlagSet(FCVAR_PROTECTED) || Q_stricmp(cvarName, "sv_password") == 0)
		{
			AdminReply(replySource, pPlayer, "Value is protected and cannot be displayed.");
			return;
		}

		if (!(pConVar->IsFlagSet(FCVAR_NEVER_AS_STRING)))  // String-type cvar
		{
			AdminReply(replySource, pPlayer, "Cvar %s is currently set to \"%s\"",
				cvarName, pConVar->GetString());
		}
		else  // Numeric value
		{
			float value = pConVar->GetFloat();
			if (fabs(value - roundf(value)) < 0.0001f)
			{
				AdminReply(replySource, pPlayer, "Cvar %s is currently set to %d",
					cvarName, static_cast<int>(value));
			}
			else
			{
				AdminReply(replySource, pPlayer, "Cvar %s is currently set to %f",
					cvarName, value);
			}
		}
		return;
	}

	// Assemble new value for cvars that allow spaces (e.g., sv_tags, hostname)
	CUtlString newValue;
	for (int i = 3; i < args.ArgC(); i++)
	{
		newValue.Append(args.Arg(i));
		if (i < args.ArgC() - 1)
		{
			newValue.Append(" ");
		}
	}

	if (Q_stricmp(newValue.Get(), "reset") == 0)
	{
		pConVar->Revert();

		const char* adminName = pPlayer ? pPlayer->GetPlayerName() : "Console";

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
			"Admin %s reset cvar %s to default value.\n",
			adminName, cvarName
		));

		CBase_Admin::LogAction(pPlayer, NULL, "reset cvar", UTIL_VarArgs("%s reset %s to default value", adminName, cvarName));
		return;
	}

	pConVar->SetValue(newValue.Get());

	CBase_Admin::LogAction(pPlayer, NULL, "changed cvar", UTIL_VarArgs(
		"%s to %s",
		cvarName,
		pConVar->IsFlagSet(FCVAR_PROTECTED) ? "***PROTECTED***" : newValue.Get()
	));

	if (pConVar->IsFlagSet(FCVAR_PROTECTED))
	{
		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
			"Admin %s changed cvar value of %s.\n",
			pPlayer ? pPlayer->GetPlayerName() : "Console",
			cvarName
		));
	}
	else
	{
		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
			"Admin %s changed cvar value %s to %s.\n",
			pPlayer ? pPlayer->GetPlayerName() : "Console",
			cvarName,
			newValue.Get()
		));
	}
}

//-----------------------------------------------------------------------------
// Purpose: SLap player (+ damage)
//-----------------------------------------------------------------------------
static void SlapPlayerCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	if (!pPlayer && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (args.ArgC() < 3)
	{
		AdminReply(replySource, pPlayer, "Usage: sa slap <name|#userid> [damage]");
		return;
	}

	const char* partialName = args.Arg(2);
	int slapDamage = (args.ArgC() >= 4) ? Q_max(atoi(args.Arg(3)), 0) : 0;

	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pPlayer, replySource, partialName, targetPlayers, pTarget, false))
		return;

	Vector slapForce(RandomFloat(-150, 150), RandomFloat(-150, 150), RandomFloat(200, 400));

	auto EmitSlapSound = [](CBasePlayer* pVictim)
		{
			int soundChoice = random->RandomInt(1, 2);
			const char* sound = (soundChoice == 1) ? "Player.FallDamage" : "Player.SonicDamage";

			CPASAttenuationFilter filter(pVictim);
			filter.AddRecipient(pVictim);
			filter.MakeReliable();

			CBaseEntity::EmitSound(filter, pVictim->entindex(), sound);
		};

	CUtlString logDetails, chatMessage;

	if (pTarget)
	{
		if (!pTarget->IsAlive())
		{
			AdminReply(replySource, pPlayer, "This player is already dead.");
			return;
		}

		pTarget->ApplyAbsVelocityImpulse(slapForce);

		if (slapDamage > 0)
		{
			CTakeDamageInfo dmg(pTarget, pTarget, slapDamage, DMG_FALL);
			pTarget->TakeDamage(dmg);

			char currentDamage[64];
			Q_snprintf(currentDamage, sizeof(currentDamage), "(Damage: %d)", slapDamage);
			logDetails.Format("%s", currentDamage);
		}

		EmitSlapSound(pTarget);

		chatMessage.Format("%s slapped %s",
			pPlayer ? pPlayer->GetPlayerName() : "Console",
			pTarget->GetPlayerName());

		if (slapDamage > 0)
		{
			chatMessage.Append(UTIL_VarArgs(" (Damage: %d)", slapDamage));
		}

		CBase_Admin::LogAction(pPlayer, pTarget, "slapped", logDetails.Get());
		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.\n", chatMessage.Get()));

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			if (slapDamage > 0)
			{
				Msg("Slapped %s (Damage: %d)\n", pTarget->GetPlayerName(), slapDamage);
			}
			else
			{
				Msg("Slapped %s\n", pTarget->GetPlayerName());
			}
		}
	}
	else
	{
		int aliveCount = 0;
		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			if (targetPlayers[i]->IsAlive())
				aliveCount++;
		}

		if (aliveCount == 0)
		{
			AdminReply(replySource, pPlayer, "All players in the target group are dead.");
			return;
		}

		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			if (targetPlayers[i]->IsAlive())
			{
				targetPlayers[i]->ApplyAbsVelocityImpulse(slapForce);
				if (slapDamage > 0)
				{
					CTakeDamageInfo dmg(targetPlayers[i], targetPlayers[i], slapDamage, DMG_FALL);
					targetPlayers[i]->TakeDamage(dmg);
				}
				EmitSlapSound(targetPlayers[i]);
			}
		}

		BuildGroupTargetMessage(partialName, pPlayer, "slapped", NULL, logDetails, chatMessage, false, NULL);

		if (slapDamage > 0)
		{
			logDetails.Append(UTIL_VarArgs(" (Damage: %d)", slapDamage));
			chatMessage.Append(UTIL_VarArgs(" (Damage: %d)", slapDamage));
		}

		CBase_Admin::LogAction(pPlayer, NULL, "slapped", logDetails.Get(), partialName + 1);
		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.\n", chatMessage.Get()));

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			if (targetPlayers.Count() > 1)
			{
				Msg("Slapped %d players.\n", aliveCount);
			}
			else if (targetPlayers.Count() == 1)
			{
				if (slapDamage > 0)
				{
					Msg("Slapped %s (Damage: %d)\n", targetPlayers[0]->GetPlayerName(), slapDamage);
				}
				else
				{
					Msg("Slapped %s\n", targetPlayers[0]->GetPlayerName());
				}
			}
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Slay player
//-----------------------------------------------------------------------------
static void SlayPlayerCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	if (!pPlayer && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (args.ArgC() < 3)
	{
		AdminReply(replySource, pPlayer, "Usage: sa slay <name|#userid> [reason]");
		return;
	}

	const char* partialName = args.Arg(2);

	CUtlString reason;
	for (int i = 3; i < args.ArgC(); ++i)
	{
		reason.Append(args.Arg(i));
		if (i < args.ArgC() - 1)
		{
			reason.Append(" ");
		}
	}
	const char* slayReason = reason.Length() > 0 ? reason.Get() : "No reason provided";

	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pPlayer, replySource, partialName, targetPlayers, pTarget, false))
	{
		return;
	}

	int aliveCount = 0;
	for (int i = 0; i < targetPlayers.Count(); i++)
	{
		if (targetPlayers[i]->IsAlive())
		{
			aliveCount++;
		}
	}

	if (pTarget && !pTarget->IsAlive())
	{
		AdminReply(replySource, pPlayer, "This player is already dead.");
		return;
	}
	else if (targetPlayers.Count() > 0 && aliveCount == 0)
	{
		AdminReply(replySource, pPlayer, "All players in the target group are dead.");
		return;
	}

	auto ExecuteSlay = [](CBasePlayer* pVictim)
		{
			if (pVictim->IsAlive())
			{
				pVictim->CommitSuicide();
			}
		};

	CUtlString logDetails, chatMessage;

	if (pTarget)
	{
		ExecuteSlay(pTarget);

		logDetails.Format("(Reason: %s)", slayReason);
		CBase_Admin::LogAction(pPlayer, pTarget, "slayed", logDetails.Get());

		PrintActionMessage(pPlayer, replySource, "slayed", pTarget->GetPlayerName(), NULL, slayReason);

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Slayed player %s (Reason: %s)\n", pTarget->GetPlayerName(), slayReason);
		}
	}
	else
	{
		int slainCount = 0;
		for (int i = 0; i < targetPlayers.Count(); ++i)
		{
			if (targetPlayers[i]->IsAlive())
			{
				ExecuteSlay(targetPlayers[i]);
				slainCount++;
			}
		}

		BuildGroupTargetMessage(partialName, pPlayer, "slayed", NULL, logDetails, chatMessage, true, slayReason);

		CBase_Admin::LogAction(pPlayer, NULL, "slayed", logDetails.Get(), partialName + 1);
		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.\n", chatMessage.Get()));

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Slayed %d player%s (Reason: %s)\n",
				slainCount, slainCount == 1 ? "" : "s", slayReason);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Kick player
//-----------------------------------------------------------------------------
static void KickPlayerCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	if (!pPlayer && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (args.ArgC() < 3)
	{
		AdminReply(replySource, pPlayer, "Usage: sa kick <name|#userid> [reason]");
		return;
	}

	const char* partialName = args.Arg(2);

	CUtlString reason;
	for (int i = 3; i < args.ArgC(); i++)
	{
		reason.Append(args.Arg(i));
		if (i < args.ArgC() - 1)
		{
			reason.Append(" ");
		}
	}
	const char* kickReason = reason.Length() > 0 ? reason.Get() : "No reason provided";

	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pPlayer, replySource, partialName, targetPlayers, pTarget, false))
	{
		return;
	}

	auto ExecuteKick = [&](CBasePlayer* pTarget)
		{
			if (reason.Length() > 0)
			{
				engine->ServerCommand(UTIL_VarArgs("kickid %d %s\n", pTarget->GetUserID(), reason.Get()));
			}
			else
			{
				engine->ServerCommand(UTIL_VarArgs("kickid %d\n", pTarget->GetUserID()));
			}
		};

	CUtlString logDetails, chatMessage;

	if (pTarget)
	{
		const char* targetSteamID = engine->GetPlayerNetworkIDString(pTarget->edict());

		// Root admin immunity check 
		// can't have random admins kick root admins
		CBase_Admin* pAdmin = CBase_Admin::GetAdmin(targetSteamID);
		if (pAdmin && pAdmin->HasPermission(ADMIN_ROOT) && (pTarget != pPlayer) && replySource != ADMIN_REPLY_SERVER_CONSOLE)
		{
			AdminReply(replySource, pPlayer, "Cannot target this player (root admin privileges).");
			return;
		}

		ExecuteKick(pTarget);

		logDetails.Format("Reason: %s", kickReason);
		CBase_Admin::LogAction(pPlayer, pTarget, "kicked", logDetails.Get());

		PrintActionMessage(pPlayer, replySource, "kicked", pTarget->GetPlayerName(), NULL, kickReason);
	}
	else
	{
		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			ExecuteKick(targetPlayers[i]);
		}

		BuildGroupTargetMessage(partialName, pPlayer, "kicked", NULL, logDetails, chatMessage, true, kickReason);

		CBase_Admin::LogAction(pPlayer, NULL, "kicked", logDetails.Get(), partialName + 1);

		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.\n", chatMessage.Get()));
	}
}

//-----------------------------------------------------------------------------
// Purpose: bans a player
//-----------------------------------------------------------------------------
bool IsStringDigitsOnly(const char* str)
{
	// We want to make sure only digits are used
	for (const char* p = str; *p; ++p)
	{
		if (!isdigit(*p))
			return false;
	}
	return true;
}

static void BanPlayerCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	if (!pPlayer && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (args.ArgC() < 4)
	{
		AdminReply(replySource, pPlayer, "Usage: sa ban <name|#userid> <time> [reason]");
		return;
	}

	const char* partialName = args.Arg(2);
	const char* timeArg = args.Arg(3);

	if (!IsStringDigitsOnly(timeArg))
	{
		AdminReply(replySource, pPlayer, "Invalid ban time provided.");
		return;
	}

	int banTime = atoi(timeArg);
	if (banTime < 0)
	{
		AdminReply(replySource, pPlayer, "Invalid ban time provided.");
		return;
	}

	CUtlString reason;
	for (int i = 4; i < args.ArgC(); i++)
	{
		reason.Append(args.Arg(i));
		if (i < args.ArgC() - 1)
		{
			reason.Append(" ");
		}
	}
	const char* kickReason = reason.Length() > 0 ? reason.Get() : "No reason provided";

	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pPlayer, replySource, partialName, targetPlayers, pTarget))
	{
		return;
	}

	// Default ban messages if none is used
	const static char defaultBanMsg[] = "You have been banned from this server";
	const static char defaultPermaBanMsg[] = "You have been permanently banned from this server";

	CUtlString banDuration = (banTime == 0) ? "permanently" : UTIL_VarArgs("%d minute%s", banTime, banTime == 1 ? "" : "s");

	auto ExecuteBan = [&](CBasePlayer* pTarget)
		{
			const char* steamID = engine->GetPlayerNetworkIDString(pTarget->edict());
			const char* finalReason = reason.Length() > 0 ? reason.Get() : (banTime == 0 ? defaultPermaBanMsg : defaultBanMsg);

			if (banTime == 0)
			{
				engine->ServerCommand(UTIL_VarArgs("banid 0 %s; kickid %d %s\n", steamID, pTarget->GetUserID(), finalReason));
				engine->ServerCommand("writeid\n");
			}
			else
			{
				engine->ServerCommand(UTIL_VarArgs("banid %d %s; kickid %d %s\n", banTime, steamID, pTarget->GetUserID(), finalReason));
			}
		};

	CUtlString logDetails, chatMessage;

	if (pTarget)
	{
		CBase_Admin* pAdmin = CBase_Admin::GetAdmin(engine->GetPlayerNetworkIDString(pTarget->edict()));
		if (pAdmin && pAdmin->HasPermission(ADMIN_ROOT) && (pTarget != pPlayer) && replySource != ADMIN_REPLY_SERVER_CONSOLE)
		{
			AdminReply(replySource, pPlayer, "Cannot target this player (root admin privileges).");
			return;
		}

		ExecuteBan(pTarget);

		logDetails.Format("%s (Reason: %s)", banDuration.Get(), kickReason);
		CBase_Admin::LogAction(pPlayer, pTarget, "banned", logDetails.Get());

		PrintActionMessage(pPlayer, replySource, "banned", pTarget->GetPlayerName(), banDuration.Get(), kickReason);
	}
	else
	{
		for (int i = 0; i < targetPlayers.Count(); i++)
		{
			ExecuteBan(targetPlayers[i]);
		}

		BuildGroupTargetMessage(partialName, pPlayer, "banned", banDuration.Get(), logDetails, chatMessage, true, kickReason);

		CBase_Admin::LogAction(pPlayer, NULL, "banned", logDetails.Get(), partialName + 1);
		UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.\n", chatMessage.Get()));
	}
}

//-----------------------------------------------------------------------------
// Purpose: adds a SteamID3 to the banned list
//-----------------------------------------------------------------------------
static void AddBanCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	if (!pPlayer && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	// We are making this Rcon only because this prevents random admins from abusing this command and 
	// simply adding a random SteamID into the ban list just because they feel like it. Despite the chat 
	// print when a SteamID is added to the ban list, they could add a ban when nobody is connected, 
	// and even then, it is not always obvious when a SteamID was banned without checking the logs, 
	// hence why it is severely restricted to one of the highest permission levels.

	if (args.ArgC() < 4)
	{
		AdminReply(replySource, pPlayer, "Usage: sa addban <time> <SteamID3> [reason]");
		return;
	}

	int banTime = atoi(args.Arg(2));
	if (banTime < 0)
	{
		AdminReply(replySource, pPlayer, "Invalid ban time provided.");
		return;
	}

	// The SteamID format requires us to break it down into multiple arguments because of colons, 
	// which is why there are many arguments below. Admittedly, we could reduce the number of 
	// arguments by placing the colons directly since they are static, but this works anyway.

	// Reassemble SteamID3 from arguments
	char steamID[64] = { 0 };
	Q_snprintf(steamID, sizeof(steamID), "%s%s%s%s%s", args.Arg(3), args.Arg(4), args.Arg(5), args.Arg(6), args.Arg(7));

	// Validate SteamID format
	if (Q_strncmp(steamID, "[U:1:", 5) != 0 || Q_strlen(steamID) < 7)
	{
		AdminReply(replySource, pPlayer, "Invalid SteamID format. SteamID must start with [U:1: and be correctly formatted.");
		return;
	}

	const char* idPart = Q_strstr(steamID, ":") + 3;
	const char* closingBracket = Q_strstr(steamID, "]");

	if (!closingBracket || idPart >= closingBracket)
	{
		AdminReply(replySource, pPlayer, "Invalid SteamID format. Missing closing bracket.");
		return;
	}

	for (const char* c = idPart; c < closingBracket; ++c)
	{
		if (!isdigit(*c))
		{
			AdminReply(replySource, pPlayer, "Invalid SteamID format. The numeric portion must contain only numbers.");
			return;
		}
	}

	if (IsSteamIDAdmin(steamID))
	{
		AdminReply(replySource, pPlayer, "This player is an admin and cannot be banned.");
		return;
	}

	// Check if banned_user.cfg exists; if not, create it
	if (!filesystem->FileExists("cfg/banned_user.cfg", "MOD"))
	{
		FileHandle_t createFile = filesystem->Open("cfg/banned_user.cfg", "w", "MOD");
		if (createFile)
		{
			filesystem->Close(createFile);
		}
		else
		{
			AdminReply(replySource, pPlayer, "Failed to create 'cfg/banned_user.cfg'. Check file permissions.");
			return;
		}
	}

	// Checking if the SteamID is already banned
	FileHandle_t file = filesystem->Open("cfg/banned_user.cfg", "r", "MOD");
	if (file)
	{
		char buffer[1024];
		while (filesystem->ReadLine(buffer, sizeof(buffer), file))
		{
			if (Q_stristr(buffer, steamID))
			{
				AdminReply(replySource, pPlayer, "SteamID is already banned.");
				filesystem->Close(file);
				return;
			}
		}
		filesystem->Close(file);
	}
	else
	{
		AdminReply(replySource, pPlayer, "Failed to read the ban list.");
		return;
	}

	// It has little purpose too. Main purpose is for logging.
	CUtlString reason;
	for (int i = 8; i < args.ArgC(); i++)
	{
		reason.Append(args.Arg(i));
		if (i < args.ArgC() - 1)
		{
			reason.Append(" ");
		}
	}

	// This too has little purpose. If we use addban, it is probably to permanently ban an ID. 
	// If we wanted to temporarily ban an ID with ban or addban, we would likely use a database 
	// of some sort, like MySQL to avoid just storing bans in memory and running into issues 
	// with it long term. Best to use 0 if this command is ever used.
	if (banTime == 0)
	{
		engine->ServerCommand(UTIL_VarArgs("banid 0 %s\n", steamID));
	}
	else
	{
		engine->ServerCommand(UTIL_VarArgs("banid %d %s\n", banTime, steamID));
	}
	engine->ServerCommand("writeid\n");

	CUtlString banDuration = (banTime == 0) ? "permanently" : UTIL_VarArgs("for %d minute%s", banTime, banTime > 1 ? "s" : "");
	CUtlString logDetails = UTIL_VarArgs("SteamID %s %s", steamID, banDuration.Get());

	if (reason.Length() > 0)
	{
		logDetails.Append(UTIL_VarArgs(" (Reason: %s)", reason.Get()));
	}

	CBase_Admin::LogAction(pPlayer, NULL, "added ban", logDetails.Get());

	CUtlString message;
	if (banTime == 0)
	{
		message.Format("SteamID %s permanently banned by %s", steamID, pPlayer ? pPlayer->GetPlayerName() : "Console");
	}
	else
	{
		message.Format("SteamID %s banned by %s for %d minute%s",
			steamID, pPlayer ? pPlayer->GetPlayerName() : "Console", banTime, banTime > 1 ? "s" : "");
	}

	if (reason.Length() > 0)
	{
		message.Append(UTIL_VarArgs(". Reason: %s", reason.Get()));
	}

	UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.\n", message.Get()));
}

//-----------------------------------------------------------------------------
// Purpose: removes a ban from the banned user list
//-----------------------------------------------------------------------------
static void UnbanPlayerCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	if (!pPlayer && replySource != ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Command must be issued by a player or the server console.\n");
		return;
	}

	if (args.ArgC() < 7)
	{
		AdminReply(replySource, pPlayer, "Usage: sa unban <SteamID3>");
		return;
	}

	// Same thing as addban, we need to reconstruct the SteamID
	char steamID[64];
	Q_snprintf(steamID, sizeof(steamID), "%s%s%s%s%s", args.Arg(2), args.Arg(3), args.Arg(4), args.Arg(5), args.Arg(6));

	if (Q_strncmp(steamID, "[U:1:", 5) != 0 || Q_strlen(steamID) < 7)
	{
		AdminReply(replySource, pPlayer, "Invalid SteamID format. SteamID must start with [U:1: and be properly formatted.");
		return;
	}

	const char* idPart = Q_strstr(steamID, ":") + 3;  // Skip "[U:1:"
	const char* closingBracket = Q_strstr(steamID, "]");

	if (!closingBracket || idPart >= closingBracket)
	{
		AdminReply(replySource, pPlayer, "Invalid SteamID format. Missing closing bracket.");
		return;
	}

	for (const char* c = idPart; c < closingBracket; ++c)
	{
		if (!isdigit(*c))
		{
			AdminReply(replySource, pPlayer, "Invalid SteamID format. The numeric portion must only contain digits.");
			return;
		}
	}

	// Check if SteamID is in the ban list
	FileHandle_t file = filesystem->Open("cfg/banned_user.cfg", "r", "MOD");
	if (!file)
	{
		AdminReply(replySource, pPlayer, "Failed to read the ban list. Check that the file exists and is placed the cfg folder.");
		return;
	}

	const int bufferSize = 1024;
	char buffer[bufferSize];
	bool steamIDFound = false;

	while (filesystem->ReadLine(buffer, bufferSize, file))
	{
		if (Q_stristr(buffer, steamID))
		{
			steamIDFound = true;
			break;
		}
	}

	filesystem->Close(file);

	if (steamIDFound)
	{
		engine->ServerCommand(UTIL_VarArgs("removeid %s\n", steamID));
		engine->ServerCommand("writeid\n");

		CUtlString logDetails;
		logDetails.Format("SteamID %s", steamID);
		CBase_Admin::LogAction(pPlayer, NULL, "unbanned", logDetails.Get());

		CUtlString message;
		message.Format("SteamID %s was unbanned by %s.", steamID, pPlayer ? pPlayer->GetPlayerName() : "Console");

		UTIL_ClientPrintAll(HUD_PRINTTALK, message.Get());
	}
	else
	{
		AdminReply(replySource, pPlayer, UTIL_VarArgs("SteamID %s was not found in the ban list.", steamID));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Toggle Buddha
//-----------------------------------------------------------------------------
static void ToggleBuddhaForPlayer(CBasePlayer* pTarget)
{
	if (pTarget->m_debugOverlays & OVERLAY_BUDDHA_MODE)
	{
		pTarget->m_debugOverlays &= ~OVERLAY_BUDDHA_MODE;
		Msg("Buddha Mode off...\n");
	}
	else
	{
		pTarget->m_debugOverlays |= OVERLAY_BUDDHA_MODE;
		Msg("Buddha Mode on...\n");
	}
}

static void BuddhaPlayerCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa buddha <name|#userID>");
		return;
	}

	const char* partialName = args.Arg(2);
	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget, true))
		return;

	if (pTarget)
	{
		ToggleBuddhaForPlayer(pTarget);

		CBase_Admin::LogAction(pAdmin, pTarget, "toggled buddha for", "");

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Console toggled buddha for player %s.\n", pTarget->GetPlayerName());
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
				"Console toggled buddha for %s\n",
				pTarget->GetPlayerName()
			));
		}
		else
		{
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
				"Admin %s toggled buddha for %s\n",
				pAdmin ? pAdmin->GetPlayerName() : "Console", pTarget->GetPlayerName()
			));
		}

		return;
	}

	for (int i = 0; i < targetPlayers.Count(); i++)
	{
		if (targetPlayers[i]->IsAlive())
		{
			ToggleBuddhaForPlayer(targetPlayers[i]);
		}
	}

	CUtlString logDetails, chatMessage;
	BuildGroupTargetMessage(partialName, pAdmin, "toggled buddha for", NULL, logDetails, chatMessage, false, NULL);

	CBase_Admin::LogAction(pAdmin, NULL, "toggled buddha for", logDetails.Get());

	UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.\n", chatMessage.Get()));

	if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Toggled buddha for %d player%s\n", targetPlayers.Count(), targetPlayers.Count() == 1 ? "" : "s");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Toggle God
//-----------------------------------------------------------------------------
static void GodPlayerCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa god <name|#userID>");
		return;
	}

	const char* partialName = args.Arg(2);
	CUtlVector<CBasePlayer*> targetPlayers;
	CBasePlayer* pTarget = NULL;

	if (!ParsePlayerTargets(pAdmin, replySource, partialName, targetPlayers, pTarget, true))
		return;

	if (pTarget)
	{
		pTarget->ToggleFlag(FL_GODMODE);

		CBase_Admin::LogAction(pAdmin, pTarget, "toggled god for", "");

		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("Console toggled god for player %s.\n", pTarget->GetPlayerName());
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
				"Console toggled god for %s\n",
				pTarget->GetPlayerName()
			));
		}
		else
		{
			UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs(
				"Admin %s toggled god for %s\n",
				pAdmin ? pAdmin->GetPlayerName() : "Console", pTarget->GetPlayerName()
			));
		}

		return;
	}

	for (int i = 0; i < targetPlayers.Count(); i++)
	{
		if (targetPlayers[i]->IsAlive())
		{
			targetPlayers[i]->ToggleFlag(FL_GODMODE);
		}
	}

	CUtlString logDetails, chatMessage;
	BuildGroupTargetMessage(partialName, pAdmin, "toggled god for", NULL, logDetails, chatMessage, false, NULL);

	CBase_Admin::LogAction(pAdmin, NULL, "toggled god for", logDetails.Get());

	UTIL_ClientPrintAll(HUD_PRINTTALK, UTIL_VarArgs("%s.\n", chatMessage.Get()));

	if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Toggled god for %d player%s\n", targetPlayers.Count(), targetPlayers.Count() == 1 ? "" : "s");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Execute VScripts
//-----------------------------------------------------------------------------
static void ExecVScriptCommand(const CCommand& args)
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
		AdminReply(replySource, pAdmin, "Usage: sa vscript <filename>");
		return;
	}

	const char* inputFilename = args.Arg(2);

	// Make the .cfg extension optional but if server ops add it, 
	// then take it into account to avoid a double extension
	char filename[MAX_PATH];
	if (!Q_stristr(inputFilename, ".nut"))
	{
		Q_snprintf(filename, sizeof(filename), "%s.nut", inputFilename);
	}
	else
	{
		Q_strncpy(filename, inputFilename, sizeof(filename));
	}

	char fullPath[MAX_PATH];
	Q_snprintf(fullPath, sizeof(fullPath), "scripts/vscripts/%s", filename);

	if (!filesystem->FileExists(fullPath))
	{
		AdminReply(replySource, pAdmin, UTIL_VarArgs("VScript file '%s' not found in scripts/vscripts folder.", filename));
	}

	engine->ServerCommand(UTIL_VarArgs("script_execute %s\n", filename));

	CBase_Admin::LogAction(
		pAdmin,
		NULL,
		"executed vscript file",
		filename
	);

	CUtlString message;
	if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
	{
		Msg("Executing vscript file: %s\n", filename);
	}
	else
	{
		message.Format("Admin %s executed vscript file: %s",
			pAdmin ? pAdmin->GetPlayerName() : "Console", filename);

		UTIL_ClientPrintAll(HUD_PRINTTALK, message.Get());
	}
}

#define BASE_COMMAND_MODULE_NAME "Base Commands"

static void LoadBaseCommandModule()
{
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "say", true, NULL, "<message> -> Sends an admin formatted message to all players in the chat", "j", AdminSay);
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "csay", true, NULL, "<message> -> Sends a centered message to all players", "j", AdminCSay );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "chat", true, NULL, "<message> -> Sends a chat message to connected admins only", "j", AdminChat );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "psay", true, NULL, "<name|#userID> <message> -> Sends a private message to a player", "j", AdminPSay );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "ban", true, NULL, "<name|#userID> <time> [reason] -> Ban a player", "d", BanPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "kick", true, NULL, "<name|#userID> [reason] -> Kick a player", "c", KickPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "addban", true, NULL, "<time> <SteamID3> [reason] -> Add a manual ban to banned_user.cfg", "m", AddBanCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "unban", true, NULL, "<SteamID3> -> Remove a banned SteamID from banned_user.cfg", "e", UnbanPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "slay", true, NULL, "<name|#userID> -> Slay a player", "f", SlayPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "slap", true, NULL, "<name|#userID> [amount] -> Slap a player with damage if defined", "f", SlapPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "gag", true, NULL, "<name|#userID> -> Gag a player", "j", GagPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "ungag", true, NULL, "<name|#userID> -> Ungag a player", "j", UnGagPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "mute", true, NULL, "<name|#userID> -> Mute a player", "j", MutePlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "unmute", true, NULL, "<name|#userID> -> Unmute a player", "j", UnMutePlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "team", true, NULL, "<name|#userID> <team index> -> Move a player to another team", "f", TeamPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "bring", true, NULL, "<name|#userID> -> Teleport a player to where an admin is aiming", "f", BringPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "goto", true, NULL, "<name|#userID> -> Teleport yourself to a player", "f", GotoPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "map", true, NULL, "<map name> -> Change the map", "g", MapCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "noclip", true, NULL, "<name|#userID> -> Toggle noclip mode for a player", "f", NoClipPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "buddha", true, NULL, "<name|#userID> -> Toggle buddha mode for a player", "f", BuddhaPlayerCommand);
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "god", true, NULL, "<name|#userID> -> Toggle god mode for a player", "f", GodPlayerCommand);
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "cvar", true, NULL, "<cvar name> [new value|reset] -> Modify or reset any cvar's value", "h", CVarCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "exec", true, NULL, "<filename> -> Executes a configuration file", "i", ExecFileCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "vscript", true, NULL, "<filename> -> Executes a vscript file", "i", ExecVScriptCommand);
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "rcon", true, NULL, "<command> [value] -> Send a command as if it was written in the server console", "m", RconCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "reloadadmins", false, NULL, "-> Refresh the admin cache", "i", ReloadAdminsCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "help", false, "Check your console for output.\n", "-> Provide instructions on how to use the admin interface", "b", HelpPlayerCommand );
	REGISTER_ADMIN_COMMAND(BASE_COMMAND_MODULE_NAME, "version", false, "Check your console for output.\n", "-> Display version", "a", VersionCommand );
}

#endif

#endif // SERVERADMIN_COMMAND_BASE_H