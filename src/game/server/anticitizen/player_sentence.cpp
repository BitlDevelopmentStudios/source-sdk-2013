//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player_sentence.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar player_sentences( "player_sentences", "1" );

//-----------------------------------------------------------------------------
// Save/load
//-----------------------------------------------------------------------------
BEGIN_SIMPLE_DATADESC(CPlayer_SentenceBase)
	DEFINE_FIELD( m_voicePitch, FIELD_INTEGER ),
END_DATADESC();

//-----------------------------------------------------------------------------
// Speech
//-----------------------------------------------------------------------------
CPlayer_SentenceBase::CPlayer_SentenceBase()
{
	m_bInit = false;
}

static ConVar sentence_kvloader_debug("sentence_kvloader_debug", "1", FCVAR_REPLICATED);

void SentenceLenKeyValuesLoader::LoadEntries(const char* fileName, const char* kvHeader)
{
	KeyValues* pKV = new KeyValues(kvHeader);
	if (pKV->LoadFromFile(filesystem, fileName, "GAME"))
	{
		m_storedKV = pKV->MakeCopy();
	}

	if (sentence_kvloader_debug.GetBool())
	{
		KeyValuesDumpAsDevMsg(pKV, 1);
	}

	pKV->deleteThis();
}

//-----------------------------------------------------------------------------
// Debug output
//-----------------------------------------------------------------------------
void CPlayer_SentenceBase::SentenceMsg( const char *pStatus, const char *pSentence, int iIndex, float flLen)
{
	int nMode = player_sentences.GetInt();
	switch( nMode )
	{
	case 0:
		return;

	case 1:
		DevMsg( "SENTENCE %i [%d %.2f] %s: %s [len: %.2f]\n", iIndex, GetOuter()->entindex(), gpGlobals->curtime, pStatus, pSentence, flLen);
		break;
	}
}

//-----------------------------------------------------------------------------
// Speech criteria
//-----------------------------------------------------------------------------
bool CPlayer_SentenceBase::MatchesCriteria( SentenceCriteria_t nCriteria )
{
	// criteria is determined by player
	return true;
}

int SENTENCEG_PlayRndSz_Player_Lookat(edict_t* entity, const char* szgroupname,
	float volume, soundlevel_t soundlevel, int flags, int pitch)
{
	char name[64];
	int ipick;
	int isentenceg;

	name[0] = 0;

	isentenceg = engine->SentenceGroupIndexFromName(szgroupname);
	if (isentenceg < 0)
	{
		Warning("No such sentence group %s\n", szgroupname);
		return -1;
	}

	ipick = engine->SentenceGroupPick(isentenceg, name, sizeof(name));
	if (ipick >= 0 && name[0])
	{
		int sentenceIndex = SENTENCEG_Lookup(name);
		CPASAttenuationFilter filter(GetContainingEntity(entity), soundlevel);

		const Vector* origin = &vec3_origin;
		const Vector* direction = &vec3_origin;

		CBasePlayer* pPlayer = ToBasePlayer(GetContainingEntity(entity));
		if (pPlayer)
		{
			// if we have no ent, make something up.
			Vector forward, right, up;
			pPlayer->GetVectors(&forward, &right, &up);
			Vector vFakeTarget = (pPlayer->EyePosition() + forward * 128 + right * random->RandomFloat(-32, 32) + up * random->RandomFloat(-16, 16));
			origin = &vFakeTarget;

			// proper direction calc
			Vector vecDirection;
			AngleVectors(pPlayer->GetAbsAngles(), &vecDirection);
			direction = &vecDirection;

			CBaseEntity* pEnt;
			// Get the entity under my crosshair
			extern CBaseEntity* FindPickerEntity(CBasePlayer * pPlayer);
			pEnt = FindPickerEntity(pPlayer);

			if (pEnt)
			{
				origin = &pEnt->GetAbsOrigin();
			}
		}

		CBaseEntity::EmitSentenceByIndex(filter, ENTINDEX(entity), CHAN_VOICE, sentenceIndex, volume, soundlevel, flags, pitch, origin, direction, false);

		return sentenceIndex;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Play the actual sentence
//-----------------------------------------------------------------------------
int CPlayer_SentenceBase::PlaySentence( const char *pSentence )
{
	if (!m_bInit)
		return -1;

	int nSentenceIndex = SENTENCEG_PlayRndSz_Player_Lookat( GetOuter()->edict(), pSentence, GetVolume(), GetSoundLevel(), 0, GetVoicePitch());
	if ( nSentenceIndex < 0 )
	{
		SentenceMsg( "BOGUS", pSentence, -1, -1);
		return -1;
	}

	const char *pSentenceName = engine->SentenceNameFromIndex( nSentenceIndex ); 

	float length = 0.0f;

	KeyValues* sentence = m_kvSentenceLen.m_storedKV->FindKey(pSentenceName);
	if (sentence)
	{
		length = sentence->GetFloat();
	}

	m_curSentenceLength = length;

	SentenceMsg( "Speaking", pSentenceName, nSentenceIndex, m_curSentenceLength);
	return nSentenceIndex;
}

//-----------------------------------------------------------------------------
// Speech
//-----------------------------------------------------------------------------
int CPlayer_SentenceBase::Speak( const char *pSentence, SentencePriority_t nSoundPriority, SentenceCriteria_t nCriteria )
{
	if (!m_bInit)
		return -1;

	if ( !MatchesCriteria(nCriteria) )
		return -1;

	if ( nSoundPriority == SENTENCE_PRIORITY_INVALID )
	{
		return PlaySentence( pSentence );
	}

	int nSentenceIndex = PlaySentence(pSentence);
	return nSentenceIndex;
}


	

	
