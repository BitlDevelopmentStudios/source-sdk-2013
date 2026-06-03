// Created by Crowbar 0.74

$modelname "weapons/w_turret.mdl"

$bodygroup "studio"
{
	studio "floor_turret_Floor_reference.smd"
}

$surfaceprop "weapon"

$contents "solid"

$illumposition -1.715 0.804 -13.168

$cdmaterials "models\Combine_turrets\Floor_turret\"

$cbox 0 0 0 0 0 0

$bbox -15.684 -29.416 -49.833 14.075 25.987 23.498

$definebone "ValveBiped" "" 0 0 0 0 0 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Hand" "ValveBiped" -4.843294 -0.443567 -1.440222 -77.816209 -38.907906 109.304993 0 0 0 0 0 0

$sequence "idle" {
	"w_turret_rb_anims\idle.smd"
	activity "ACT_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "attack1" {
	"w_turret_rb_anims\attack1.smd"
	activity "ACT_RANGE_ATTACK" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$collisionmodel "floor_turret_Floor_reference.smd"
{
	$mass 1
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
}
