//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: SDK C_PlayerResource
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_anticitizen_player_resource.h"
#include "c_hl2mp_player.h"
#include "hl2mp_gamerules.h"
#include "shareddefs.h"
#include "hud.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_Anticitizen_PlayerResource* g_Anticitizen_PR;
const CAnticitizen_FilePlayerClassInfo_t pNullPlayerClassInfo = CAnticitizen_FilePlayerClassInfo_t();

IMPLEMENT_CLIENTCLASS_DT(C_Anticitizen_PlayerResource, DT_Anticitizen_PlayerResource, CAnticitizen_PlayerResource)
	RecvPropArray3( RECVINFO_ARRAY(m_iPlayerClass), RecvPropInt( RECVINFO(m_iPlayerClass[0]))),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_Anticitizen_PlayerResource::C_Anticitizen_PlayerResource()
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
C_Anticitizen_PlayerResource::~C_Anticitizen_PlayerResource()
{
	g_Anticitizen_PR = NULL;
}

int C_Anticitizen_PlayerResource::GetPlayerClass( int iIndex )
{
	if ( !IsConnected( iIndex ) )
		return PLAYERCLASS_UNDEFINED;

	return m_iPlayerClass[iIndex];
}

void C_Anticitizen_PlayerResource::AddPlayerClass(const char* szClassName)
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

const CAnticitizen_FilePlayerClassInfo_t& C_Anticitizen_PlayerResource::GetPlayerClassInfo(int iPlayerClass) const
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

