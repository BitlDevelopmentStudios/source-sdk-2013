// Created by Crowbar 0.74

$modelname "weapons/c_irifle.mdl"

$bodygroup "studio"
{
	studio "c_irifle_c_ar2.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition 0 0 0

$cdmaterials "models\Weapons\V_hand\"
$cdmaterials "models\Weapons\V_irifle\"

$attachment "muzzle" "Base" 0 0 20 rotate -90 0 0

$cbox 0 0 0 0 0 0

$bbox -7.388 -12.016 -18.464 40.372 12.292 1.128

$definebone "Base" "" -5.846632 -11.638401 61.824333 0 0 89.999982 0 0 0 0 0 0
$definebone "Reload" "Base" 1.526086 -2.764851 14.0826 0 0 0 0 0 0 0 0 0
$definebone "Reload1" "Reload" 0.832381 1.842239 -0.000374 0 0 0 0 0 0 0 0 0
$definebone "Shell1" "Base" -0.05046 -0.056503 18.287693 0 0 0 0 0 0 0 0 0
$definebone "Bolt1" "Base" -0.112315 0.055737 8.441936 0 0 0 0 0 0 0 0 0
$definebone "Bolt2" "Bolt1" 0.487353 -0.167217 2.950579 0 0 0 0 0 0 0 0 0
$definebone "Shell2" "Base" 1.617459 -1.028965 14.611858 0 0 0 0 0 0 0 0 0
$definebone "Vent" "Base" 0.790197 0.067791 19.4118 0 0 0 0 0 0 0 0 0
$definebone "Claw2" "Base" 0.599579 0.80545 17.492916 0 -38.249978 -30.000014 0 0 0 0 0 0
$definebone "Claw1" "Base" 1.023674 -0.207508 17.423466 30.000018 -6.750017 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Spine4" "Base" 6.799025 -2.73719 -20.065849 -10.250043 89.999982 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Neck1" "ValveBiped.Bip01_Spine4" 3.307266 0.000002 0.000001 0 22.945698 179.999855 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_Head1" "ValveBiped.Bip01_Neck1" 3.593712 0.000004 -0.000001 -0.000057 23.295719 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Clavicle" "ValveBiped.Bip01_Spine4" 2.033352 1.000772 -1.93761 73.712037 168.618779 -95.596557 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_UpperArm" "ValveBiped.Bip01_R_Clavicle" 6.028143 -0.000004 0.000001 56.223317 -27.688929 97.090565 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Forearm" "ValveBiped.Bip01_R_UpperArm" 11.692551 0.000004 0 0.000003 -64.264096 0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Hand" "ValveBiped.Bip01_R_Forearm" 11.481701 -0.000002 0 -9.147041 27.620689 -59.51456 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger4" "ValveBiped.Bip01_R_Hand" 3.859675 -0.13257 1.193111 -0.448972 -45.040559 7.945314 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger41" "ValveBiped.Bip01_R_Finger4" 1.312562 0.000004 -0.000004 -0.000006 -64.395071 10.069504 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger42" "ValveBiped.Bip01_R_Finger41" 0.729362 0.000002 -0.000008 3.158201 -54.131913 10.285116 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger3" "ValveBiped.Bip01_R_Hand" 3.942298 0.050329 0.431049 3.534516 -45.518689 7.031962 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger31" "ValveBiped.Bip01_R_Finger3" 1.539101 -0.000002 -0.000004 -0.000006 -70.725338 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger32" "ValveBiped.Bip01_R_Finger31" 1.196325 -0.000002 -0.000004 -0.000001 -77.511275 0.000006 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger2" "ValveBiped.Bip01_R_Hand" 3.883657 -0.050325 -0.431038 -8.508882 -29.654865 9.363844 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger21" "ValveBiped.Bip01_R_Finger2" 1.719577 -0.000001 0.000004 -0.095117 -98.977829 -2.659553 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger22" "ValveBiped.Bip01_R_Finger21" 1.209179 0 0 -0.1708 -60.46063 0.190114 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger1" "ValveBiped.Bip01_R_Hand" 3.859831 -0.148935 -1.33128 14.90865 9.219522 -14.117686 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger11" "ValveBiped.Bip01_R_Finger1" 1.719423 -0.000002 0 7.931509 -100.756348 -4.117449 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger12" "ValveBiped.Bip01_R_Finger11" 1.099661 -0.000002 -0.000004 -0.241275 -13.998048 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger0" "ValveBiped.Bip01_R_Hand" 0.806017 -0.359833 -1.318333 38.596439 -45.467517 69.981633 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger01" "ValveBiped.Bip01_R_Finger0" 1.789782 0 -0.000008 0 44.144564 0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger02" "ValveBiped.Bip01_R_Finger01" 1.207001 -0.000004 0.000008 -0.000003 59.823031 0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Clavicle" "ValveBiped.Bip01_Spine4" -5.398506 12.745693 8.201042 -30.794625 129.070399 127.569096 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_UpperArm" "ValveBiped.Bip01_L_Clavicle" 6.028147 0 -0.000001 -75.015947 -145.007894 -15.778627 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Forearm" "ValveBiped.Bip01_L_UpperArm" 11.692558 -0.000011 0.000003 -0.375808 -1.422371 0.006126 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Hand" "ValveBiped.Bip01_L_Forearm" 11.481672 -0.000011 -0.000033 34.978833 -12.445503 22.553336 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger4" "ValveBiped.Bip01_L_Hand" 3.859703 -0.142406 -1.191978 8.682198 -50.431573 -26.822037 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger41" "ValveBiped.Bip01_L_Finger4" 1.312557 0 0.000004 -0.000002 -25.194104 0.000003 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger42" "ValveBiped.Bip01_L_Finger41" 0.729359 0.000001 0.000004 0.000001 -13.997302 0.000003 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger3" "ValveBiped.Bip01_L_Hand" 3.942326 0.046768 -0.431442 4.194391 -46.466023 -8.696345 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger31" "ValveBiped.Bip01_L_Finger3" 1.539097 0 0.000004 0.000001 -14.197728 0.000006 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger32" "ValveBiped.Bip01_L_Finger31" 1.19632 0.000002 0 0.000004 -24.683139 0.000005 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger2" "ValveBiped.Bip01_L_Hand" 3.883694 -0.046772 0.431442 1.25076 -33.682296 4.921829 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger21" "ValveBiped.Bip01_L_Finger2" 1.719578 0 0 0.316042 -20.997639 0.00001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger22" "ValveBiped.Bip01_L_Finger21" 1.209175 0 -0.000004 0.176703 -12.398806 0.000012 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger1" "ValveBiped.Bip01_L_Hand" 3.859859 -0.137947 1.332466 -2.449402 -30.465202 20.308437 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger11" "ValveBiped.Bip01_L_Finger1" 1.719425 0.000003 0 0.23342 -20.798881 0.000005 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger12" "ValveBiped.Bip01_L_Finger11" 1.099663 0.000002 0 0.14874 -13.999139 0.000007 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger0" "ValveBiped.Bip01_L_Hand" 0.806046 -0.348923 1.321259 -32.613798 -45.204545 -46.520955 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger01" "ValveBiped.Bip01_L_Finger0" 1.789783 0.000001 -0.000004 1.356757 14.390918 5.143384 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger02" "ValveBiped.Bip01_L_Finger01" 1.207005 0.000001 -0.000004 9.143083 3.881842 2.758501 0 0 0 0 0 0

$bonemerge "Base"
$bonemerge "ValveBiped.Bip01_Spine4"
$bonemerge "ValveBiped.Bip01_Neck1"
$bonemerge "ValveBiped.Bip01_Head1"
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


$sectionframes 30 150

$poseparameter "VentPoses" 0 1 loop 0

$animation "a_vent_open_corrective_animation" "c_irifle_anims\a_vent_open_corrective_animation.smd" {
}

$animation "a_vent_closed_corrective_animation" "c_irifle_anims\a_vent_closed_corrective_animation.smd" {
}

$animation "a_vent_open" "c_irifle_anims\a_vent_open.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_vent_open_corrective_animation" 0
}

$animation "a_vent_closed" "c_irifle_anims\a_vent_closed.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_vent_closed_corrective_animation" 0
}

// one-handed

$sequence "IR_idle" {
	"c_irifle_anims\IR_idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
	loop
}

$sequence "IR_fire" {
	"c_irifle_anims\IR_fire.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "fire2" {
	"c_irifle_anims\fire2.smd"
	activity "ACT_VM_RECOIL1" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "fire3" {
	"c_irifle_anims\fire3.smd"
	activity "ACT_VM_RECOIL2" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "fire4" {
	"c_irifle_anims\fire4.smd"
	activity "ACT_VM_RECOIL3" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "IR_fire2" {
	"c_irifle_anims\IR_fire2.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "IR_reload" {
	"c_irifle_anims\IR_reload.smd"
	activity "ACT_VM_RELOAD" 1
	{ event AE_CL_PLAYSOUND 1 "Weapon_AR2.Reload_Rotate" }
	{ event AE_CL_PLAYSOUND 19 "Weapon_AR2.Reload_Push" }
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
}

$sequence "IR_draw" {
	"c_irifle_anims\IR_draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	snap
	fps 30
}

$sequence "IR_holster" {
	"c_irifle_anims\IR_holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
}

$sequence "idletolow" {
	"c_irifle_anims\idletolow.smd"
	activity "ACT_VM_LOWERED_TO_IDLE" 1
	fadein 0.2
	fadeout 0.2
	transition "Ready" "Low"
	fps 30
}

$sequence "lowtoidle" {
	"c_irifle_anims\lowtoidle.smd"
	activity "ACT_VM_IDLE_TO_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	transition "Low" "Ready"
	fps 30
}

$sequence "lowidle" {
	"c_irifle_anims\lowidle.smd"
	activity "ACT_VM_IDLE_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	node "Low"
	fps 30
	loop
}

$sequence "shake" {
	"c_irifle_anims\shake.smd"
	activity "ACT_VM_FIDGET" 1
	fadein 0.2
	fadeout 0.2
	node "Fire"
	fps 30
	loop
}

// two-handed

$sequence "IR_idle_2hand" {
	"c_irifle_anims_2hand\IR_idle_2hand.smd"
	activity "ACT_VM_IDLE_TWO_HANDED" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
	loop
}

$sequence "IR_fire_2hand" {
	"c_irifle_anims_2hand\IR_fire_2hand.smd"
	activity "ACT_VM_PRIMARYATTACK_TWO_HANDED" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "fire2_2hand" {
	"c_irifle_anims_2hand\fire2_2hand.smd"
	activity "ACT_VM_RECOIL1_TWO_HANDED" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "fire3_2hand" {
	"c_irifle_anims_2hand\fire3_2hand.smd"
	activity "ACT_VM_RECOIL2_TWO_HANDED" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "fire4_2hand" {
	"c_irifle_anims_2hand\fire4_2hand.smd"
	activity "ACT_VM_RECOIL3_TWO_HANDED" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "IR_fire2_2hand" {
	"c_irifle_anims_2hand\IR_fire2_2hand.smd"
	activity "ACT_VM_SECONDARYATTACK_TWO_HANDED" 1
	{ event AE_MUZZLEFLASH 0 "COMBINE MUZZLE" }
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}

$sequence "IR_reload_2hand" {
	"c_irifle_anims_2hand\IR_reload_2hand.smd"
	activity "ACT_VM_RELOAD_TWO_HANDED" 1
	{ event AE_CL_PLAYSOUND 1 "Weapon_AR2.Reload_Rotate" }
	{ event AE_CL_PLAYSOUND 19 "Weapon_AR2.Reload_Push" }
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
}

$sequence "IR_draw_2hand" {
	"c_irifle_anims_2hand\IR_draw_2hand.smd"
	activity "ACT_VM_DRAW_TWO_HANDED" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	snap
	fps 30
}

$sequence "IR_holster_2hand" {
	"c_irifle_anims_2hand\IR_holster_2hand.smd"
	activity "ACT_VM_HOLSTER_TWO_HANDED" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
}

$sequence "idletolow_2hand" {
	"c_irifle_anims_2hand\idletolow_2hand.smd"
	activity "ACT_VM_LOWERED_TO_IDLE_TWO_HANDED" 1
	fadein 0.2
	fadeout 0.2
	transition "Ready" "Low"
	fps 30
}

$sequence "lowtoidle_2hand" {
	"c_irifle_anims_2hand\lowtoidle_2hand.smd"
	activity "ACT_VM_IDLE_TO_LOWERED_TWO_HANDED" 1
	fadein 0.2
	fadeout 0.2
	transition "Low" "Ready"
	fps 30
}

$sequence "lowidle_2hand" {
	"c_irifle_anims_2hand\lowidle_2hand.smd"
	activity "ACT_VM_IDLE_LOWERED_TWO_HANDED" 1
	fadein 0.2
	fadeout 0.2
	node "Low"
	fps 30
	loop
}

$sequence "shake_2hand" {
	"c_irifle_anims_2hand\shake_2hand.smd"
	activity "ACT_VM_FIDGET_TWO_HANDED" 1
	fadein 0.2
	fadeout 0.2
	node "Fire"
	fps 30
	loop
}

$sequence "VentPoses" {
	"a_vent_open"
	"a_vent_closed"
	autoplay
	blend "VentPoses" 1 0
	blendwidth 2
	delta
	fadein 0.2
	fadeout 0.2
}
