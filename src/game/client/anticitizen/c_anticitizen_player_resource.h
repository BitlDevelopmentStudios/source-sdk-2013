//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK C_PlayerResource
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_ANTICITIZEN_PLAYER_RESOURCE_H
#define C_ANTICITIZEN_PLAYER_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "c_playerresource.h"
#include "anticitizen_playerclass_info_parse.h"

class C_Anticitizen_PlayerResource : public C_PlayerResource
{
	DECLARE_CLASS( C_Anticitizen_PlayerResource, C_PlayerResource );
public:
	DECLARE_CLIENTCLASS();

					C_Anticitizen_PlayerResource();
	virtual			~C_Anticitizen_PlayerResource();

	int GetPlayerClass( int iIndex );

	void AddPlayerClass(const char* szClassName);
	const CAnticitizen_FilePlayerClassInfo_t& GetPlayerClassInfo(int iPlayerClass) const;
	int GetNumPlayerClasses(void) { return m_hPlayerClassInfoHandles.Count(); }
	
protected:
	int		m_iPlayerClass[MAX_PLAYERS+1];

private:
	CUtlVector < PLAYERCLASS_FILE_INFO_HANDLE >		m_hPlayerClassInfoHandles;
};
C_Anticitizen_PlayerResource * AnticitizenGameResources( void );

#endif // C_Anticitizen_PlayerResource_H
