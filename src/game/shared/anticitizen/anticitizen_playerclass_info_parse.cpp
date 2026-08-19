//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#include "cbase.h"
#include "anticitizen_playerclass_info_parse.h"

CAnticitizen_FilePlayerClassInfo_t::CAnticitizen_FilePlayerClassInfo_t()
{
	bAllWeapons = false;
	bSuit = false;
	bADSWeapons = false;
	bCanThrowGrenade = false;
	// all classes can sprint for the moment.
	bCanSprint = true;
	bSilentFootsteps = false;
	bOverlay = false;
	bLivesNoOverride = false;
	m_szCArmModel[0] = 0;
	iCArmSkin = 0;
	iMovementType = 0;
	iClassType = CLS_TYPE_NONE;
	flNormSpeed = DEFAULT_NORM_SPEED;
	flSprintSpeed = DEFAULT_SPRINT_SPEED;
	szPrimaryWeapon[0] = 0;
	szSecondaryWeapon[0] = 0;
	szTertiaryWeapon[0] = 0;
}

void CAnticitizen_FilePlayerClassInfo_t::Parse(KeyValues* pKeyValuesData, const char* szClassName)
{
	BaseClass::Parse(pKeyValuesData, szClassName);

	Q_strncpy(m_szCArmModel, pKeyValuesData->GetString("CArmsModel", "!! Missing CArmsModel on Player Class"), MAX_PLAYERCLASS_NAME_LENGTH);
	iCArmSkin = pKeyValuesData->GetInt("CArmsSkin", 0);

	iHealth = pKeyValuesData->GetInt("Health", 100);
	iLives = pKeyValuesData->GetInt("Lives", -1);
	iClassType = pKeyValuesData->GetInt("ClassType", CLS_TYPE_NONE);
	iSentenceVoice = pKeyValuesData->GetInt("VoiceMode", VOICE_TYPE_NONE);
	bAllWeapons = pKeyValuesData->GetBool("AllWeapons");
	// just so gordon's movemnt works with this new system.
	iMovementType = pKeyValuesData->GetInt("MovementType", MOVE_TYPE_NONE);
	bSuit = pKeyValuesData->GetBool("HEVSuit");
	bTwoHandedWeaponAnims = pKeyValuesData->GetBool("TwoHandedWeaponAnims");

	iGrenades = pKeyValuesData->GetInt("Grenades", 0);
	iCombineBalls = pKeyValuesData->GetInt("CombineBalls", 0);
	iManhacks = pKeyValuesData->GetInt("Manhacks", 0);
	iCrates = pKeyValuesData->GetInt("Crates", 0);
	iTurret = pKeyValuesData->GetInt("Turrets", 0);

	if (bSuit)
	{
		iSuitArmor = pKeyValuesData->GetInt("HEVSuitArmor", 0);
	}

	bADSWeapons = pKeyValuesData->GetBool("CanADSOfWeapons");
	bCanThrowGrenade = pKeyValuesData->GetBool("CanThrowGrenade");
	bCanSprint = pKeyValuesData->GetBool("CanSprint", true);
	bSilentFootsteps = pKeyValuesData->GetBool("SilentFootsteps");
	bOverlay = pKeyValuesData->GetBool("CombineOverlay");
	bHasEyeEffect = pKeyValuesData->GetBool("HasEyeEffect");
	bLivesNoOverride = pKeyValuesData->GetBool("LivesNoOverride");
	flNormSpeed = pKeyValuesData->GetFloat("Speed", DEFAULT_NORM_SPEED);
	flSprintSpeed = pKeyValuesData->GetFloat("SprintSpeed", DEFAULT_SPRINT_SPEED);
	flADSSpeed = pKeyValuesData->GetFloat("ADSSpeed", DEFAULT_ADS_SPEED);

	if (!bAllWeapons)
	{
		// Classname
		Q_strncpy(szPrimaryWeapon, pKeyValuesData->GetString("PrimaryWeapon", ""), MAX_WEAPON_STRING);
		// Classname
		Q_strncpy(szSecondaryWeapon, pKeyValuesData->GetString("SecondaryWeapon", ""), MAX_WEAPON_STRING);
		// Classname
		Q_strncpy(szTertiaryWeapon, pKeyValuesData->GetString("TertiaryWeapon", ""), MAX_WEAPON_STRING);
	}
}

FilePlayerClassInfo_t *CreatePlayerClassInfo()
{
	return new CAnticitizen_FilePlayerClassInfo_t();
}
