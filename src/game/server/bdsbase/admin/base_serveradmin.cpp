#include "cbase.h"
#include "base_serveradmin.h"
#include "filesystem.h"
#include <KeyValues.h>
#include "convar.h"
#include "tier0/icommandline.h"
#include <time.h>
#include "fmtstr.h"

//MODULES
#include "serveradmin_register.h"

// always comes last
#include "tier0/memdbgon.h"

#ifdef BDSBASE

CBase_Admin *g_pBaseAdmin = NULL;
bool g_bAdminSystem = false;

// global list of admins
CUtlVector<CBase_Admin *> g_AdminList;
FileHandle_t g_AdminLogFile = FILESYSTEM_INVALID_HANDLE;

CUtlMap<const char *, SpecialTarget> g_SpecialTargets( DefLessFunc( const char * ) );
static CUtlMap<CUtlString, AdminData_t> g_AdminMap( DefLessFunc( CUtlString ) );

CUtlMap<CUtlString, AdminData_t> &CBase_Admin::GetAdminMap()
{
	return g_AdminMap;
}

bool CBase_Admin::bIsListenServerMsg = false;

// Was the command typed from the chat or from the console?
AdminReplySource GetCmdReplySource( CBasePlayer *pPlayer )
{
	if ( !pPlayer && UTIL_IsCommandIssuedByServerAdmin() )
	{
		return ADMIN_REPLY_SERVER_CONSOLE;
	}

	// If player and flag was set, it was chat-triggered
	if ( pPlayer && pPlayer->WasCommandUsedFromChat() )
	{
		pPlayer->SetChatCommandResetThink();  // Reset for next command
		return ADMIN_REPLY_CHAT;
	}

	return ADMIN_REPLY_CONSOLE;  // Player console (not chat)
}

void AdminReply( AdminReplySource source, CBasePlayer *pPlayer, const char *fmt, ... )
{
	char msg[ 512 ];
	va_list argptr;
	va_start( argptr, fmt );
	Q_vsnprintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );

	switch ( source )
	{
	case ADMIN_REPLY_SERVER_CONSOLE:
		Msg( "%s\n", msg );
		break;

	case ADMIN_REPLY_CONSOLE:
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "%s\n", msg ) );
		}
		break;

	case ADMIN_REPLY_CHAT:
		if ( pPlayer )
		{
			ClientPrint( pPlayer, HUD_PRINTTALK, msg );
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor/destructor
//-----------------------------------------------------------------------------
CBase_Admin::CBase_Admin()
{
	Assert( !g_pBaseAdmin );
	g_pBaseAdmin = this;

	bAll = bBlue = bRed = bAllButMe = bMe = bAlive = bDead = bBots = bHumans = false;
	bIsListenServerMsg = false;
	m_steamID = NULL;
	m_permissions = NULL;

	RegisterCommands();
}

CBase_Admin::~CBase_Admin()
{
	if ( m_steamID )
	{
		free( ( void * ) m_steamID );   // free the copied steamID string
	}

	if ( m_permissions )
	{
		free( ( void * ) m_permissions );  // free the copied permissions string
	}

	Assert( g_pBaseAdmin == this );
	g_pBaseAdmin = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Set up the admins
//-----------------------------------------------------------------------------
void CBase_Admin::Initialize( const char *steamID, const char *permissions )
{
	if ( m_steamID )
	{
		free( ( void * ) m_steamID );
		m_steamID = NULL;
	}
	if ( m_permissions )
	{
		free( ( void * ) m_permissions );
		m_permissions = NULL;
	}

	m_steamID = steamID ? V_strdup( steamID ) : NULL;
	m_permissions = permissions ? V_strdup( permissions ) : NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Reload the admins.txt file
//-----------------------------------------------------------------------------
bool CBase_Admin::ParseAdminFile( const char *filename, CUtlMap<CUtlString, AdminData_t> &outAdminMap )
{
	outAdminMap.RemoveAll();

	KeyValues *kv = new KeyValues( "Admins" );
	if ( !kv->LoadFromFile( filesystem, filename ) )
	{
		kv->deleteThis();
		return false;
	}

	for ( KeyValues *pAdmin = kv->GetFirstSubKey(); pAdmin; pAdmin = pAdmin->GetNextKey() )
	{
		const char *steamID = pAdmin->GetName();
		const char *flags = pAdmin->GetString( "flags", "" );

		outAdminMap.Insert( steamID, AdminData_t( flags ) );
	}

	kv->deleteThis();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Cache the admins
//			This allows adding and removing admins without resorting to
//			restarting the entire admin system by calling the initializer
//-----------------------------------------------------------------------------
void CBase_Admin::SaveAdminCache()
{
	KeyValues *kv = new KeyValues( "Admins" );

	for ( int i = 0; i < g_AdminList.Count(); ++i )
	{
		CBase_Admin *pAdmin = g_AdminList[ i ];
		KeyValues *pAdminKV = new KeyValues( pAdmin->GetSteamID() );
		pAdminKV->SetString( "flags", pAdmin->m_permissions );
		kv->AddSubKey( pAdminKV );
	}

	kv->SaveToFile( filesystem, "cfg/admin/admins.txt" );
	kv->deleteThis();
}

//-----------------------------------------------------------------------------
// Purpose: Admin permissions
//-----------------------------------------------------------------------------
bool CBase_Admin::HasPermission( char flag ) const
{
	if ( !m_permissions )
		return false;

	DevMsg( "Checking permission flag %c against permissions %s\n", flag, m_permissions );

	bool hasPermission = ( strchr( m_permissions, flag ) != NULL ) || ( strchr( m_permissions, ADMIN_ROOT ) != NULL );

	// quick dev perms check
	if ( hasPermission )
		DevMsg( "Admin has the required permission.\n" );
	else
		DevMsg( "Admin does NOT have the required permission.\n" );

	return hasPermission;
}


//-----------------------------------------------------------------------------
// Purpose: Add a new admin with SteamID and permissions to the global admin list
//-----------------------------------------------------------------------------
void CBase_Admin::AddAdmin( const char *steamID, const char *permissions )
{
	// Check if admin already exists
	CBase_Admin *existingAdmin = GetAdmin( steamID );
	if ( existingAdmin )
	{
		Msg( "Admin with SteamID %s already exists.\n", steamID );
		return;
	}

	// steamID and permissions must be valid
	if ( steamID == NULL || permissions == NULL || *steamID == '\0' || *permissions == '\0' )
	{
		Msg( "Invalid admin data: SteamID or permissions are null.\n" );
		return;
	}

	CBase_Admin *pNewAdmin = new CBase_Admin();

	// Set the steamID and permissions after creation
	pNewAdmin->Initialize( steamID, permissions );

	// Add to the global list
	g_AdminList.AddToTail( pNewAdmin );

	Msg( "Added admin with SteamID %s and permissions %s.\n", steamID, permissions );
}


//-----------------------------------------------------------------------------
// Purpose: Do we have an admin?
//-----------------------------------------------------------------------------
CBase_Admin *CBase_Admin::GetAdmin( const char *steamID )
{
	for ( int i = 0; i < g_AdminList.Count(); i++ )
	{
		DevMsg( "Comparing against: %s\n", g_AdminList[ i ]->GetSteamID() );
		if ( Q_stricmp( g_AdminList[ i ]->GetSteamID(), steamID ) == 0 )
		{
			return g_AdminList[ i ];
		}
	}
	return NULL;  // No admin found
}

//-----------------------------------------------------------------------------
// Purpose: Check if a player's SteamID has admin permissions
//			Different from GetAdmin() just above since we do not
//			directly get the SteamID of an active player on the
//			the server using engine->GetPlayerNetworkIDString( edict )
//-----------------------------------------------------------------------------
bool IsSteamIDAdmin( const char *steamID )
{
	KeyValues *kv = new KeyValues( "Admins" );

	if ( !kv->LoadFromFile( filesystem, "cfg/admin/admins.txt", "MOD" ) )
	{
		Msg( "Failed to open cfg/admin/admins.txt for reading.\n" );
		kv->deleteThis();
		return false;
	}

	// checking all the admin flags
	const char *adminFlags = kv->GetString( steamID, NULL );
	if ( adminFlags && *adminFlags != '\0' )
	{
		if ( Q_stristr( adminFlags, "b" ) || Q_stristr( adminFlags, "c" ) || Q_stristr( adminFlags, "d" ) ||
			Q_stristr( adminFlags, "e" ) || Q_stristr( adminFlags, "f" ) || Q_stristr( adminFlags, "g" ) ||
			Q_stristr( adminFlags, "h" ) || Q_stristr( adminFlags, "i" ) || Q_stristr( adminFlags, "j" ) ||
			Q_stristr( adminFlags, "k" ) || Q_stristr( adminFlags, "l" ) || Q_stristr( adminFlags, "m" ) ||
			Q_stristr( adminFlags, "n" ) || Q_stristr( adminFlags, "z" ) )
		{
			kv->deleteThis();
			return true;
		}
	}

	kv->deleteThis();
	return false;
}

//ConVar sa_listenserverhostimmune("sa_listenserverhostimmune", "1", FCVAR_DEVELOPMENTONLY);

//-----------------------------------------------------------------------------
// Purpose: Check if a player has admin permissions
//-----------------------------------------------------------------------------
bool CBase_Admin::IsPlayerAdmin( CBasePlayer *pPlayer, const char *requiredFlags )
{
	if ( !pPlayer )
		return false;

	if (!engine->IsDedicatedServer() && pPlayer == UTIL_GetListenServerHost())
	{
		// We only need to see this once
		if ( !bIsListenServerMsg )
		{
			Msg( "Not a dedicated server, local server host is %s; granting all permissions\n", pPlayer->GetPlayerName() );
			bIsListenServerMsg = true;
		}
		return true;
	}

	const char *steamID = engine->GetPlayerNetworkIDString( pPlayer->edict() );

	CBase_Admin *pAdmin = GetAdmin( steamID );

	if ( pAdmin )
	{
		if ( pAdmin->HasPermission( ADMIN_ROOT ) )  // root is z, all permissions
		{
			DevMsg( "Admin has root ('z') flag, granting all permissions.\n" );
			return true;
		}

		// else does this player have at least one flag?
		for ( int i = 0; requiredFlags[ i ] != '\0'; i++ )
		{
			if ( pAdmin->HasPermission( requiredFlags[ i ] ) )
			{
				// they do
				return true;
			}
		}
	}

	// or they don't
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Clear all admins (e.g., when the server resets or map changes)
//-----------------------------------------------------------------------------
void CBase_Admin::ClearAllAdmins()
{
	g_AdminList.PurgeAndDeleteElements();
	DevMsg( "All admins have been cleared from the server.\n" );
}

void AppendDuration(CUtlString& chatMessage, const char* action, const char* duration)
{
	if (duration && duration[0] != '\0')
	{
		if (Q_stricmp(action, "banned") == 0 && Q_stricmp(duration, "permanently") == 0)
		{
			chatMessage.Append(" permanently");
		}
		else
		{
			chatMessage.Append(UTIL_VarArgs(" for %s", duration));
		}
	}
}

void AppendLogReason(CUtlString& logDetails, bool hasReason, const char* reason = NULL)
{
	if (hasReason)
	{
		logDetails.Append(UTIL_VarArgs(" (Reason: %s)", (reason && reason[0] != '\0') ? reason : "No reason provided"));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Centralized function for sending messages and logging admin actions
//			when using a special group target like @all, @alive, @humans, etc.
//-----------------------------------------------------------------------------
void BuildGroupTargetMessage(
	const char *partialName,
	CBasePlayer *pPlayer,
	const char *action,
	const char *duration,
	CUtlString &logDetails,
	CUtlString &chatMessage,
	bool hasReason,
	const char *reason)
{
	const char *adminName = pPlayer ? pPlayer->GetPlayerName() : "Console";

	if ( !Q_stricmp( partialName, "@me" ) )
	{
		logDetails.Format( "themself%s%s", duration ? " " : "", duration ? duration : "" );
		chatMessage = UTIL_VarArgs( "%s %s themself", adminName, action );
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}
	else if ( !Q_stricmp( partialName, "@!me" ) )
	{
		logDetails.Format( "all players except themself%s%s", duration ? " " : "", duration ? duration : "" );
		chatMessage = UTIL_VarArgs( "%s %s everyone but themself", adminName, action );
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}
	else if ( !Q_stricmp( partialName, "@all" ) )
	{
		logDetails.Format( "all players%s%s", duration ? " " : "", duration ? duration : "" );
		chatMessage = UTIL_VarArgs( "%s %s everyone", adminName, action );
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}
	else if ( !Q_stricmp( partialName, "@bots" ) )
	{
		logDetails.Format( "all bots%s%s", duration ? " " : "", duration ? duration : "" );
		chatMessage = UTIL_VarArgs( "%s %s all bots", adminName, action );
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}
	else if ( !Q_stricmp( partialName, "@humans" ) )
	{
		logDetails.Format( "all human players%s%s", duration ? " " : "", duration ? duration : "" );
		chatMessage = UTIL_VarArgs( "%s %s all human players", adminName, action );
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}
	else if ( !Q_stricmp( partialName, "@alive" ) )
	{
		logDetails.Format( "all alive players%s%s", duration ? " " : "", duration ? duration : "" );
		chatMessage = UTIL_VarArgs( "%s %s all alive players", adminName, action );
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}
	else if ( !Q_stricmp( partialName, "@dead" ) )
	{
		logDetails.Format( "all dead players%s%s", duration ? " " : "", duration ? duration : "" );
		chatMessage = UTIL_VarArgs( "%s %s all dead players", adminName, action );
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}
	else if ( !Q_stricmp( partialName, "@red" ) )
	{
#ifdef HL2MP
		logDetails.Format("all Rebels players%s%s", duration ? " " : "", duration ? duration : "");
		chatMessage = UTIL_VarArgs("%s %s all Rebels players", adminName, action);
#elif TF_DLL
		logDetails.Format("all RED players%s%s", duration ? " " : "", duration ? duration : "");
		chatMessage = UTIL_VarArgs("%s %s all RED players", adminName, action);
#endif
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}
	else if ( !Q_stricmp( partialName, "@blue" ) )
	{
#ifdef HL2MP
		logDetails.Format("all Combine players%s%s", duration ? " " : "", duration ? duration : "");
		chatMessage = UTIL_VarArgs("%s %s all Combine players", adminName, action);
#elif TF_DLL
		logDetails.Format("all BLU players%s%s", duration ? " " : "", duration ? duration : "");
		chatMessage = UTIL_VarArgs("%s %s all BLU players", adminName, action);
#endif
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}
	else
	{
		logDetails.Format( "players in group %s%s%s", partialName + 1, duration ? " " : "", duration ? duration : "" );
		chatMessage = UTIL_VarArgs( "%s %s all players in group %s", adminName, action, partialName + 1 );
		AppendDuration(chatMessage, action, duration);
		AppendLogReason(logDetails, hasReason, reason);
	}

	if ( hasReason && reason && reason[ 0 ] != '\0' )
	{
		chatMessage.Append( UTIL_VarArgs( ". Reason: %s", reason ) );
	}
}

int CBase_Admin::GetRedNumber()
{
#ifdef HL2MP
	return TEAM_REBELS;
#elif TF_DLL
	return TF_TEAM_RED;
#else
	return TEAM_UNASSIGNED;
#endif
}

int CBase_Admin::GetBlueNumber()
{
#ifdef HL2MP
	return TEAM_COMBINE;
#elif TF_DLL
	return TF_TEAM_BLUE;
#else
	return TEAM_UNASSIGNED;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Find players or target groups
//-----------------------------------------------------------------------------
// We're going to be calling the functions below a lot
bool ParsePlayerTargets(
	CBasePlayer *pAdmin,
	AdminReplySource replySource,
	const char *partialName,
	CUtlVector<CBasePlayer *> &targetPlayers,
	CBasePlayer *&pSingleTarget,
	bool excludeDeadPlayers)
{
	pSingleTarget = NULL;
	targetPlayers.RemoveAll();

	if ( partialName[ 0 ] == '@' )
	{
		int index = g_SpecialTargets.Find( partialName );
		if ( index == g_SpecialTargets.InvalidIndex() )
		{
			AdminReply( replySource, pAdmin, "Invalid special target specifier." );
			return false;
		}

		if ( !pAdmin && ( !Q_stricmp( partialName, "@me" ) || !Q_stricmp( partialName, "@!me" ) ) )
		{
			AdminReply( replySource, pAdmin, "The console cannot use special target %s.", partialName );
			return false;
		}

		if ( BaseAdmin()->FindSpecialTargetGroup( partialName ) )
		{
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
				if ( !pPlayer || !pPlayer->IsPlayer() )
					continue;

				if ( excludeDeadPlayers && !pPlayer->IsAlive() )
					continue;

				if ( ( BaseAdmin()->IsAllPlayers() ) ||
					( BaseAdmin()->IsAllBluePlayers() && pPlayer->GetTeamNumber() == BaseAdmin()->GetBlueNumber() ) ||
					( BaseAdmin()->IsAllRedPlayers() && pPlayer->GetTeamNumber() == BaseAdmin()->GetRedNumber() ) ||
					( BaseAdmin()->IsAllButMePlayers() && pPlayer != pAdmin ) ||
					( BaseAdmin()->IsMe() && pPlayer == pAdmin ) ||
					( BaseAdmin()->IsAllAlivePlayers() && pPlayer->IsAlive() ) ||
					( BaseAdmin()->IsAllDeadPlayers() && !pPlayer->IsAlive() ) ||
					( BaseAdmin()->IsAllBotsPlayers() && pPlayer->IsPlayerBot() ) ||
					( BaseAdmin()->IsAllHumanPlayers() && !pPlayer->IsPlayerBot() ) )
				{
					targetPlayers.AddToTail( pPlayer );
				}
			}
			BaseAdmin()->ResetSpecialTargetGroup();
		}

		if ( targetPlayers.Count() == 0 )
		{
			if ( !Q_stricmp( partialName, "@alive" ) )
			{
				AdminReply( replySource, pAdmin, "All players in the target group are dead." );
			}
			else
			{
				AdminReply( replySource, pAdmin, "No players found matching the target group." );
			}			
			return false;
		}

		return true;
	}

	if ( partialName[ 0 ] == '#' )
	{
		int userID = atoi( &partialName[ 1 ] );
		if ( userID > 0 )
		{
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
				if ( pPlayer && pPlayer->GetUserID() == userID )
				{
					if ( excludeDeadPlayers && !pPlayer->IsAlive() )
					{
						AdminReply( replySource, pAdmin, "Player is currently dead." );
						return false;
					}

					pSingleTarget = pPlayer;
					return true;
				}
			}
		}

		AdminReply( replySource, pAdmin, "No player found with that UserID." );
		return false;
	}

	// Partial name search
	CUtlVector<CBasePlayer *> matchingPlayers;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer && Q_stristr( pPlayer->GetPlayerName(), partialName ) )
		{
			matchingPlayers.AddToTail( pPlayer );
		}
	}

	if ( matchingPlayers.Count() == 0 )
	{
		AdminReply( replySource, pAdmin, "No players found matching that name." );
		return false;
	}
	else if ( matchingPlayers.Count() > 1 )
	{
		AdminReply( replySource, pAdmin, "Multiple players match that name:" );
		for ( int i = 0; i < matchingPlayers.Count(); i++ )
		{
			AdminReply( replySource, pAdmin, "%s", matchingPlayers[ i ]->GetPlayerName() );
		}
		return false;
	}

	pSingleTarget = matchingPlayers[ 0 ];

	if ( excludeDeadPlayers && pSingleTarget && !pSingleTarget->IsAlive() )
	{
		AdminReply( replySource, pAdmin, "Player is currently dead." );
		pSingleTarget = NULL;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Print messages when action occurs
//-----------------------------------------------------------------------------
void PrintActionMessage(
	CBasePlayer *pPlayer,
	bool isServerConsole,
	const char *action,
	const char *targetName,
	const char *duration,
	const char *reason)
{
	CUtlString message;
	const char *adminName = pPlayer ? pPlayer->GetPlayerName() : "Console";

	message.Format( "%s %s %s", adminName, action, targetName );

	// Special case for bans
	if ( duration && duration[ 0 ] != '\0' )
	{
		if ( Q_stricmp( action, "banned" ) == 0 )
		{
			if ( Q_stricmp( duration, "permanently" ) == 0 )
			{
				message.Format( "%s permanently banned %s", adminName, targetName );
			}
			else
			{
				message.Format( "%s banned %s for %s", adminName, targetName, duration );
			}
		}
		else
		{
			message.Append( UTIL_VarArgs( " (%s)", duration ) );
		}
	}

	if ( reason && reason[ 0 ] != '\0' )
	{
		message.Append( UTIL_VarArgs( ". Reason: %s", reason ) );
	}

	UTIL_ClientPrintAll( HUD_PRINTTALK, UTIL_VarArgs( "%s.\n", message.Get() ) );
}

//-----------------------------------------------------------------------------
// Purpose: Display version
//-----------------------------------------------------------------------------
// 10/17/24 - First version
#define BASE_YEAR 2024
#define BASE_MONTH 10
#define BASE_DAY 17

int GetBuildNumber()
{
	struct tm baseDate = {};
	baseDate.tm_year = BASE_YEAR - 1900;
	baseDate.tm_mon = BASE_MONTH - 1;
	baseDate.tm_mday = BASE_DAY;

	// Current date (parsed from __DATE__)
	struct tm currentDate = {};
	const char *compileDate = __DATE__;
	char monthStr[ 4 ] = {};
	int day, year;

	sscanf( compileDate, "%3s %d %d", monthStr, &day, &year );

	// Convert month string to index
	const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
	const char *pos = strstr( months, monthStr );
	if ( pos )
	{
		currentDate.tm_mon = ( pos - months ) / 3;
	}
	currentDate.tm_year = year - 1900;
	currentDate.tm_mday = day;

	// Convert to time_t
	time_t baseTime = mktime( &baseDate );
	time_t currentTime = mktime( &currentDate );

	if ( baseTime == -1 || currentTime == -1 )
	{
		return 0;
	}

	// Difference in days = build number
	return static_cast< int >( difftime( currentTime, baseTime ) / ( 60 * 60 * 24 ) );
}

void InitializeSpecialTargets()
{
	g_SpecialTargets.Insert( "@all", TARGET_ALL );
	g_SpecialTargets.Insert( "@blue", TARGET_BLUE );
	g_SpecialTargets.Insert( "@red", TARGET_RED );
	g_SpecialTargets.Insert( "@!me", TARGET_ALL_BUT_ME );
	g_SpecialTargets.Insert( "@me", TARGET_ME );
	g_SpecialTargets.Insert( "@alive", TARGET_ALIVE );
	g_SpecialTargets.Insert( "@dead", TARGET_DEAD );
	g_SpecialTargets.Insert( "@bots", TARGET_BOTS );
	g_SpecialTargets.Insert( "@humans", TARGET_HUMANS );
}

void CBase_Admin::ResetSpecialTargetGroup()
{
	bAll = bBlue = bRed = bAllButMe = bMe = bAlive = bDead = bBots = bHumans = false;
}

bool CBase_Admin::FindSpecialTargetGroup( const char *targetSpecifier )
{
	int index = g_SpecialTargets.Find( targetSpecifier );

	if ( index == g_SpecialTargets.InvalidIndex() )
		return false;

	switch ( g_SpecialTargets[ index ] )
	{
	case TARGET_ALL: bAll = true; break;
	case TARGET_BLUE: bBlue = true; break;
	case TARGET_RED: bRed = true; break;
	case TARGET_ALL_BUT_ME: bAllButMe = true; break;
	case TARGET_ME: bMe = true; break;
	case TARGET_ALIVE: bAlive = true; break;
	case TARGET_DEAD: bDead = true; break;
	case TARGET_BOTS: bBots = true; break;
	case TARGET_HUMANS: bHumans = true; break;
	default: return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Show all admin commands via "sa" main command
//-----------------------------------------------------------------------------
AdminCommandFunction FindAdminCommand(const char* cmd)
{
	FOR_EACH_VEC(BaseAdmin()->GetCommands(), i)
	{
		const CommandEntry *entry = BaseAdmin()->GetCommands()[i];

		if (Q_stricmp(entry->chatCommand, cmd) == 0)
		{
			return entry->function;
		}
	}

	return NULL;
}

bool IsCommandAllowed(const char* cmd, bool isServerConsole, CBasePlayer* pAdmin)
{
	FOR_EACH_VEC(BaseAdmin()->GetCommands(), i)
	{
		const CommandEntry *entry = BaseAdmin()->GetCommands()[i];

		if (Q_stricmp(entry->chatCommand, cmd) == 0)
		{
			if (!isServerConsole)
			{
				bool bImmunity = false;

				if (CBase_Admin::IsPlayerAdmin(pAdmin, "z"))
				{
					return true;
				}

				if (!bImmunity)
				{
					if (strchr(entry->requiredFlags, ADMIN_UNDEFINED) != NULL)
					{
						return true;
					}

					if (!CBase_Admin::IsPlayerAdmin(pAdmin, entry->requiredFlags))
					{
						//do we at least have the b flag?
						if (CBase_Admin::IsPlayerAdmin(pAdmin, "b"))
						{
							return true;
						}
						else
						{
							return false;
						}
					}
				}
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the admin system (parse the file, add admins, register commands)
//-----------------------------------------------------------------------------
void CBase_Admin::InitAdminSystem()
{
	if ( CommandLine()->CheckParm( "-noadmin" ) )
		return;

	g_bAdminSystem = true;

	CBase_Admin::ClearAllAdmins();
	InitializeSpecialTargets();

	new CBase_Admin();

	if ( !filesystem->IsDirectory( "cfg/admin/logs", "GAME" ) )
	{
		filesystem->CreateDirHierarchy( "cfg/admin/logs", "GAME" );
	}

	CUtlMap<CUtlString, AdminData_t> newAdminMap( DefLessFunc( CUtlString ) );

	if ( !ParseAdminFile( "cfg/admin/admins.txt", newAdminMap ) )
	{
		Warning( "Error: Unable to load admins.txt\nDoes the file exist and is placed in the right location?\n" );
		return;
	}

	// Populate g_AdminList and g_AdminMap directly from parsed data
	g_AdminMap.RemoveAll();

	for ( int i = newAdminMap.FirstInorder(); i != newAdminMap.InvalidIndex(); i = newAdminMap.NextInorder( i ) )
	{
		const CUtlString &steamID = newAdminMap.Key( i );
		AdminData_t &adminData = newAdminMap.Element( i );

		CBase_Admin *newAdmin = new CBase_Admin();
		newAdmin->Initialize( steamID.Get(), adminData.flags.Get() );
		g_AdminList.AddToTail( newAdmin );

		g_AdminMap.Insert( steamID, adminData );
	}

	DevMsg( "Admin list loaded from admins.txt.\n" );

	// Initialize log file
	char date[ 9 ];
	time_t now = time( 0 );
	strftime( date, sizeof( date ), "%Y%m%d", localtime( &now ) );

	char logFileName[ 256 ];
	Q_snprintf( logFileName, sizeof( logFileName ), "cfg/admin/logs/ADMINLOG_%s.txt", date );

	g_AdminLogFile = filesystem->Open( logFileName, "a+", "GAME" );
	if ( !g_AdminLogFile )
	{
		Warning( "Unable to create admin log file, but it will be created on first admin command usage.\n" );
		g_bAdminSystem = false;
	}
	else
	{
		Msg( "Admin log initialized: %s\n", logFileName );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Checks chat for certain strings (chat commands)
//-----------------------------------------------------------------------------
void CBase_Admin::CheckChatText( char *p, int bufsize )
{
	if ( !g_bAdminSystem )
		return;

	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !p || bufsize <= 0 )
		return;

	if ( p[ 0 ] != '!' && p[ 0 ] != '/' )
		return;

	p = p + 1;

	FOR_EACH_VEC(BaseAdmin()->GetCommands(), i)
	{
		const CommandEntry *entry = BaseAdmin()->GetCommands()[i];

		size_t cmdLen = strlen(entry->chatCommand);
		if (Q_strncmp(p, entry->chatCommand, cmdLen) == 0)
		{
			char consoleCmd[256];

			if (entry->requiresArguments)
			{
				const char* args = p + cmdLen;
				Q_snprintf(consoleCmd, sizeof(consoleCmd), "sa %s%s", entry->chatCommand, args);
			}
			else
			{
				Q_snprintf(consoleCmd, sizeof(consoleCmd), "sa %s", entry->chatCommand);
			}

			if (pPlayer)
			{
				pPlayer->SetLastCommandWasFromChat(true);
				engine->ClientCommand(pPlayer->edict(), consoleCmd);
				if (entry->consoleMessage)
				{
					ClientPrint(pPlayer, HUD_PRINTTALK, entry->consoleMessage);
				}
			}
			return;
		}
	}

}

//-----------------------------------------------------------------------------
// Purpose: Action log
//-----------------------------------------------------------------------------
void CBase_Admin::LogAction( CBasePlayer *pAdmin, CBasePlayer *pTarget, const char *action, const char *details, const char *groupTarget )
{
	if ( g_AdminLogFile == FILESYSTEM_INVALID_HANDLE )
	{
		// Try to reopen the log file if it's missing.
		char date[ 9 ];
		time_t now = time( 0 );
		strftime( date, sizeof( date ), "%Y%m%d", localtime( &now ) );

		char logFileName[ 256 ];
		Q_snprintf( logFileName, sizeof( logFileName ), "cfg/admin/logs/ADMINLOG_%s.txt", date );

		g_AdminLogFile = filesystem->Open( logFileName, "a+", "GAME" );

		if ( g_AdminLogFile == FILESYSTEM_INVALID_HANDLE )
		{
			Warning( "Failed to open admin log file: %s\n", logFileName );
			return;
		}
	}

	time_t now = time( 0 );
	struct tm *localTime = localtime( &now );
	char dateString[ 11 ];
	char timeString[ 9 ];
	strftime( dateString, sizeof( dateString ), "%Y/%m/%d", localTime );
	strftime( timeString, sizeof( timeString ), "%H:%M:%S", localTime );

	const char *mapName = STRING( gpGlobals->mapname );
	const char *adminName = pAdmin ? pAdmin->GetPlayerName() : "Console";
	const char *adminSteamID = pAdmin ? engine->GetPlayerNetworkIDString( pAdmin->edict() ) : "Console";
	const char *targetName = pTarget ? pTarget->GetPlayerName() : "";
	const char *targetSteamID = pTarget ? engine->GetPlayerNetworkIDString( pTarget->edict() ) : "";

	CUtlString logEntry;

	if ( pTarget )
	{
		if ( Q_strlen( details ) > 0 )
		{
			logEntry.Format( "[%s] %s @ %s => Admin %s <%s> %s %s <%s> %s\n",
				mapName, dateString, timeString, adminName, adminSteamID,
				action, targetName, targetSteamID, details );
		}
		else
		{
			logEntry.Format( "[%s] %s @ %s => Admin %s <%s> %s %s <%s>\n",
				mapName, dateString, timeString, adminName, adminSteamID,
				action, targetName, targetSteamID );
		}
	}
	else if ( groupTarget )
	{
		if ( Q_strlen( details ) > 0 )
		{
			logEntry.Format( "[%s] %s @ %s => Admin %s <%s> %s %s\n",
				mapName, dateString, timeString, adminName, adminSteamID,
				action, details );
		}
		else
		{
			logEntry.Format( "[%s] %s @ %s => Admin %s <%s> %s\n",
				mapName, dateString, timeString, adminName, adminSteamID,
				action );
		}
	}
	else
	{
		if ( Q_strlen( details ) > 0 )
		{
			logEntry.Format( "[%s] %s @ %s => Admin %s <%s> %s %s\n",
				mapName, dateString, timeString, adminName, adminSteamID,
				action, details );
		}
		else
		{
			logEntry.Format( "[%s] %s @ %s => Admin %s <%s> %s\n",
				mapName, dateString, timeString, adminName, adminSteamID,
				action );
		}
	}

	filesystem->FPrintf( g_AdminLogFile, "%s", logEntry.Get() );
	filesystem->Flush( g_AdminLogFile );
}

void CBase_Admin::ReloadCommands(void)
{
	g_AdminCommands.PurgeAndDeleteElements();
	RegisterCommands();
	DevMsg("Reloaded commands.\n");
}

void ToggleModule_ChangeCallback(IConVar* pConVar, char const* pOldString, float flOldValue)
{
	BaseAdmin()->ReloadCommands();
}

const char* PrintHelpStringForCommand(const char* cmd)
{
	FOR_EACH_VEC(BaseAdmin()->GetCommands(), i)
	{
		const CommandEntry* entry = BaseAdmin()->GetCommands()[i];

		if (Q_strncmp(cmd, entry->chatCommand, strlen(entry->chatCommand)) == 0)
		{
			return UTIL_VarArgs("[%s] (Flags: %s) %s %s\n", entry->moduleName, entry->requiredFlags, entry->chatCommand, entry->helpMessage);
		}
	}

	return "";
}

void PrintCommandHelpStrings(bool isServerConsole, CBasePlayer* pAdmin)
{
	const char* szTitle = "[Server Admin] Usage: sa <command> [arguments]\n==============================================\n";

	if (isServerConsole)
	{
		Msg(szTitle);
	}
	else
	{
		if (pAdmin)
		{
			ClientPrint(pAdmin, HUD_PRINTCONSOLE, szTitle);
		}
	}

	if (BaseAdmin()->GetCommands().IsEmpty())
	{
		const char* szError = "No commands found!\n";

		if (isServerConsole)
		{
			Msg(szError);
		}
		else
		{
			if (pAdmin)
			{
				ClientPrint(pAdmin, HUD_PRINTCONSOLE, szError);
			}
		}
		return;
	}

	FOR_EACH_VEC(BaseAdmin()->GetCommands(), i)
	{
		const CommandEntry* entry = BaseAdmin()->GetCommands()[i];

		if (!isServerConsole)
		{
			if (pAdmin && !IsCommandAllowed(entry->chatCommand, false, pAdmin))
				continue;
		}

		const char* szMsg = PrintHelpStringForCommand(entry->chatCommand);

		if (isServerConsole)
		{
			Msg(szMsg);
		}
		else
		{
			if (pAdmin)
			{
				ClientPrint(pAdmin, HUD_PRINTCONSOLE, szMsg);
			}
		}
	}
}

static void PrintAdminHelp(CBasePlayer* pPlayer = NULL, bool isServerConsole = false)
{
	if (isServerConsole)
	{
		PrintCommandHelpStrings(true, NULL);
		return;
	}

	if (!pPlayer)
		return;

	PrintCommandHelpStrings(false, pPlayer);
}

static void AdminCommand(const CCommand& args)
{
	CBasePlayer* pPlayer = UTIL_GetCommandClient();

	if (!g_bAdminSystem && engine->IsDedicatedServer())
	{
		if (UTIL_IsCommandIssuedByServerAdmin())
		{
			Msg("Admin system disabled by the -noadmin launch command\nRemove launch command and restart the server\n");
		}
		else if (pPlayer)
		{
			ClientPrint(pPlayer, HUD_PRINTTALK, "Admin system disabled by the -noadmin launch command\n");
		}
		return;
	}

	AdminReplySource replySource = GetCmdReplySource(pPlayer);

	// Handle "sa" with no arguments (print help menu)
	if (args.ArgC() < 2)
	{
		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			PrintAdminHelp(NULL, true);
		}
		else if (pPlayer)
		{
			PrintAdminHelp(pPlayer);
		}
		return;
	}

	// Extract the subcommand
	const char* subCommand = args.Arg(1);

	AdminCommandFunction commandFunc = FindAdminCommand(subCommand);
	if (!commandFunc)
	{
		if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
		{
			Msg("[Server Admin] Unknown command: %s\n", subCommand);
			PrintAdminHelp(NULL, true);
		}
		else if (pPlayer)
		{
			AdminReply(replySource, pPlayer, "[Server Admin] Unknown command: %s", subCommand);
			PrintAdminHelp(pPlayer);
		}
		return;
	}

	// Server console can run anything without permission checks
	if (replySource == ADMIN_REPLY_SERVER_CONSOLE)
	{
		commandFunc(args);
		return;
	}

	bool isAllowed = false;

	if (pPlayer)
	{
		isAllowed = IsCommandAllowed(subCommand, false, pPlayer);
	}

	if (!isAllowed)
	{
		AdminReply(replySource, pPlayer, "You do not have access to this command.");
		return;
	}

	commandFunc(args);
}

ConCommand sa("sa", AdminCommand, "Admin menu.", FCVAR_SERVER_CAN_EXECUTE | FCVAR_CLIENTCMD_CAN_EXECUTE);

#endif