#ifndef ACTUALBULLET_H
#define ACTUALBULLET_H

#include "cbase.h"
#include "baseentity.h"
#include "baseentity_shared.h"
#include "baseanimating.h"

struct LineTracerInfo_t
{
	Color color;
	float speed;
};

class CActualBullet : public CBaseAnimating
{
	DECLARE_CLASS(CActualBullet, CBaseAnimating);
	DECLARE_DATADESC();

public:
	CActualBullet(void);

	void Start(void);
	void Precache(void);
	void Think(void);
	void DoImpactEffect(trace_t& tr, int nDamageType);

public:
	Vector m_vecDir;
	int m_Speed;
	FireBulletsInfo_t info;
	bool m_ImpactEffect;
	const char* m_ImpactEffectName;
	bool m_Line;
	LineTracerInfo_t m_LineTracerInfo;
	bool m_Model;
	const char* m_ModelName;
};

extern ConVar debug_actual_bullet_path;

///so this is the actual bullet creation function.
inline void FireActualBullet(FireBulletsInfo_t &info, 
								int iSpeed, 
								const char* tracertype, 
								bool bWhiz = false, 
								bool bImpactEffect = false, 
								const char *szImpactEffectName = "",
								bool bLine = false, 
								Color cLineColor = Color(255.0f, 255.0f, 255.0f),
								float flLineSpeed = 0.05f, 
								bool bModel = false, 
								const char* szModelName = "" )
{
	if (!info.m_pAttacker)
	{
		Warning("ERROR: Firing an actual bullet without an attacker specified. This will crash the game without it. Cancelling.\n");
		return;
	}
	int iShots = info.m_iShots;

	for (int i = 0; i < iShots; i++)
	{
		Vector vecSpreadSrc = info.m_vecSpread;
		Vector vecSpread = Vector(RandomFloat(-vecSpreadSrc[0], vecSpreadSrc[0]), RandomFloat(-vecSpreadSrc[1], vecSpreadSrc[1]), RandomFloat(-vecSpreadSrc[2], vecSpreadSrc[2]));
		Vector vecShot = info.m_vecDirShooting + vecSpread;
		Vector vecShotDir = vecShot.Normalized();
		trace_t tr;
		UTIL_TraceLine(info.m_vecSrc, info.m_vecSrc + (vecShotDir * MAX_TRACE_LENGTH), MASK_SHOT, info.m_pAttacker, COLLISION_GROUP_NONE, &tr);

		if (debug_actual_bullet_path.GetBool())
		{
			DebugDrawLine(info.m_vecSrc, info.m_vecSrc + (vecShotDir * MAX_TRACE_LENGTH),
						  255,
						  0,
						  0,
						  false,
						  1.0f);
		}

		CActualBullet *pBullet = (CActualBullet*)CBaseEntity::Create("actual_bullet", info.m_vecSrc, vec3_angle, info.m_pAttacker);
		pBullet->m_vecDir = vecShotDir;
		pBullet->m_Speed = iSpeed;
		pBullet->m_ImpactEffect = bImpactEffect;
		pBullet->m_ImpactEffectName = szImpactEffectName;

		if (bLine)
		{
			pBullet->m_Line = true;
			pBullet->m_LineTracerInfo.color = cLineColor;
			pBullet->m_LineTracerInfo.speed = flLineSpeed;
		}

		if (bModel)
		{
			pBullet->m_Model = true;
			pBullet->m_ModelName = szModelName;
		}

		pBullet->SetOwnerEntity(info.m_pAttacker);
		pBullet->SetAbsOrigin(info.m_vecSrc);
		pBullet->SetAbsAngles(info.m_pAttacker->EyeAngles());
		pBullet->info = info;
		pBullet->Start();
		UTIL_Tracer(info.m_vecSrc, tr.endpos, info.m_pAttacker->entindex(), -1, (float)iSpeed, bWhiz, tracertype, 0);
	}
}

#endif //ACTUALBULLET_H