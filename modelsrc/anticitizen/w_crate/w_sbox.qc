// Created by Crowbar 0.74

$modelname "weapons/w_crate.mdl"

$bodygroup "sbox"
{
	studio "w_sbox_ref.smd"
}


$surfaceprop "wood"

$contents "solid"

$illumposition 0.132 0.738 8

$cdmaterials "models\items\"

$cbox 0 0 0 0 0 0

$bbox -12.738 -12.736 -1 11.262 13 17

$definebone "ValveBiped.Bip01_R_Hand" "" -13 6 9 0 -89.999982 -179.999978 0 0 0 0 0 0


$sequence "idle" {
	"w_sbox_anims\idle.smd"
	activity "ACT_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "attack1" {
	"w_sbox_anims\attack1.smd"
	activity "ACT_RANGE_ATTACK" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$collisionmodel "w_sbox_ref.smd"
{
	$mass 15
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave
	$maxconvexpieces 1

}