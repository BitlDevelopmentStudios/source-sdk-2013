"GameMenu"
{
	"1"
	{
		"label" "#GameUI_GameMenu_ResumeGame"
		"command" "ResumeGame"
		"OnlyInGame" "1"
	}
	"2"
	{
		"label" "#GameUI_GameMenu_Disconnect"
		"command" "Disconnect"
		"OnlyInGame" "1"
	}
	"3"
	{
		"label" "#GameUI_GameMenu_PlayerList"
		"command" "OpenPlayerListDialog"
		"OnlyInGame" "1"
	}
	"4"
	{
		"label" ""
		"command" ""
		"OnlyInGame" "1"
	}
	"5"
	{
		"label" "#GameUI_GameMenu_Serverfinder" 
		"command" "engine serverfinderdialog"
	} 
	"6"
	{
		"label" "#GameUI_GameMenu_FindServers"
		"command" "OpenServerBrowser"
	}
	"7"
	{
		"label" "#GameUI_GameMenu_CreateServer"
		"command" "OpenCreateMultiplayerGameDialog"
	}
	"8"
	{
		"label" ""
		"command" ""
	}
	"9"
	{
		"label" "#GameUI_GameMenu_Achievements"
		"command" "OpenAchievementsDialog"
	}
	"10"
	{
		"label" "#GameUI_GameMenu_ActivateVR"
		"command" "engine vr_activate"
		"InGameOrder" "40"
		"OnlyWhenVREnabled" "1"
		"OnlyWhenVRInactive" "1"
	}
	"11"
	{
		"label" "#GameUI_GameMenu_DeactivateVR"
		"command" "engine vr_deactivate"
		"InGameOrder" "40"
		"OnlyWhenVREnabled" "1"
		"OnlyWhenVRActive" "1"
	}
	"12"
	{
		"label" ""
		"command" ""
	}
	"13"
	{
		"label" "#GameUI_GameMenu_Options"
		"command" "OpenOptionsDialog"
	}
	"14"
	{
		"label" "#GameUI_GameMenu_Quit"
		"command" "Quit"
	}
}

