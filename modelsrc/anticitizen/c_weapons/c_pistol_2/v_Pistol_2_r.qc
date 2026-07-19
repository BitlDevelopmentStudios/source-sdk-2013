// Created by Crowbar 0.74

$modelname "weapons/c_Pistol_2.mdl"

$bodygroup "studio"
{
	studio "v_Pistol_2_r_Pistol_reference_rs_2.smd"
}

$surfaceprop "default"

$contents "solid"

$illumposition 0.069 -14.614 -33.885

$cdmaterials "models\Weapons\v_pistol\"

$attachment "muzzle" "ValveBiped.muzzle" 0 0 0 rotate 0 0 0
$attachment "1" "ValveBiped.eject" -1 0 0 rotate 0 90 -180
$attachment "2" "ValveBiped.eject1" -1 0 0 rotate 0 -90 0
$attachment "muzzle1" "ValveBiped.muzzle1" 0 0 0 rotate 0 0 0

$cbox 0 0 0 0 0 0

$bbox -5.067 -12.11 -66 34.296 12.249 -1.771

$definebone "ValveBiped" "" 0 0 0 0 0 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01" "ValveBiped" -0.000005 38.566917 0.533613 0 0 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine" "ValveBiped.Bip01" 0.000005 3.345128 -2.981901 4.944225 89.999982 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine1" "ValveBiped.Bip01_Spine" 4.018332 0 0 0 -1.675443 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine2" "ValveBiped.Bip01_Spine1" 3.518567 0 0 0 5.74883 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine4" "ValveBiped.Bip01_Spine2" 8.942642 0 0 0 11.120881 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Clavicle" "ValveBiped.Bip01_Spine4" 2.033347 1.000767 1.93766 -73.711859 168.619271 91.480542 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_UpperArm" "ValveBiped.Bip01_L_Clavicle" 6.028141 0.000001 0 2.738853 -33.527937 -90.505156 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Forearm" "ValveBiped.Bip01_L_UpperArm" 11.692556 0 0.000001 -0.000003 -3.459061 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Hand" "ValveBiped.Bip01_L_Forearm" 11.481672 0 -0.000031 -6.108477 2.532072 90.136743 0 0 0 0 0 0
$definebone "ValveBiped.clip1" "ValveBiped.Bip01_L_Hand" 3.820721 -1.405028 0.601078 0.197212 -90.610068 89.627108 0 0 0 0 0 0
$definebone "ValveBiped.base1" "ValveBiped.Bip01_L_Hand" 6.114854 -1.434045 1.934793 0.197212 -90.610068 89.627108 0 0 0 0 0 0
$definebone "ValveBiped.eject1" "ValveBiped.base1" 0.051127 3.12217 -0.141701 0 89.999982 -0.000004 0 0 0 0 0 0
$definebone "ValveBiped.muzzle1" "ValveBiped.base1" -0.000001 1.53787 -5.938447 89.999982 0 0 0 0 0 0 0 0
$definebone "ValveBiped.square1" "ValveBiped.base1" 0.000001 1.607459 0.020758 -0.000005 0 0 0 0 0 0 0 0
$definebone "ValveBiped.hammer1" "ValveBiped.base1" -0.000001 1.225114 3.954165 -0.000005 0 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Clavicle" "ValveBiped.Bip01_Spine4" 2.033355 1.000771 -1.93761 73.712037 168.618779 -95.596496 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_UpperArm" "ValveBiped.Bip01_R_Clavicle" 6.028143 0 0 -0.462091 -33.623799 93.935236 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Forearm" "ValveBiped.Bip01_R_UpperArm" 11.69255 0 0.000001 0.000001 -3.459061 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Hand" "ValveBiped.Bip01_R_Forearm" 11.481699 0 0.000003 6.108076 2.532072 -89.66423 0 0 0 0 0 0
$definebone "ValveBiped.base" "ValveBiped.Bip01_R_Hand" 6.114873 -1.450003 -1.922835 -0.669731 -90.60985 -89.621945 0 0 0 0 0 0
$definebone "ValveBiped.eject" "ValveBiped.base" 0.051129 3.122169 0.141701 0 89.999982 0 0 0 0 0 0 0
$definebone "ValveBiped.muzzle" "ValveBiped.base" 0.000001 1.53787 5.938447 -89.999982 0 0 0 0 0 0 0 0
$definebone "ValveBiped.square" "ValveBiped.base" 0 1.607459 -0.020758 0.000001 0 0 0 0 0 0 0 0
$definebone "ValveBiped.hammer" "ValveBiped.base" 0 1.225114 -3.954162 0.000001 0 0 0 0 0 0 0 0
$definebone "ValveBiped.clip" "ValveBiped.Bip01_R_Hand" 3.820735 -1.409995 -0.589408 -0.669731 -90.60985 -89.621945 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger4" "ValveBiped.Bip01_L_Hand" 3.859704 -0.142406 -1.191974 8.719846 -50.413063 -27.452933 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger41" "ValveBiped.Bip01_L_Finger4" 1.312562 0 0.000001 0.553762 -25.194095 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger42" "ValveBiped.Bip01_L_Finger41" 0.729363 0 0 0.284817 -13.997299 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger3" "ValveBiped.Bip01_L_Hand" 3.942326 0.046772 -0.431443 4.201386 -46.446704 -8.881647 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger31" "ValveBiped.Bip01_L_Finger3" 1.539094 0 0 0.255539 -14.197719 -0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger32" "ValveBiped.Bip01_L_Finger31" 1.196323 0 0 0.421754 -24.683139 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger2" "ValveBiped.Bip01_L_Hand" 3.883687 -0.046769 0.431443 1.250767 -33.682292 4.921822 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger21" "ValveBiped.Bip01_L_Finger2" 1.719576 -0.000001 0 0.316045 -20.997634 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger22" "ValveBiped.Bip01_L_Finger21" 1.209181 -0.000001 0 0.176702 -12.398801 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger1" "ValveBiped.Bip01_L_Hand" 3.859859 -0.137948 1.332466 -2.449395 -30.465195 20.308432 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger11" "ValveBiped.Bip01_L_Finger1" 1.719429 -0.000001 0 0.233426 -20.798879 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger12" "ValveBiped.Bip01_L_Finger11" 1.099665 0 0 0.148742 -13.999134 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger0" "ValveBiped.Bip01_L_Hand" 0.806046 -0.348923 1.321258 -38.931802 -45.224011 -69.147647 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger01" "ValveBiped.Bip01_L_Finger0" 1.789784 0 0.000001 0.423239 13.093403 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger02" "ValveBiped.Bip01_L_Finger01" 1.206999 0 0 0.645546 20.790346 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger4" "ValveBiped.Bip01_R_Hand" 3.859672 -0.132568 1.193108 -9.083959 -50.36534 27.452701 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger41" "ValveBiped.Bip01_R_Finger4" 1.312563 0.000001 0 -0.683653 -25.191353 -0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger42" "ValveBiped.Bip01_R_Finger41" 0.729365 0 0 -0.351452 -13.995697 -0.000003 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger3" "ValveBiped.Bip01_R_Hand" 3.942296 0.050325 0.431043 -4.543842 -46.421326 8.880788 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger31" "ValveBiped.Bip01_R_Finger3" 1.539096 -0.000001 0 -0.33541 -14.196232 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger32" "ValveBiped.Bip01_R_Finger31" 1.196322 0.000002 0 -0.55365 -24.680786 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger2" "ValveBiped.Bip01_R_Hand" 3.883656 -0.050329 -0.431043 -1.512838 -33.672447 -4.921994 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger21" "ValveBiped.Bip01_R_Finger2" 1.719576 -0.000001 0 -0.456877 -20.995237 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger22" "ValveBiped.Bip01_R_Finger21" 1.20918 -0.000002 0 -0.255597 -12.39743 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger1" "ValveBiped.Bip01_R_Hand" 3.859828 -0.148936 -1.331282 2.209726 -30.481413 -20.306714 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger11" "ValveBiped.Bip01_R_Finger1" 1.719426 0.000001 0 -0.378897 -20.79665 0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger12" "ValveBiped.Bip01_R_Finger11" 1.099666 0.000001 0 -0.241272 -13.998049 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger0" "ValveBiped.Bip01_R_Hand" 0.806018 -0.359836 -1.318334 38.595469 -45.490327 68.995579 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger01" "ValveBiped.Bip01_R_Finger0" 1.789783 0.000001 -0.000001 -0.361021 13.095235 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger02" "ValveBiped.Bip01_R_Finger01" 1.207001 0 0.000001 -0.551071 20.792921 0.000001 0 0 0 0 0 0

$bonemerge "ValveBiped.Bip01_Spine4"
$bonemerge "ValveBiped.Bip01_L_Clavicle"
$bonemerge "ValveBiped.Bip01_L_UpperArm"
$bonemerge "ValveBiped.Bip01_L_Forearm"
$bonemerge "ValveBiped.Bip01_L_Hand"
$bonemerge "ValveBiped.Bip01_L_Finger4"
$bonemerge "ValveBiped.Bip01_L_Finger41"
$bonemerge "ValveBiped.Bip01_L_Finger42"
$bonemerge "ValveBiped.Bip01_L_Finger3"
$bonemerge "ValveBiped.Bip01_L_Finger31"
$bonemerge "ValveBiped.Bip01_L_Finger32"
$bonemerge "ValveBiped.Bip01_L_Finger2"
$bonemerge "ValveBiped.Bip01_L_Finger21"
$bonemerge "ValveBiped.Bip01_L_Finger22"
$bonemerge "ValveBiped.Bip01_L_Finger1"
$bonemerge "ValveBiped.Bip01_L_Finger11"
$bonemerge "ValveBiped.Bip01_L_Finger12"
$bonemerge "ValveBiped.Bip01_L_Finger0"
$bonemerge "ValveBiped.Bip01_L_Finger01"
$bonemerge "ValveBiped.Bip01_L_Finger02"
$bonemerge "ValveBiped.Bip01_R_Clavicle"
$bonemerge "ValveBiped.Bip01_R_UpperArm"
$bonemerge "ValveBiped.Bip01_R_Forearm"
$bonemerge "ValveBiped.Bip01_R_Hand"
$bonemerge "ValveBiped.Bip01_R_Finger4"
$bonemerge "ValveBiped.Bip01_R_Finger41"
$bonemerge "ValveBiped.Bip01_R_Finger42"
$bonemerge "ValveBiped.Bip01_R_Finger3"
$bonemerge "ValveBiped.Bip01_R_Finger31"
$bonemerge "ValveBiped.Bip01_R_Finger32"
$bonemerge "ValveBiped.Bip01_R_Finger2"
$bonemerge "ValveBiped.Bip01_R_Finger21"
$bonemerge "ValveBiped.Bip01_R_Finger22"
$bonemerge "ValveBiped.Bip01_R_Finger1"
$bonemerge "ValveBiped.Bip01_R_Finger11"
$bonemerge "ValveBiped.Bip01_R_Finger12"
$bonemerge "ValveBiped.Bip01_R_Finger0"
$bonemerge "ValveBiped.Bip01_R_Finger01"
$bonemerge "ValveBiped.Bip01_R_Finger02"

$sequence "idle01_2" {
	"v_Pistol_2_r_anims\idle01_2.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
	loop
}

$sequence "idle01empty_2" {
	"v_Pistol_2_r_anims\idle01empty_2.smd"
	activity "ACT_VM_IDLE_EMPTY" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
	loop
}

$sequence "idle01empty_l_2" {
	"v_Pistol_2_r_anims\idle01empty_l_2.smd"
	activity "ACT_VM_IDLE_EMPTY_LEFT" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
	loop
}

$sequence "idle01empty_r_2" {
	"v_Pistol_2_r_anims\idle01empty_r_2.smd"
	activity "ACT_VM_IDLE_EMPTY_RIGHT" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
	loop
}

$sequence "fire_r" {
	"v_Pistol_2_r_anims\fire_r.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event AE_MUZZLEFLASH 0 "PISTOL MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fire_r_1" {
	"v_Pistol_2_r_anims\fire_r_1.smd"
	activity "ACT_VM_RECOIL1" 1
	{ event AE_MUZZLEFLASH 0 "PISTOL MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fire_r_2" {
	"v_Pistol_2_r_anims\fire_r_2.smd"
	activity "ACT_VM_RECOIL2" 1
	{ event AE_MUZZLEFLASH 0 "PISTOL MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fire_r_3" {
	"v_Pistol_2_r_anims\fire_r_3.smd"
	activity "ACT_VM_RECOIL3" 1
	{ event AE_MUZZLEFLASH 0 "PISTOL MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fireempty_r" {
	"v_Pistol_2_r_anims\fireempty_r.smd"
	activity "ACT_VM_DRYFIRE" 1
	fadein 0.2
	fadeout 0.2
	node "2"
	fps 30
}

$sequence "fire_l" {
	"v_Pistol_2_r_anims\fire_l.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	{ event AE_MUZZLEFLASH 0 "PISTOL MUZZLE1" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fire_l_1" {
	"v_Pistol_2_r_anims\fire_l_1.smd"
	activity "ACT_VM_HITLEFT" 1
	{ event AE_MUZZLEFLASH 0 "PISTOL MUZZLE1" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fire_l_2" {
	"v_Pistol_2_r_anims\fire_l_2.smd"
	activity "ACT_VM_HITLEFT2" 1
	{ event AE_MUZZLEFLASH 0 "PISTOL MUZZLE1" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fire_l_3" {
	"v_Pistol_2_r_anims\fire_l_3.smd"
	activity "ACT_VM_MISSLEFT" 1
	{ event AE_MUZZLEFLASH 0 "PISTOL MUZZLE1" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$sequence "fireempty_l" {
	"v_Pistol_2_r_anims\fireempty_l.smd"
	activity "ACT_VM_DRYFIRE_LEFT" 1
	fadein 0.2
	fadeout 0.2
	node "2"
	fps 30
}

$sequence "draw_2" {
	"v_Pistol_2_r_anims\draw_2.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	snap
	fps 30
}

$sequence "drawempty_2" {
	"v_Pistol_2_r_anims\drawempty_2.smd"
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$sequence "reload_2" {
	"v_Pistol_2_r_anims\reload_2.smd"
	activity "ACT_VM_RELOAD" 1
	{ event AE_CL_PLAYSOUND 15 "Weapon_Pistol.Reload" }
	{ event AE_CL_PLAYSOUND 18 "Weapon_Pistol.Reload" }
	{ event AE_CL_PLAYSOUND 35 "Weapon_357.ReplaceLoader" }
	{ event AE_CL_PLAYSOUND 63 "Weapon_Pistol.Reload" }
	{ event AE_CL_PLAYSOUND 79 "Weapon_357.ReplaceLoader" }
	fadein 0.2
	fadeout 0.2
	fps 30
}

$sequence "holster_2" {
	"v_Pistol_2_r_anims\holster_2.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$sequence "holsterempty_2" {
	"v_Pistol_2_r_anims\holsterempty_2.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$sequence "idletolow_2" {
	"v_Pistol_2_r_anims\idletolow_2.smd"
	activity "ACT_VM_LOWERED_TO_IDLE" 1
	fadein 0.2
	fadeout 0.2
	transition "0" "1"
	fps 30
}

$sequence "lowtoidle_2" {
	"v_Pistol_2_r_anims\lowtoidle_2.smd"
	activity "ACT_VM_IDLE_TO_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	transition "1" "0"
	fps 30
}

$sequence "lowidle_2" {
	"v_Pistol_2_r_anims\lowidle_2.smd"
	activity "ACT_VM_IDLE_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	node "1"
	fps 30
	loop
}
