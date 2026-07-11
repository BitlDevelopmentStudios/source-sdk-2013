#base "SourceSchemeBase_BDS.res"

Scheme
{
	BaseSettings
	{
		FrameSystemButton.Icon			"resource/icon_overwatch"
		FrameSystemButton.DisabledIcon	"resource/icon_overwatch_disabled"
	}
	
	Fonts
	{
		"Tip"
		{
			"1"	
			{
				"name"		"Tahoma" [!$OSX]
				"name"		"Verdana" [$OSX]
				"tall"		"16" [!$LINUX]
				"tall"		"18" [$LINUX]
				"weight"	"600"
			}
		}
	}
}
