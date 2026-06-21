//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Rich Presence support
//
//=====================================================================================//

#include "cbase.h"
#include "anticitizen_tips.h"
#include "tier3/tier3.h"
#include "vgui/ILocalize.h"
#include "cdll_util.h"
#include "fmtstr.h"
#include "filesystem.h"
#include "c_hl2mp_player.h"

//-----------------------------------------------------------------------------
// Purpose: constructor
//-----------------------------------------------------------------------------
CAnticitizenTips::CAnticitizenTips() : CAutoGameSystem( "CAnticitizenTips" )
{
	Q_memset( m_iTipCount, 0, sizeof( m_iTipCount ) );
	m_iTipCountAllClasses = 0;
	m_iCurrentClassTip = 0;
	m_bInited = false;
}

//-----------------------------------------------------------------------------
// Purpose: Initializer
//-----------------------------------------------------------------------------
bool CAnticitizenTips::Init()
{
	if ( !m_bInited )
	{
		// count how many tips there are for each class and in total
		m_iTipCountAllClasses = 0;
		for ( int iClass = CLS_FIRST_PLAYER_CLASS; iClass <= CLS_LAST_PLAYER_CLASS; iClass++ )
		{
			// tip count per class is stored in resource file
			wchar_t *wzTipCount = g_pVGuiLocalize->Find( CFmtStr( "Tip_%d_Count", iClass ) );
			int iClassTipCount = wzTipCount ? _wtoi( wzTipCount ) : 0;
			m_iTipCount[iClass] = iClassTipCount;
			m_iTipCountAllClasses += iClassTipCount;
		}

		wchar_t* wzGeneralTipCount = g_pVGuiLocalize->Find("Tip_General_Count");
		int iGeneralTipCount = wzGeneralTipCount ? _wtoi(wzGeneralTipCount) : 0;
		m_iTipCountGeneral = iGeneralTipCount;

		m_bInited = true;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Returns a random tip, selected from tips for all classes, 
//          fills in iClassUsed with the class the tip is for
//-----------------------------------------------------------------------------
const wchar_t *CAnticitizenTips::GetRandomTip( int &iClassUsed )
{
	Init();

	int abuseHintChance = 33;

	if (RandomInt(1, 100) <= abuseHintChance)
	{
		int iTip = RandomInt(0, m_iTipCountGeneral - 1);

		Assert(iTip >= 0);

		if (iTip < m_iTipCountGeneral)
		{
			iClassUsed = RandomInt(CLS_FIRST_PLAYER_CLASS, CLS_LAST_PLAYER_CLASS);

			// return the tip
			return GetGeneralTip(iTip + 1);
		}
		iTip -= m_iTipCountGeneral;
	}
	else
	{
		// pick a random tip
		int iTip = RandomInt(0, m_iTipCountAllClasses - 1);
		// walk through each class until we find the class this tip lands in
		for (int iClass = CLS_FIRST_PLAYER_CLASS; iClass <= CLS_LAST_PLAYER_CLASS; iClass++)
		{
			Assert(iTip >= 0);
			int iClassTipCount = m_iTipCount[iClass];
			if (iTip < iClassTipCount)
			{
				iClassUsed = iClass;

				// return the tip
				return GetTip(iClass, iTip + 1);
			}
			iTip -= iClassTipCount;
		}
	}

	Assert( false );	// shouldn't hit this

	iClassUsed = CLS_INVALID;
	return L"";
}

//-----------------------------------------------------------------------------
// Purpose: Returns the next tip for specified class
//-----------------------------------------------------------------------------
const wchar_t *CAnticitizenTips::GetNextClassTip( int iClass )
{
	int iTipClass = CLS_INVALID;

	// OK to call this function with TF_CLASS_UNDEFINED or TF_CLASS_RANDOM, just return a random tip for any class in that case
	if ( iClass < CLS_FIRST_PLAYER_CLASS || iClass > CLS_LAST_PLAYER_CLASS )
		return GetRandomTip( iTipClass );
	
	int iClassTipCount = m_iTipCount[iClass];
	Assert( 0 != iClassTipCount );
	if ( 0 == iClassTipCount )
		return L"";
	// wrap the tip index to the valid range for this class
	if ( m_iCurrentClassTip >= iClassTipCount )
	{
		m_iCurrentClassTip %= iClassTipCount;
	}

	// return the tip
	const wchar_t *wzTip = GetTip( iClass, m_iCurrentClassTip+1 );
	m_iCurrentClassTip++;

	return wzTip;
}

const wchar_t* CAnticitizenTips::GetGeneralTip(int iTip)
{
	int iGeneralTipCount = m_iTipCountGeneral;
	Assert(0 != iGeneralTipCount);

	if (0 == iGeneralTipCount)
		return L"";

	// return the tip
	const wchar_t* wzFmt = g_pVGuiLocalize->Find(CFmtStr("#Tip_General_%d", iTip));
	static wchar_t wzTip[512] = L"";

	// replace any commands with their bound keys
	UTIL_ReplaceKeyBindings(wzFmt, 0, wzTip, sizeof(wzTip));

	return wzTip;
}

//-----------------------------------------------------------------------------
// Purpose: Returns specified tip index for specified class
//-----------------------------------------------------------------------------
const wchar_t *CAnticitizenTips::GetTip( int iClass, int iTip )
{
	static wchar_t wzTip[512] = L"";

	// get the tip
	const wchar_t *wzFmt = g_pVGuiLocalize->Find( CFmtStr( "#Tip_%d_%d", iClass, iTip ) );
	// replace any commands with their bound keys
	UTIL_ReplaceKeyBindings( wzFmt, 0, wzTip, sizeof( wzTip ) );

	return wzTip;
}

// global instance
CAnticitizenTips g_AnticitizenTips;

const wchar_t* UTIL_GetRandomTipUnicode(void)
{
	int iClassUsed;

	wchar_t wzTipLabel[1024] = L"";
	const wchar_t* wzTip = g_AnticitizenTips.GetRandomTip(iClassUsed);

	Assert(wzTip && wzTip[0]);

	g_pVGuiLocalize->ConstructString_safe(wzTipLabel, g_pVGuiLocalize->Find("#Tip_Fmt"), 1, wzTip);

	return wzTipLabel;
}

const char *UTIL_GetRandomTipANSI(void)
{
	const wchar_t* wzTip = UTIL_GetRandomTipUnicode();

	Assert(wzTip && wzTip[0]);

	char szTipLabel[1024];
	g_pVGuiLocalize->ConvertUnicodeToANSI(wzTip, szTipLabel, sizeof(szTipLabel));

	return szTipLabel;
}

void TipCommand(void)
{
	Msg("%s\n", UTIL_GetRandomTipANSI());
}

ConCommand tip("tip", TipCommand, "", FCVAR_CLIENTDLL);