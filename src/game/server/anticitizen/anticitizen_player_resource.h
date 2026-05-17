//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK CPlayerResource
//
// $NoKeywords: $
//=============================================================================//

#ifndef ANTICITIZEN_PLAYER_RESOURCE_H
#define ANTICITIZEN_PLAYER_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "player_resource.h"
#include "anticitizen_playerclass_info_parse.h"

class CAnticitizen_PlayerResource : public CPlayerResource
{
	DECLARE_CLASS( CAnticitizen_PlayerResource, CPlayerResource );
	
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CAnticitizen_PlayerResource();
	~CAnticitizen_PlayerResource();

	virtual void UpdatePlayerData( void );
	virtual void Spawn( void );

	void AddPlayerClass(const char* szClassName);
	const CAnticitizen_FilePlayerClassInfo_t& GetPlayerClassInfo(int iPlayerClass) const;
	int GetNumPlayerClasses(void) { return m_hPlayerClassInfoHandles.Count(); }

protected:

	CNetworkArray( int, m_iPlayerClass, MAX_PLAYERS+1 );

private:
	CUtlVector < PLAYERCLASS_FILE_INFO_HANDLE >		m_hPlayerClassInfoHandles;
};

extern CAnticitizen_PlayerResource* g_Anticitizen_PR;

#endif // SDK_PLAYER_RESOURCE_H
