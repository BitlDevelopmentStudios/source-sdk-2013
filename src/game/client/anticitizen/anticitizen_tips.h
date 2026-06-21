//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TF implementation of the IPresence interface
//
//=============================================================================

#ifndef ANTICITIZEN_TIPS_H
#define ANTICITIZEN_TIPS_H
#ifdef _WIN32
#pragma once
#endif

#include "hl2mp_player_shared.h"

//-----------------------------------------------------------------------------
// Purpose: helper class for TF tips
//-----------------------------------------------------------------------------
class CAnticitizenTips : public CAutoGameSystem
{
public:
	CAnticitizenTips();

	virtual bool Init();
	virtual char const *Name() { return "CAnticitizenTips"; }

	const wchar_t *GetRandomTip( int &iClassUsed ); // iClassUsed will be filled in with the class that was selected
	const wchar_t *GetNextClassTip( int iClass );
	const wchar_t* GetGeneralTip(int iTip);

private:
	const wchar_t *GetTip( int iClass, int iTip );

	int m_iTipCount[CLS_LAST_PLAYER_CLASS+1];		// how many tips there are for each class
	int m_iTipCountGeneral;							// how many general tips exist
	int m_iTipCountAllClasses;						// how many tips there are total
	int m_iCurrentClassTip;							// index of current per-class tip
	bool m_bInited;									// have we been initialized
};

extern CAnticitizenTips g_AnticitizenTips;

extern const wchar_t* UTIL_GetRandomTipUnicode(void);
extern const char* UTIL_GetRandomTipANSI(void);
#endif // TF_TIPS_H
