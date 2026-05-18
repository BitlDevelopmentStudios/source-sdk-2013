//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#include "cbase.h"
#include "anticitizen_playerclass_info_parse.h"

const char* pWeaponCategories[NUM_WEAPON_TYPES] =
{
	"empty",
	"primary",
	"secondary",
	"melee"
};

CAnticitizen_FilePlayerClassInfo_t::CAnticitizen_FilePlayerClassInfo_t()
{
	bAllWeapons = false;
	bSPMovement = false;
	bSuit = false;
	bADSWeapons = false;
	bNoFiringWhileSprinting = false;
	iClassType = CLS_TYPE_NONE;
	flNormSpeed = DEFAULT_NORM_SPEED;
	flSprintSpeed = DEFAULT_SPRINT_SPEED;
	szPrimaryWeapon[0] = 0;
	szSecondaryWeapon[0] = 0;
	szMeleeWeapon[0] = 0;
}

void CAnticitizen_FilePlayerClassInfo_t::Parse(KeyValues* pKeyValuesData, const char* szClassName)
{
	BaseClass::Parse(pKeyValuesData, szClassName);

	iHealth = pKeyValuesData->GetInt("Health", 100);
	iClassType = pKeyValuesData->GetInt("ClassType", CLS_TYPE_NONE);
	iSentenceVoice = pKeyValuesData->GetInt("VoiceMode", VOICE_TYPE_NONE);
	bAllWeapons = pKeyValuesData->GetBool("AllWeapons");
	bSPMovement = pKeyValuesData->GetBool("SPMovement");
	bSuit = pKeyValuesData->GetBool("HEVSuit");
	bNoFiringWhileSprinting = pKeyValuesData->GetBool("NoFiringWhileSprinting");

	if (bSuit)
	{
		iSuitArmor = pKeyValuesData->GetInt("HEVSuitArmor", 0);
	}

	bADSWeapons = pKeyValuesData->GetBool("CanADSOfWeapons");
	flNormSpeed = pKeyValuesData->GetFloat("Speed", DEFAULT_NORM_SPEED);
	flSprintSpeed = pKeyValuesData->GetFloat("SprintSpeed", DEFAULT_SPRINT_SPEED);

	if (!bAllWeapons)
	{
		// Classname
		Q_strncpy(szPrimaryWeapon, pKeyValuesData->GetString("PrimaryWeapon", ""), MAX_WEAPON_STRING);
		// Classname
		Q_strncpy(szSecondaryWeapon, pKeyValuesData->GetString("SecondaryWeapon", ""), MAX_WEAPON_STRING);
		// Classname
		Q_strncpy(szMeleeWeapon, pKeyValuesData->GetString("MeleeWeapon", ""), MAX_WEAPON_STRING);
	}
}

FilePlayerClassInfo_t *CreatePlayerClassInfo()
{
	return new CAnticitizen_FilePlayerClassInfo_t();
}
