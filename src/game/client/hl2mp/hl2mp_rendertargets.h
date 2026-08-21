//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//===========================================================================//
#ifndef HL2MP_RENDERTARGETS_H
#define HL2MP_RENDERTARGETS_H
#ifdef _WIN32
#pragma once
#endif

#include "baseclientrendertargets.h" // Base class, with interfaces called by engine and inherited members to init common render targets

// externs
class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CHL2MPRenderTargets : public CBaseClientRenderTargets
{
	// no networked vars
	DECLARE_CLASS_GAMEROOT( CHL2MPRenderTargets, CBaseClientRenderTargets );
public:
	virtual void InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig );
	virtual void ShutdownClientRenderTargets();
};

extern CHL2MPRenderTargets* g_pHL2MPRenderTargets;


#endif // TF_RENDERTARGETS_H
