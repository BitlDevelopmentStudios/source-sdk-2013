//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "anticitzen_demo_support.h"
#include "vguicenterprint.h"
#include "time.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

// Global singleton
static CAnticitizenDemoSupport g_DemoSupport;

ConVar ds_enable( "ds_enable", "0", FCVAR_CLIENTDLL | FCVAR_DONTRECORD | FCVAR_ARCHIVE, "Demo support - enable automatic .dem file recording and features. 0 - Manual, 1 - Auto-record all matches", true, 0, true, 3 ); 
ConVar ds_dir( "ds_dir", "demos", FCVAR_CLIENTDLL | FCVAR_DONTRECORD | FCVAR_ARCHIVE, "Demo support - will put all files into this folder under the gamedir. 24 characters max." );
ConVar ds_prefix( "ds_prefix", "anticitizen_", FCVAR_CLIENTDLL | FCVAR_DONTRECORD | FCVAR_ARCHIVE, "Demo support - will prefix files with this string. 24 characters max." );
ConVar ds_sound("ds_sound", "1", FCVAR_CLIENTDLL | FCVAR_DONTRECORD | FCVAR_ARCHIVE, "Demo support - play start sound for demo recording.", true, 0, true, 1);
ConVar ds_sound_file("ds_sound_file", "buttons/button1.wav", FCVAR_CLIENTDLL | FCVAR_DONTRECORD | FCVAR_ARCHIVE, "Demo support - name of the sound to play when demo recording starts.");

CON_COMMAND_F( ds_record, "Demo support - start recording a demo.", FCVAR_CLIENTDLL | FCVAR_DONTRECORD )
{
	g_DemoSupport.StartRecording();
}

CON_COMMAND_F( ds_stop, "Demo support - stop recording a demo.", FCVAR_CLIENTDLL | FCVAR_DONTRECORD )
{
	g_DemoSupport.StopRecording();
}

CON_COMMAND_F( ds_status, "Demo support - show the current recording status.", FCVAR_CLIENTDLL | FCVAR_DONTRECORD )
{
	g_DemoSupport.Status();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAnticitizenDemoSupport::CAnticitizenDemoSupport() : CAutoGameSystemPerFrame( "CAnticitizenDemoSupport" )
{
	m_bRecording = false;
	m_bAlreadyAutoRecordedOnce = false;
	m_flNextRecordStartCheckTime = -1.f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAnticitizenDemoSupport::Init()
{
	ListenForGameEvent( "client_disconnect" );
	ListenForGameEvent( "ds_stop" );

	DemoSupport_Log("DemoSupport initalized.\n");
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizenDemoSupport::LevelInitPostEntity()
{
	if ( engine->IsPlayingDemo() )
		return;

	m_bAlreadyAutoRecordedOnce = false;
	m_flNextRecordStartCheckTime = -1.f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizenDemoSupport::LevelShutdownPostEntity()
{
	if ( engine->IsPlayingDemo() )
		return;

	StopRecording();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizenDemoSupport::Update( float frametime )
{
	if ( engine->IsPlayingDemo() )
		return;

	const char* pLevelName = engine->GetLevelName();
	bool inLevel = (pLevelName && *pLevelName && !engine->IsLevelMainMenuBackground());

	if (!inLevel)
		return;

	if ( ds_enable.GetInt() > 0 )
	{
		if ( !m_bRecording && !m_bAlreadyAutoRecordedOnce )
		{
			if ( ( m_flNextRecordStartCheckTime < 0 ) || ( m_flNextRecordStartCheckTime < gpGlobals->curtime ) )
			{
				if (!StartRecording())
				{
					// we'll try again in 5 seconds
					float curtime = gpGlobals->curtime;
					m_flNextRecordStartCheckTime = curtime + 5.f;
					DemoSupport_Log("Trying again in %f seconds.\n", m_flNextRecordStartCheckTime - curtime);
					return;
				}
			}
		}
	}

	if ( !m_bRecording )
		return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizenDemoSupport::Status( void )
{
	if ( engine->IsPlayingDemo() )
		return;

	char szStatus[64] = {0};

	if ( m_bRecording )
	{
		V_sprintf_safe( szStatus, "Currently recording to %s\n", m_szFolderAndFilename );
	}
	else
	{
		V_strcpy_safe( szStatus, "Not currently recording\n" );
	}

	DemoSupport_Log("%s\n", szStatus);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizenDemoSupport::FireGameEvent( IGameEvent * event )
{
	if ( engine->IsPlayingDemo() )
		return;

	if ( !m_bRecording )
		return;

	const char *pszEvent = event->GetName();

	if ( FStrEq( pszEvent, "client_disconnect" ) )
	{
		StopRecording();
	}
	else if ( FStrEq( pszEvent, "ds_stop" ) )
	{
		StopRecording( true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAnticitizenDemoSupport::IsValidPath( const char *pszFolder )
{
	if ( !pszFolder )
		return false;

	if ( Q_strlen( pszFolder ) <= 0 ||
		Q_strstr( pszFolder, "\\\\" ) ||	// to protect network paths
		Q_strstr( pszFolder, ":" ) ||	// to protect absolute paths
		Q_strstr( pszFolder, ".." ) ||	// to protect relative paths
		Q_strstr( pszFolder, "\n" ) ||	// CFileSystem_Stdio::FS_fopen doesn't allow this
		Q_strstr( pszFolder, "\r" ) )	// CFileSystem_Stdio::FS_fopen doesn't allow this
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CAnticitizenDemoSupport::StartRecording( void )
{
	if ( engine->IsPlayingDemo() )
		return false;

	// are we already recording?
	if ( m_bRecording )
	{
		DemoSupport_Log("Already recording\n");
		return false;
	}

	// start recording the demo
	char szTime[k_RTimeRenderBufferSize] = {0};

	time_t ltime = time(0);
	const time_t* ptime = &ltime;
	struct tm* today = localtime(ptime);

	V_sprintf_safe(szTime, "%04u-%02u-%02u_%02u-%02u-%02u",
		today->tm_year + 1900, today->tm_mon + 1, today->tm_mday,
		today->tm_hour, today->tm_min, today->tm_sec);

	char szPrefix[24] = {0};
	V_sprintf_safe( szPrefix, "%s", ds_prefix.GetString() );

	if ( Q_strlen( ds_dir.GetString() ) > 0 )
	{
		// check folder
		if ( !IsValidPath( ds_dir.GetString() ) )
		{
			DemoSupport_Log("Invalid folder.\n" );
			return false;
		}

		V_sprintf_safe( m_szFolder, "%s", ds_dir.GetString() );

		// make sure the folder exists
		g_pFullFileSystem->CreateDirHierarchy( m_szFolder, "GAME" );

		V_sprintf_safe( m_szFolderAndFilename, "%s%c%s", m_szFolder, CORRECT_PATH_SEPARATOR, m_szFilename );
	}
	else
	{
		m_szFolder[0] = '\0';
		V_sprintf_safe( m_szFolderAndFilename, "%s", m_szFilename );
	}

	if ( !engine->StartDemoRecording( m_szFilename, m_szFolder ) )
	{
		DemoSupport_Log("Unable to start recording\n" );
		return false;
	}

	m_bRecording = true;
	m_bAlreadyAutoRecordedOnce = true;

	if (ds_sound.GetBool())
	{
		vgui::surface()->PlaySound(ds_sound_file.GetString());
	}

	char szMessage[MAX_PATH] = { 0 };
	V_sprintf_safe( szMessage, "Started recording to %s\n", m_szFolderAndFilename );
	DemoSupport_Log("%s\n", szMessage );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizenDemoSupport::StopRecording( bool bFromEngine /* = false */ )
{
	if ( engine->IsPlayingDemo() )
		return;

	if ( !m_bRecording )
		return;

	m_bRecording = false;

	// stop recording the demo
	if ( !bFromEngine )
	{
		engine->StopDemoRecording();
	}

	char szMessage[MAX_PATH] = { 0 };
	V_sprintf_safe( szMessage, "Ended recording to %s\n", m_szFolderAndFilename );
	DemoSupport_Log("%s\n", szMessage );
}


