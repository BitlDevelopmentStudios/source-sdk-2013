//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Revision: $
// $NoKeywords: $
//=============================================================================//

#ifndef FR_DEMO_SUPPORT_H
#define FR_DEMO_SUPPORT_H

// Render buffer size from rtime.h
const size_t k_RTimeRenderBufferSize = 25;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CAnticitizenDemoSupport : public CAutoGameSystemPerFrame, public CGameEventListener
{
public:
	CAnticitizenDemoSupport();

	virtual bool Init() OVERRIDE;
	virtual void Update( float frametime ) OVERRIDE;
	virtual char const *Name() OVERRIDE { return "CAnticitizenDemoSupport"; }
	virtual void FireGameEvent( IGameEvent *event ) OVERRIDE;
	virtual void LevelInitPostEntity() OVERRIDE;
	virtual void LevelShutdownPostEntity() OVERRIDE;
	bool StartRecording( void );
	void StopRecording( bool bFromEngine = false );
	bool IsRecording( void ){ return m_bRecording; }
	void Status( void );

private:
	bool IsValidPath( const char *pszFolder );

	bool m_bRecording;
	char m_szFolder[24];
	char m_szFilename[MAX_PATH];
	char m_szFolderAndFilename[MAX_PATH];
	bool m_bAlreadyAutoRecordedOnce;
	float m_flNextRecordStartCheckTime;
};

#define DemoSupport_Log(...) ConColorMsg( Color( 82, 235, 179, 255 ), "[AnticitizenDemoSupport] " __VA_ARGS__ )

#endif // TF_DEMO_SUPPORT_H
