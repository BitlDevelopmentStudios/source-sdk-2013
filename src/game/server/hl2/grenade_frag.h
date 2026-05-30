//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_FRAG_H
#define GRENADE_FRAG_H
#pragma once

#include "basegrenade_shared.h"
#include "grenade_frag.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "soundent.h"

class CBaseGrenade;
struct edict_t;

class CGrenadeFrag : public CBaseGrenade
{
	DECLARE_CLASS(CGrenadeFrag, CBaseGrenade);

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif

public:
	CGrenadeFrag(void);
	~CGrenadeFrag(void);

	void	Spawn(void);
	void	OnRestore(void);
	void	Precache(void);
	bool	CreateVPhysics(void);
	void	CreateEffects(void);
	void	SetTimer(float detonateDelay, float warnDelay);
	void	SetVelocity(const Vector& velocity, const AngularImpulse& angVelocity);
	int		OnTakeDamage(const CTakeDamageInfo& inputInfo);
	void	BlipSound() { EmitSound("Grenade.Blip"); }
	void	DelayThink();
	void	VPhysicsUpdate(IPhysicsObject* pPhysics);
	void	OnPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason);
	void	SetCombineSpawned(bool combineSpawned) { m_combineSpawned = combineSpawned; }
	bool	IsCombineSpawned(void) const { return m_combineSpawned; }
	void	SetPunted(bool punt) { m_punted = punt; }
	bool	WasPunted(void) const { return m_punted; }

	// this function only used in episodic.

	void	InputSetTimer(inputdata_t& inputdata);

protected:
	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;

	float	m_flNextBlipTime;
	bool	m_inSolid;
	bool	m_combineSpawned;
	bool	m_punted;
};

class CGrenadeManager
{
public:
	CGrenadeManager();

	CGrenadeFrag** AccessFrags();
	CGrenadeFrag* AccessFragByIndex(int index);
	int				NumFrags();

	void AddFrags(CGrenadeFrag* pFrag);
	void RemoveFrags(CGrenadeFrag* pFrag);

	bool FindFrag(CGrenadeFrag* pFrag) { return (m_FragGrenades.Find(pFrag) != m_FragGrenades.InvalidIndex()); }

private:
	enum
	{
		MAX_FRAGS = 256
	};

	typedef CUtlVector<CGrenadeFrag*> CFragArray;

	CFragArray m_FragGrenades;

};

extern CGrenadeManager g_GrenadeManager;

CBaseGrenade *Fraggrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned );
bool	Fraggrenade_WasPunted( const CBaseEntity *pEntity );
bool	Fraggrenade_WasCreatedByCombine( const CBaseEntity *pEntity );

#endif // GRENADE_FRAG_H
