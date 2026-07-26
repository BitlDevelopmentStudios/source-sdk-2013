#include "cbase.h"
#include "actual_bullet.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar debug_actual_bullet_path("debug_actual_bullet_path", "0", FCVAR_GAMEDLL);
ConVar debug_actual_bullet_proj("debug_actual_bullet_proj", "0", FCVAR_GAMEDLL);

LINK_ENTITY_TO_CLASS(actual_bullet, CActualBullet);

BEGIN_DATADESC(CActualBullet)
END_DATADESC()

CActualBullet::CActualBullet(void)
{
	m_ImpactEffect = false;
	m_Line = false;
	m_LineTracerInfo.color = Color(255.0f, 255.0f, 255.0f);
	m_LineTracerInfo.speed = 0.05f;
}

void CActualBullet::Start(void)
{
	SetThink(&CActualBullet::Think);
	SetNextThink(gpGlobals->curtime);
	SetOwnerEntity(info.m_pAttacker);
}

void CActualBullet::Think(void)
{
	SetNextThink(gpGlobals->curtime + 0.05f);
	Vector vecStart;
	Vector vecEnd;
	float flInterval;

	flInterval = gpGlobals->curtime - GetLastThink();
	vecStart = GetAbsOrigin();
	vecEnd = vecStart + (m_vecDir * (m_Speed * flInterval));
	float flDist = (vecStart - vecEnd).Length();

	trace_t tr;
	UTIL_TraceLine(vecStart, vecEnd, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	if (debug_actual_bullet_proj.GetBool())
	{
		DebugDrawLine(vecStart, vecEnd, 
					  0, 
					  128, 
					  255, 
					  false, 
					  0.1f);
	}
	else
	{
		if (m_Line)
		{
			DebugDrawLine(vecStart, vecEnd,
						  m_LineTracerInfo.color.r(),
						  m_LineTracerInfo.color.g(),
						  m_LineTracerInfo.color.b(),
						  false,
						  m_LineTracerInfo.speed);
		}
	}

	if (tr.fraction != 1.0)
	{
		FireBulletsInfo_t info2;
		info2.m_iShots = 1;
		info2.m_vecSrc = vecStart;
		info2.m_vecSpread = vec3_origin;
		info2.m_vecDirShooting = m_vecDir;
		info2.m_flDistance = flDist;
		info2.m_iAmmoType = info.m_iAmmoType;
		info2.m_iTracerFreq = 0;
		info2.m_pAttacker = GetOwnerEntity();
		FireBullets(info2);
		SetThink(NULL);
		UTIL_Remove(this);
	}
	else
	{
		SetAbsOrigin(vecEnd);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CActualBullet::DoImpactEffect(trace_t& tr, int nDamageType)
{
	if (m_ImpactEffect)
	{
		CEffectData data;

		data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
		data.m_vNormal = tr.plane.normal;

		DispatchEffect(m_ImpactEffectName, data);
	}

	BaseClass::DoImpactEffect(tr, nDamageType);
}