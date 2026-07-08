#base "SourceSchemeBase.res"

Scheme
{
	BaseSettings
	{
		MainMenu.MenuItemHeight		"20"
	}
	
	Fonts
	{
		"MenuLarge"
		{
			"1"
			{
				"name"		"GorDIN"
				"tall"		"16" [$DECK]
				"tall"		"12"
				"weight"	"600"
				"antialias" "1"
				"yres"	"1 599"
			}
			"2"
			{
				"name"		"GorDIN"
				"tall"		"22" [$DECK]
				"tall"		"14"
				"weight"	"600"
				"antialias" "1"
				"yres"	"600 767"
			}
			"3"
			{
				"name"		"GorDIN"
				"tall"		"26" [$DECK]
				"tall"		"19"
				"weight"	"700"
				"antialias" "1"
				"yres"	"768 1023"
			}
			"4"
			{
				"name"		"GorDIN"
				"tall"		"30" [$DECK]
				"tall"		"21"
				"weight"	"700"
				"antialias" "1"
				"yres"	"1024 1199"
			}
			"5" // Proportional - misyl
			{
				"name"		"GorDIN"
				"tall"		"14" [$DECK]
				"tall"		"10"
				"weight"	"700"
				"antialias" "1"
				"additive"	"1"
			}
		}
	}
	
	CustomFontFiles
	{
		"1"	"resource/GorDIN-Regular.ttf"
		"2"	"resource/GorDIN-Bold.ttf"
		"3"	"resource/GorDIN-Semibold.ttf"
		"4"	"resource/GorDIN-Black.ttf"
		"5"	"resource/GorDIN-Light.ttf"
	}
}