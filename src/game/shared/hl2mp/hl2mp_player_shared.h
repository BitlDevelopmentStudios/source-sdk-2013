//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef HL2MP_PLAYER_SHARED_H
#define HL2MP_PLAYER_SHARED_H
#pragma once

#define HL2MP_PUSHAWAY_THINK_INTERVAL		(1.0f / 20.0f)
#include "studio.h"
enum EClasses
{
	// rand/invalid classes
	CLS_RAND = -2,
	CLS_INVALID = -1,

	// combine classes
	CLS_METROPOLICE,
	CLS_COMBINE_SOLDIER,
	CLS_COMBINE_SHOTGUNNER,
	CLS_COMBINE_ELITE,
	CLS_COMBINE_ASSASSIN,

	// first/last estimators for arrays/random
	CLS_FIRST_COMBINE_CLASS = CLS_METROPOLICE,
	CLS_LAST_COMBINE_CLASS = CLS_COMBINE_ASSASSIN,

	// rebel classes
	CLS_FREEMAN = CLS_LAST_COMBINE_CLASS + 1,

	CLS_FIRST_PLAYER_CLASS = CLS_FIRST_COMBINE_CLASS,
	CLS_LAST_PLAYER_CLASS = CLS_FREEMAN
};

extern const char* pszCombineClasses[];
extern const char* pszFreemanClasses[];

enum
{
	PLAYER_SOUNDS_CITIZEN = 0,
	PLAYER_SOUNDS_COMBINESOLDIER,
	PLAYER_SOUNDS_METROPOLICE,
	PLAYER_SOUNDS_MAX,
};

enum HL2MPPlayerState
{
	// Happily running around in the game.
	STATE_ACTIVE=0,
	STATE_OBSERVER_MODE,		// Noclipping around, watching players, etc.
	NUM_PLAYER_STATES
};

#define PLAYERCLASS_UNDEFINED	-1

#if defined( CLIENT_DLL )
#define CHL2MP_Player C_HL2MP_Player
#endif

#endif //HL2MP_PLAYER_SHARED_h
