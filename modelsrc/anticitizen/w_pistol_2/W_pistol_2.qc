// Created by Crowbar 0.74

$modelname "weapons/W_pistol_2.mdl"

$bodygroup "studio"
{
	studio "w_pistol_2_w_pistl2_TWO_ref.smd"
}

$lod 16
{
	replacemodel "w_pistol_2_w_pistl2_TWO_ref.smd" "w_pistol_2_w_pistl2_TWO_ref_lod1.smd"
	nofacial
}

$surfaceprop "weapon"

$contents "solid"

$illumposition 0 0.91 20.399

$cdmaterials "models\weapons\w_pistol\"

$attachment "muzzle" "ValveBiped.Weapon_bone" 6.31 -0.68 0.42 rotate 4.49 -1.17 -98.82
$attachment "0" "ValveBiped.Weapon_bone" 1.06 0.18 0.72 rotate 8.79 89.53 -85.45
$attachment "1" "ValveBiped.Weapon_bone1" 1.06 0.18 -0.72 rotate -8.79 89.53 85.45
$attachment "muzzle1" "ValveBiped.Weapon_bone1" 6.31 -0.68 -0.42 rotate -4.49 -1.17 98.82

$cbox 0 0 0 0 0 0

$bbox -4.721 -29.761 0 2.888 29.761 40.798

$definebone "ValveBiped" "" 0 0 0 0 0 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Hand" "ValveBiped" 22.676542 40.797947 -2.35188 1.296529 -47.570096 -0.568758 0 0 0 0 0 0
$definebone "ValveBiped.Weapon_bone1" "ValveBiped.Bip01_L_Hand" 5.000002 -1.499996 5 0 0.000003 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Hand" "ValveBiped" -22.676542 40.797947 -2.35188 1.296529 -132.429882 -179.431228 0 0 0 0 0 0
$definebone "ValveBiped.Weapon_bone" "ValveBiped.Bip01_R_Hand" 5.000001 -1.5 -5 0 0.000003 0 0 0 0 0 0 0


$sequence "idle" {
	"W_pistol_2_anims\idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$sequence "attack1" {
	"W_pistol_2_anims\attack1.smd"
	activity "ACT_RANGE_ATTACK_PISTOL" 1
	{ event 3014 1 "" }
	{ event AE_NPC_MUZZLEFLASH 0 "PISTOL MUZZLE" }
	fadein 0.2
	fadeout 0.2
	fps 30
}

$collisionmodel "W_pistol_2_physics.smd"
{
	$mass 2
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "

}
