#include "cbase.h"
#include "mapadd.h"
#include "filesystem.h"
#include "gameweaponmanager.h"
//#include "weapon_custom.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CC_CallLabel(const CCommand &args)
{
	CMapAdd *pMapadd = GetMapAddEntity();

	char szMapadd[128];
	Q_snprintf(szMapadd, sizeof(szMapadd), "mapadd/%s.txt", STRING(gpGlobals->mapname));
	if (!pMapadd)
	{
		pMapadd = CreateMapAddEntity();
		pMapadd->RunLabel(args[1]);
	}
	else
	{
		pMapadd->RunLabel(args[1]);
	}
}
static ConCommand mapadd_runlabel("mapadd_runlabel", CC_CallLabel, "Run a Mapadd label. Useful for testing.\n");

static CMapAdd *g_MapAddEntity = NULL;

LINK_ENTITY_TO_CLASS( mapadd, CMapAdd  );

BEGIN_DATADESC(CMapAdd)
// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_STRING, "RunLabel", InputRunLabel),
END_DATADESC()

CMapAdd* GetMapAddEntity()
{
	return g_MapAddEntity;
}

CMapAdd* CreateMapAddEntity()
{
	return dynamic_cast<CMapAdd*>((CBaseEntity*)CBaseEntity::Create("mapadd", Vector(0,0,0), QAngle(0,0,0)));
}

void CMapAdd::Precache( void )
{
	g_MapAddEntity = this;
}

bool CMapAdd::RunLabel( const char *szLabel)
{
	if(!szLabel || AllocPooledString(szLabel) == AllocPooledString(""))
		return false; //Failed to load!

	char szMapadd[128];
	Q_snprintf(szMapadd, sizeof(szMapadd), "scripts/mapadd/%s.txt", STRING(gpGlobals->mapname));

	KeyValues* pInfo = new KeyValues("MapAdd");

	if (pInfo->LoadFromFile(filesystem, szMapadd))
	{
		for (auto pMapAddEnt = pInfo->GetFirstSubKey(); pMapAddEnt != NULL; pMapAddEnt = pMapAddEnt->GetNextKey())
		{
			const char* kvLabel = pMapAddEnt->GetString("label", "Init");

			if (AllocPooledString(kvLabel) != AllocPooledString("") && 
				AllocPooledString(kvLabel) == AllocPooledString(szLabel))
			{
				const char* kvEntName = pMapAddEnt->GetString("entity", "");
				if (AllocPooledString(kvEntName) != AllocPooledString("") && 
					!HandlePlayerEntity(pMapAddEnt, false) && 
					!HandleRemoveEnitity(pMapAddEnt) && 
					!HandleSpecialEnitity(pMapAddEnt))
				{
					Vector SpawnVector = Vector(0, 0, 0);
					QAngle SpawnAngle = QAngle(0, 0, 0);

					SpawnVector.x = pMapAddEnt->GetFloat("x", SpawnVector.x);
					SpawnVector.y = pMapAddEnt->GetFloat("y", SpawnVector.y);
					SpawnVector.z = pMapAddEnt->GetFloat("z", SpawnVector.z);

					SpawnAngle[PITCH] = pMapAddEnt->GetFloat("pitch", SpawnAngle[PITCH]);
					SpawnAngle[YAW] = pMapAddEnt->GetFloat("yaw", SpawnAngle[YAW]);
					SpawnAngle[ROLL] = pMapAddEnt->GetFloat("roll", SpawnAngle[ROLL]);

					CBaseEntity* createEnt = CBaseEntity::CreateNoSpawn(kvEntName, SpawnVector, SpawnAngle);
					KeyValues* pEntKeyValues = pMapAddEnt->FindKey("KeyValues");
					KeyValues* pEntFlags = pMapAddEnt->FindKey("Flags");
					if (createEnt)
					{
						if (pEntKeyValues)
						{
							DevMsg("KeyValue for %s Found!\n", kvEntName);
							KeyValues* pEntKeyValuesAdd = pEntKeyValues->GetFirstValue();
							while (pEntKeyValuesAdd && createEnt)
							{
								if (AllocPooledString(pEntKeyValuesAdd->GetName()) == AllocPooledString("model"))
								{
									PrecacheModel(pEntKeyValuesAdd->GetString(""));
									createEnt->SetModel(pEntKeyValuesAdd->GetString(""));
								}
								else
								{
									createEnt->KeyValue(pEntKeyValuesAdd->GetName(), pEntKeyValuesAdd->GetString(""));
								}
								pEntKeyValuesAdd = pEntKeyValuesAdd->GetNextValue();
							}
						}

						if (pEntFlags)
						{
							DevMsg("Flag for %s Found!\n", kvEntName);
							KeyValues* pEntFlagsAdd = pEntFlags->GetFirstValue();
							while (pEntFlagsAdd && createEnt)
							{
								createEnt->AddSpawnFlags(pEntFlagsAdd->GetInt());
								pEntFlagsAdd = pEntFlagsAdd->GetNextValue();
							}
						}
					}
					//createEnt->Activate();//Is this a good idea? Not sure!
					//createEnt->Spawn();
					DispatchSpawn(createEnt); //I derped
				}
			}
			else
			{
				continue;
			}
		}
	}

	return true;
}

bool CMapAdd::HandlePlayerEntity( KeyValues *playerEntityKV, bool initLevel)
{
	const char* kvEntName = playerEntityKV->GetString("entity", "");
	if(AllocPooledString(kvEntName) == AllocPooledString("player"))
	{
		CBasePlayer *playerEnt = UTIL_GetLocalPlayer();
		if(!playerEnt) //He doesn't exist, just pretend
			return true;
		Vector SpawnVector = playerEnt->GetAbsOrigin();
		QAngle SpawnAngle = playerEnt->GetAbsAngles();
			
		SpawnVector.x = playerEntityKV->GetFloat("x", SpawnVector.x);
		SpawnVector.y = playerEntityKV->GetFloat("y", SpawnVector.y);
		SpawnVector.z = playerEntityKV->GetFloat("z", SpawnVector.z);

		SpawnAngle[PITCH] = playerEntityKV->GetFloat("pitch", SpawnAngle[PITCH]);
		SpawnAngle[YAW] = playerEntityKV->GetFloat("yaw", SpawnAngle[YAW]);
		SpawnAngle[ROLL] = playerEntityKV->GetFloat("roll", SpawnAngle[ROLL]);

		playerEnt->SetAbsOrigin(SpawnVector);
		playerEnt->SetAbsAngles(SpawnAngle);
		return true;
	}
	return false;
}

bool CMapAdd::HandleSpecialEnitity( KeyValues *specialEntity)
{
	//weaponmanagers are moved to HandleSpecialEntity
	const char* kvEntName = specialEntity->GetString("entity", "");
	if (AllocPooledString(kvEntName) == AllocPooledString("gameweaponmanager"))
	{
		const char* pWeaponName = "";
		int iMaxAllowed = 0;
		pWeaponName = specialEntity->GetString("weaponname", pWeaponName);
		iMaxAllowed = specialEntity->GetFloat("maxallowed", iMaxAllowed);
		CreateWeaponManager(pWeaponName, iMaxAllowed);
		return true;
	}
	return false;
}

bool CMapAdd::HandleRemoveEnitity( KeyValues *mapaddValue)
{
	const char* kvEntName = mapaddValue->GetString("entity", "");
	const char* pClassname = mapaddValue->GetString("remove_classname");
	const char* pTargetname = mapaddValue->GetString("remove_targetname");

	if (AllocPooledString(kvEntName) == AllocPooledString("remove_sphere"))
	{
		Vector RemoveVector = Vector(0,0,0);
		CBaseEntity *ppEnts[256];
		RemoveVector.x = mapaddValue->GetFloat("x", RemoveVector.x);
		RemoveVector.y = mapaddValue->GetFloat("y", RemoveVector.y);
		RemoveVector.z = mapaddValue->GetFloat("z", RemoveVector.z);
		int nEntCount = UTIL_EntitiesInSphere( ppEnts, 256, RemoveVector, mapaddValue->GetFloat("radius", 0), 0 );

		for (int i = 0; i < nEntCount; ++i)
		{
			if (ppEnts[i] == NULL)
				continue;

			if (AllocPooledString(pClassname) != AllocPooledString("") && 
				AllocPooledString(pClassname) == AllocPooledString(ppEnts[i]->GetClassname()))
			{
				UTIL_Remove(ppEnts[i]);
				continue;
			}
			else if (AllocPooledString(pTargetname) != AllocPooledString("") && 
					AllocPooledString(pTargetname) == ppEnts[i]->GetEntityName())
			{
				UTIL_Remove(ppEnts[i]);
				continue;
			}
		}
		
		return true;
	}
	else if ( AllocPooledString(kvEntName) == AllocPooledString("remove_all"))
	{
		if (AllocPooledString(pClassname) != AllocPooledString(""))
		{
			CBaseEntity* pent = NULL;
			while ((pent = gEntList.FindEntityByClassname(pent, STRING(AllocPooledString(pClassname)))) != 0)
				UTIL_Remove(pent);
		}
		else if (AllocPooledString(pTargetname) != AllocPooledString(""))
		{
			CBaseEntity* pent = NULL;
			while ((pent = gEntList.FindEntityByName(pent, STRING(AllocPooledString(pTargetname)))) != 0)
				UTIL_Remove(pent);
		}
		
		return true;
	}
	else
	{
		return false;
	}
}

void CMapAdd::InputRunLabel( inputdata_t &inputData ) //Input this directly!
{
	char szMapadd[128];
	Q_snprintf( szMapadd, sizeof( szMapadd ), "mapadd/%s.txt", STRING(gpGlobals->mapname) );
	this->RunLabel( inputData.value.String());
}
 