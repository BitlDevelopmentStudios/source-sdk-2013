//======= Maestra Fenix, 2017 ==================================================//
//
// Purpose: Map load background panel
//
//==============================================================================//

#include "cbase.h"
#include "mapload_background.h"
#include "filesystem.h"
#include <vgui_controls/Label.h>
#include "anticitizen_tips.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapLoadBG::CMapLoadBG( char const *panelName ) : EditablePanel( NULL, panelName )
{
	VPANEL toolParent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	SetParent( toolParent );	

	m_pTipPanel = new Label(this, "TipText", "");
	m_pBackground = NULL;

	m_flLastTipChange = 0;

	ResizePanel();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapLoadBG::~CMapLoadBG()
{
	// None
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapLoadBG::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	ResizePanel();
}

void CMapLoadBG::ResizePanel(void)
{
	// Fenix: We load a RES file rather than create the element here for taking advantage of the "F" parameter for wide and tall
	// Is the sole thing that makes fill the background to the entire screen regardless of the texture size
	// Congratulations to Valve for once again give options to only one side and not both
	LoadControlSettings("resource/loadingdialogbackground.res");

	int iWide, iTall;
	surface()->GetScreenSize(iWide, iTall);
	SetSize(iWide, iTall);

	UpdateMainBackground();
}

//-----------------------------------------------------------------------------
// Purpose: Set the background image based on the current mode
//-----------------------------------------------------------------------------
void CMapLoadBG::UpdateMainBackground(void)
{
	m_pBackground = FindControl<ImagePanel>("LoadingImage", true);
	if (m_pBackground)
	{
		// determine if we're in widescreen or not and select the appropriate image
		int iWide, iTall;
		surface()->GetScreenSize(iWide, iTall);
		float aspectRatio = (float)iWide / (float)iTall;
		bool bIsWidescreen = aspectRatio >= 1.5999f;

		const char *newBG = SetRandBackgroundImage(bIsWidescreen);

		m_pBackground->SetImage(newBG);

		if (m_pTipPanel)
		{
			// change tip.
			m_pTipPanel->SetText(UTIL_GetRandomTipUnicode());

			// position the panel.
			int x, y;
			m_pTipPanel->GetPos(x, y);

			if (iWide <= 640)
			{
				y = 10;
			}
			else
			{
				// position it propertionately to the resolution, with a 15% offset applied.
				y = (y + (iTall - y) - (iTall * 0.15));
			}

			m_pTipPanel->SetPos(x, y);
		}
	}
}

const char *CMapLoadBG::SetRandBackgroundImage(bool bIsWidescreen)
{
	// pulled from the background music code
	char path[512];
	const char *szBGName = (bIsWidescreen ? "materials/vgui/loading/background*_widescreen.vmt" : "materials/vgui/loading/background*.vmt");
	Q_snprintf(path, sizeof(path), szBGName);
	Q_FixSlashes(path);
	CUtlVector<char*> fileNames;
	FileFindHandle_t fh;

	char const* fn = g_pFullFileSystem->FindFirstEx(path, "MOD", &fh);
	if (fn)
	{
		do
		{
			char ext[10];
			Q_ExtractFileExtension(fn, ext, sizeof(ext));

			if (!Q_stricmp(ext, "vmt"))
			{
				char temp[512];
				{
					Q_snprintf(temp, sizeof(temp), "loading/%s", fn);
				}

				char* found = new char[strlen(temp) + 1];
				Q_strncpy(found, temp, strlen(temp) + 1);

				Q_FixSlashes(found);
				fileNames.AddToTail(found);
			}

			fn = g_pFullFileSystem->FindNext(fh);

		} while (fn);

		g_pFullFileSystem->FindClose(fh);
	}

	int count = fileNames.Count();

	if (!count)
	{
		DevWarning("No loading backgrounds can be found.\n");
		return "";
	}

	// HACK
	int m_nRandomSeed = RandomInt(0, 9999);
	CUniformRandomStream randomize;
	randomize.SetSeed(m_nRandomSeed);
	int index = randomize.RandomInt(0, count - 1);

	const char* pBackgroundFile = NULL;

	if (fileNames.IsValidIndex(index) && fileNames[index])
		pBackgroundFile = fileNames[index];

	char* ext = Q_strstr(pBackgroundFile, ".vmt");
	if (ext)
	{
		*ext = 0;
	}

	return pBackgroundFile;
}

//-----------------------------------------------------------------------------
// Purpose: Called when we are activated during level load
//-----------------------------------------------------------------------------
void CMapLoadBG::OnActivate()
{
	ResizePanel();
}

void CMapLoadBG::Paint()
{
	// don't paint if we're in game.
	if (engine->IsInGame() && !engine->IsLevelMainMenuBackground())
		return;

	BaseClass::Paint();
}