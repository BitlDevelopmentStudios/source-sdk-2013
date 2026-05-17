//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK CPlayerResource
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hl2mp_player.h"
#include "anticitizen_player_resource.h"
#include "hl2mp_gamerules.h"
#include "coordsize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CAnticitizen_PlayerResource* g_Anticitizen_PR;
const CAnticitizen_FilePlayerClassInfo_t pNullPlayerClassInfo = CAnticitizen_FilePlayerClassInfo_t();

// Datatable
IMPLEMENT_SERVERCLASS_ST(CAnticitizen_PlayerResource, DT_Anticitizen_PlayerResource)
	SendPropArray3( SENDINFO_ARRAY3(m_iPlayerClass), SendPropInt( SENDINFO_ARRAY(m_iPlayerClass), 4 ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CAnticitizen_PlayerResource )
	DEFINE_ARRAY( m_iPlayerClass, FIELD_INTEGER, MAX_PLAYERS+1 ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( anticitizen_player_manager, CAnticitizen_PlayerResource );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAnticitizen_PlayerResource::CAnticitizen_PlayerResource()
{
	g_Anticitizen_PR = this;

	//parse our classes
	int i = 0;
	while (pszCombineClasses[i] != NULL)
	{
		AddPlayerClass(pszCombineClasses[i]);
		i++;
	}

	i = 0;
	while (pszFreemanClasses[i] != NULL)
	{
		AddPlayerClass(pszFreemanClasses[i]);
		i++;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CAnticitizen_PlayerResource::~CAnticitizen_PlayerResource()
{
	g_Anticitizen_PR = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAnticitizen_PlayerResource::UpdatePlayerData( void )
{
	int i;

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = (CHL2MP_Player*)UTIL_PlayerByIndex( i );
		
		if ( pPlayer && pPlayer->IsConnected() )
		{
			m_iPlayerClass.Set( i, pPlayer->GetPlayerClass() );
		}
	}

	BaseClass::UpdatePlayerData();
}

void CAnticitizen_PlayerResource::Spawn( void )
{
	int i;

	for ( i=0; i < MAX_PLAYERS+1; i++ )
	{
		m_iPlayerClass.Set( i, PLAYERCLASS_UNDEFINED );
	}

	BaseClass::Spawn();
}

void CAnticitizen_PlayerResource::AddPlayerClass(const char* szClassName)
{
	PLAYERCLASS_FILE_INFO_HANDLE hPlayerClassInfo;

	if (ReadPlayerClassDataFromFileForSlotEx(filesystem, szClassName, &hPlayerClassInfo, "scripts/classes/playerclass_%s", HL2MPRules()->GetEncryptionKey()))
	{
		m_hPlayerClassInfoHandles.AddToTail(hPlayerClassInfo);
	}
	else
	{
		Assert(!"missing playerclass script file");
		Msg("Missing playerclass script file for class: %s\n", szClassName);
	}
}

const CAnticitizen_FilePlayerClassInfo_t& CAnticitizen_PlayerResource::GetPlayerClassInfo(int iPlayerClass) const
{
	Assert(iPlayerClass >= 0 && iPlayerClass < m_hPlayerClassInfoHandles.Count());

	if (iPlayerClass < 0)
	{
		return pNullPlayerClassInfo;
	}

	const FilePlayerClassInfo_t* pPlayerClassInfo = GetFilePlayerClassInfoFromHandle(m_hPlayerClassInfoHandles[iPlayerClass]);
	const CAnticitizen_FilePlayerClassInfo_t* pSDKInfo;

#ifdef _DEBUG
	pSDKInfo = dynamic_cast<const CAnticitizen_FilePlayerClassInfo_t*>(pPlayerClassInfo);
	Assert(pSDKInfo);
#else
	pSDKInfo = static_cast<const CAnticitizen_FilePlayerClassInfo_t*>(pPlayerClassInfo);
#endif

	return *pSDKInfo;
}
