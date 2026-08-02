#base "SourceSchemeBase_BDS.res"

Scheme
{
	Colors
	{
		"AnticitizenOneBaseColor"				"47 98 168 255"
		"AnticitizenOneBaseColor_Trans"			"47 98 168 128"
		
		"AnticitizenOneDarkColor"				"35 75 130 255"
		"AnticitizenOneDarkColor_Trans"			"35 75 130 128"
		
		"AnticitizenOneVeryDarkColor"			"18 42 74 255"
		"AnticitizenOneVeryDarkColor_Trans"		"18 42 74 128"
		
		"AnticitizenOneSelection"				"221 221 221 255"
		"AnticitizenOneSelection_OutOfFocus"	"168 168 168 255"
		
		//AnticitizenOneVeryDarkColor
		"AchievementsProgressBG"	"18 42 74 255"
		//AnticitizenOneBaseColor
		"AchievementsLightGrey"		"47 98 168 255"
		//AnticitizenOneDarkColor
		"AchievementsDarkGrey"		"35 75 130 255"
		//AnticitizenOneSelection
		"AchievementsInactiveFG"	"221 221 221 255"
	}

	BaseSettings
	{
		FrameSystemButton.Icon				"resource/icon_overwatch"
		FrameSystemButton.DisabledIcon		"resource/icon_overwatch_disabled"
		
		Frame.BgColor						"AnticitizenOneBaseColor"
		Frame.OutOfFocusBgColor				"AnticitizenOneBaseColor_Trans"
		
		Menu.BgColor						"AnticitizenOneBaseColor"
		Menu.ArmedBgColor					"AnticitizenOneSelection"
		
		ListPanel.SelectedBgColor			"AnticitizenOneSelection"
		ListPanel.SelectedOutOfFocusBgColor	"AnticitizenOneSelection_OutOfFocus"
		
		Menu.ArmedBgColor					"AnticitizenOneSelection"
		
		SectionedListPanel.SelectedBgColor	"AnticitizenOneSelection"
		
		Tooltip.BgColor						"AnticitizenOneSelection"
		
		NewGame.SelectionColor				"AnticitizenOneSelection"
		
		RichText.SelectedBgColor			"AnticitizenOneSelection"
		
		TextEntry.SelectedBgColor			"AnticitizenOneSelection"
		TextEntry.OutOfFocusSelectedBgColor	"AnticitizenOneSelection_OutOfFocus"
		TextEntry.FocusEdgeColor			"AnticitizenOneSelection"
		
		Slider.NobColor						"AnticitizenOneDarkColor"		
		Slider.TrackColor					"AnticitizenOneDarkColor"
		
		ScrollBarButton.BgColor				"AnticitizenOneBaseColor"
		ScrollBarButton.ArmedBgColor		"AnticitizenOneSelection"
		ScrollBarButton.DepressedBgColor	"AnticitizenOneDarkColor"

		ScrollBarSlider.BgColor				"AnticitizenOneDarkColor"
		ScrollBarSlider.FgColor				"AnticitizenOneBaseColor"
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
