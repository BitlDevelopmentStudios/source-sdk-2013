//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#ifndef SDK_PLAYERCLASS_INFO_PARSE_H
#define SDK_PLAYERCLASS_INFO_PARSE_H
#ifdef _WIN32
#pragma once
#endif

#include "playerclass_info_parse.h"
#include "networkvar.h"

typedef enum {
	WPN_EMPTY,
	WPN_PRIMARY,
	WPN_SECONDARY,
	WPN_MELEE,

	// Add new weapon types here

	NUM_WEAPON_TYPES,
} WeaponTypes_t;

typedef enum {
	CLS_TYPE_NONE,
	CLS_TYPE_LOW_TIER,
	CLS_TYPE_MID_TIER,
	CLS_TYPE_HIGH_TIER,
	CLS_TYPE_FREEMAN,

	// Add new weapon types here

	NUM_CLASS_TYPES,
} ClassType_t;

#define	DEFAULT_NORM_SPEED 190
#define	DEFAULT_SPRINT_SPEED 320

//-----------------------------------------------------------------------------
// Purpose: Contains the data read from the player class script files. 
// It's cached so we only read each script file once.
// Each game provides a CreatePlayerClassInfo function so it can have game-specific
// data in the player class scripts.
//-----------------------------------------------------------------------------
class CAnticitizen_FilePlayerClassInfo_t : public FilePlayerClassInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT(CAnticitizen_FilePlayerClassInfo_t, FilePlayerClassInfo_t);

	CAnticitizen_FilePlayerClassInfo_t();
	virtual ~CAnticitizen_FilePlayerClassInfo_t() {}

	// Each game can override this to get whatever values it wants from the script.
	void Parse(KeyValues* pKeyValuesData, const char* szClassName) OVERRIDE;

public:
	// Class properties

	int						iHealth;
	int						iSuitArmor;

	int						iClassType;

	bool					bAllWeapons;
	bool					bSPMovement;
	bool					bSuit;
	bool					bADSWeapons;
	bool					bNoFiringWhileSprinting;

	float					flNormSpeed;
	float					flSprintSpeed;

	char					szPrimaryWeapon[MAX_WEAPON_STRING];
	char					szSecondaryWeapon[MAX_WEAPON_STRING];
	char					szMeleeWeapon[MAX_WEAPON_STRING];
};

#endif
