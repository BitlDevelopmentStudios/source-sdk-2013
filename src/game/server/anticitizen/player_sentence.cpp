//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player_sentence.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar player_sentences( "player_sentences", "1" );

//-----------------------------------------------------------------------------
// Save/load
//-----------------------------------------------------------------------------
BEGIN_SIMPLE_DATADESC(CPlayer_SentenceBase)
	DEFINE_FIELD( m_voicePitch, FIELD_INTEGER ),
	DEFINE_FIELD( m_nQueuedSentenceIndex, FIELD_INTEGER ),
	DEFINE_FIELD( m_flQueueTimeout, FIELD_TIME ),
	DEFINE_FIELD( m_nQueueSoundPriority, FIELD_INTEGER ),
END_DATADESC();


//-----------------------------------------------------------------------------
// Speech
//-----------------------------------------------------------------------------
CPlayer_SentenceBase::CPlayer_SentenceBase()
{
	m_bInit = false;
	ClearQueue();
}

//-----------------------------------------------------------------------------
// Debug output
//-----------------------------------------------------------------------------
void CPlayer_SentenceBase::SentenceMsg( const char *pStatus, const char *pSentence )
{
	int nMode = player_sentences.GetInt();
	switch( nMode )
	{
	case 0:
		return;

	case 1:
		DevMsg( "SENTENCE [%d %.2f] %s: %s\n", GetOuter()->entindex(), gpGlobals->curtime, pStatus, pSentence );
		break;
	}
}


//-----------------------------------------------------------------------------
// Check for queued-up-sentences + speak them
//-----------------------------------------------------------------------------
void CPlayer_SentenceBase::ClearQueue()
{
	m_nQueuedSentenceIndex = -1;
}


//-----------------------------------------------------------------------------
// Check for queued-up-sentences + speak them
//-----------------------------------------------------------------------------
void CPlayer_SentenceBase::UpdateSentenceQueue()
{
	if (!m_bInit)
		return;

	if ( m_nQueuedSentenceIndex == -1 )
		return;

	// Check for timeout
	if ( m_flQueueTimeout < gpGlobals->curtime )
	{
		ClearQueue();
		return;
	}

	SENTENCEG_PlaySentenceIndex(GetOuter()->edict(), m_nQueuedSentenceIndex, GetVolume(), GetSoundLevel(), 0, GetVoicePitch());

	const char* pSentenceName = engine->SentenceNameFromIndex(m_nQueuedSentenceIndex);
	SentenceMsg("Speaking [from QUEUE]", pSentenceName);

	ClearQueue();
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
			direction = &pPlayer->BodyDirection3D();

			CBaseEntity* pEnt;
			// Get the entity under my crosshair
			extern CBaseEntity* FindPickerEntity(CBasePlayer * pPlayer);
			pEnt = FindPickerEntity(pPlayer);

			if (pEnt)
			{
				origin = &pEnt->GetAbsOrigin();
			}
		}

		CBaseEntity::EmitSentenceByIndex(filter, ENTINDEX(entity), CHAN_VOICE, sentenceIndex, volume, soundlevel, flags, pitch, origin, direction);
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
		SentenceMsg( "BOGUS", pSentence );
		return -1;
	}

	const char *pSentenceName = engine->SentenceNameFromIndex( nSentenceIndex ); 
	SentenceMsg( "Speaking", pSentenceName );
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

	// Speaking clears the queue
	ClearQueue();

	if ( nSoundPriority == SENTENCE_PRIORITY_INVALID )
	{
		return PlaySentence( pSentence );
	}

	int nSentenceIndex = PlaySentence(pSentence);
	return nSentenceIndex;
}


//-----------------------------------------------------------------------------
// Speech w/ queue
//-----------------------------------------------------------------------------
int CPlayer_SentenceBase::SpeakQueued( const char *pSentence, SentencePriority_t nSoundPriority, SentenceCriteria_t nCriteria )
{
	if (!m_bInit)
		return -1;

	if ( !MatchesCriteria(nCriteria) )
		return -1;

	// Speaking clears the queue
	ClearQueue();

	int nSentenceIndex = Speak( pSentence, nSoundPriority, nCriteria );
	if ( nSentenceIndex >= 0 )
		return nSentenceIndex;
	
	// Queue up the sentence for later playing 
	int nQueuedSentenceIndex = SENTENCEG_PickRndSz( pSentence );
	if ( nQueuedSentenceIndex == -1 )
		return -1;

	m_flQueueTimeout = gpGlobals->curtime + 2.0f;
	m_nQueueSoundPriority = nSoundPriority;
	m_nQueuedSentenceIndex = nQueuedSentenceIndex;
	return -1;
}


	

	
