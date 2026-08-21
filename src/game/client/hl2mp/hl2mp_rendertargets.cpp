//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: L4D mod render targets are specified by and accessable through this singleton
//
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include "hl2mp_rendertargets.h"
#include "materialsystem/imaterialsystem.h"
#include "rendertexture.h"

ConVar hl2mp_water_resolution( "hl2mp_water_resolution", "1024", FCVAR_NONE, "Needs to be set at game launch time to override." );
ConVar hl2mp_monitor_resolution( "hl2mp_monitor_resolution", "1024", FCVAR_NONE, "Needs to be set at game launch time to override." );

//-----------------------------------------------------------------------------
// Purpose: InitClientRenderTargets, interface called by the engine at material system init in the engine
// Input  : pMaterialSystem - the interface to the material system from the engine (our singleton hasn't been set up yet)
//			pHardwareConfig - the user's hardware config, useful for conditional render targets setup
//-----------------------------------------------------------------------------
void CHL2MPRenderTargets::InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig )
{
	BaseClass::InitClientRenderTargets( pMaterialSystem, pHardwareConfig, hl2mp_water_resolution.GetInt(), hl2mp_monitor_resolution.GetInt() );
}

//-----------------------------------------------------------------------------
// Purpose: Shutdown client render targets. This gets called during shutdown in the engine
// Input  :  - 
//-----------------------------------------------------------------------------
void CHL2MPRenderTargets::ShutdownClientRenderTargets()
{
	BaseClass::ShutdownClientRenderTargets();
}

static CHL2MPRenderTargets g_HL2MPRenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CHL2MPRenderTargets, IClientRenderTargets, 
	CLIENTRENDERTARGETS_INTERFACE_VERSION, g_HL2MPRenderTargets );
CHL2MPRenderTargets* g_pHL2MPRenderTargets = &g_HL2MPRenderTargets;