// Created by Crowbar 0.74

$modelname "weapons/c_357.mdl"

$bodygroup "studio"
{
	studio "c_357_Python_357_ref.smd"
}
$bodygroup "shells"
{
	studio "c_357_Python_357_shell_6_ref.smd"
	studio "c_357_Python_357_shell_5_ref.smd"
	studio "c_357_Python_357_shell_4_ref.smd"
	studio "c_357_Python_357_shell_3_ref.smd"
	studio "c_357_Python_357_shell_2_ref.smd"
	studio "c_357_Python_357_shell_1_ref.smd"
	blank
}


$surfaceprop "default"

$contents "solid"

$illumposition 0 0 0

$cdmaterials "models\weapons\V_357\"

$attachment "muzzle" "Python" 0 0.77 8.03 rotate -90 90 0

$cbox 0 0 0 0 0 0

$bbox -8.336 -8.663 -11.76 28.936 6.724 -0.507

$definebone "ValveBiped.Bip01_Spine4" "" 0.000006 0 0 -79.749938 -89.999873 -90.000098 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Clavicle" "ValveBiped.Bip01_Spine4" 2.033349 1.000767 1.93766 -73.711859 168.619271 91.480542 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_UpperArm" "ValveBiped.Bip01_L_Clavicle" 6.028141 0 0 2.738853 -33.527944 -90.505156 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Forearm" "ValveBiped.Bip01_L_UpperArm" 11.692554 0.000001 0.000002 -0.000002 -3.459061 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Hand" "ValveBiped.Bip01_L_Forearm" 11.481674 0.000001 -0.000032 -6.108474 2.532073 90.136743 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger4" "ValveBiped.Bip01_L_Hand" 3.859709 -0.142404 -1.191975 8.682204 -50.431562 -26.82204 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger41" "ValveBiped.Bip01_L_Finger4" 1.312555 -0.000002 0.000001 -0.000001 -25.194097 0.000003 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger42" "ValveBiped.Bip01_L_Finger41" 0.729363 -0.000002 -0.000002 0.000002 -13.997298 0.00001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger3" "ValveBiped.Bip01_L_Hand" 3.942335 0.046767 -0.431443 4.194397 -46.466006 -8.696352 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger31" "ValveBiped.Bip01_L_Finger3" 1.539097 0 0 0 -14.197714 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger32" "ValveBiped.Bip01_L_Finger31" 1.196326 -0.000002 -0.000001 0 -24.683134 0.000003 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger2" "ValveBiped.Bip01_L_Hand" 3.883696 -0.046765 0.431443 1.250768 -33.682286 4.921822 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger21" "ValveBiped.Bip01_L_Finger2" 1.71957 -0.000004 -0.000001 0.316047 -20.997627 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger22" "ValveBiped.Bip01_L_Finger21" 1.209171 -0.000002 0 0.176703 -12.398792 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger1" "ValveBiped.Bip01_L_Hand" 3.859865 -0.137947 1.332466 -2.449394 -30.465191 20.308437 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger11" "ValveBiped.Bip01_L_Finger1" 1.719429 0.000002 -0.000001 0.23343 -20.798874 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger12" "ValveBiped.Bip01_L_Finger11" 1.099665 0 0 0.148747 -13.999133 -0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger0" "ValveBiped.Bip01_L_Hand" 0.806053 -0.348925 1.321258 -38.916041 -45.195987 -70.054231 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger01" "ValveBiped.Bip01_L_Finger0" 1.78978 -0.000002 0.000002 -0.000009 13.093408 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_L_Finger02" "ValveBiped.Bip01_L_Finger01" 1.207006 0 0 -0.00001 20.79035 -0.000003 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Clavicle" "ValveBiped.Bip01_Spine4" 2.033356 1.000772 -1.93761 73.712037 168.618779 -95.596496 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_UpperArm" "ValveBiped.Bip01_R_Clavicle" 6.028144 0 0 -0.46209 -33.623802 93.935243 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Forearm" "ValveBiped.Bip01_R_UpperArm" 11.692554 0.000001 0.000003 -0.000003 -3.459062 0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Hand" "ValveBiped.Bip01_R_Forearm" 11.481697 0.000001 0.000002 6.108072 2.532072 -89.664285 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger4" "ValveBiped.Bip01_R_Hand" 3.859678 -0.13257 1.193109 -9.056514 -50.352981 26.713353 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger41" "ValveBiped.Bip01_R_Finger4" 1.312557 -0.000002 0.000001 -0.000005 -25.191363 -0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger42" "ValveBiped.Bip01_R_Finger41" 0.729365 0.000002 -0.000002 -0.000003 -13.995702 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger3" "ValveBiped.Bip01_R_Hand" 3.942305 0.050327 0.431044 -4.537309 -46.42134 8.598834 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger31" "ValveBiped.Bip01_R_Finger3" 1.539099 -0.000002 0 -0.000002 -14.196239 0 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger32" "ValveBiped.Bip01_R_Finger31" 1.196321 0.000008 0 -0.000001 -24.680788 0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger2" "ValveBiped.Bip01_R_Hand" 3.883661 -0.050323 -0.431042 -1.512838 -33.672464 -4.921996 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger21" "ValveBiped.Bip01_R_Finger2" 1.719578 0.000004 0 -0.456876 -20.995259 -0.000002 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger22" "ValveBiped.Bip01_R_Finger21" 1.209185 0.000002 0 -0.255596 -12.397447 -0.000001 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger1" "ValveBiped.Bip01_R_Hand" 3.859838 -0.148937 -1.331282 2.209727 -30.481423 -20.306714 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger11" "ValveBiped.Bip01_R_Finger1" 1.719423 0.000002 0 -0.37889 -20.796659 0.000005 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger12" "ValveBiped.Bip01_R_Finger11" 1.099667 0.000002 0 -0.241267 -13.99806 -0.000003 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger0" "ValveBiped.Bip01_R_Hand" 0.806021 -0.359837 -1.318334 38.596446 -45.467538 69.98164 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger01" "ValveBiped.Bip01_R_Finger0" 1.789782 0 -0.000004 -0.000009 13.095232 0.000007 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Finger02" "ValveBiped.Bip01_R_Finger01" 1.206999 0.000003 0.000002 -0.000009 20.792926 0.000003 0 0 0 0 0 0
$definebone "Python" "" -5.648674 -26.343084 62.296627 1.050805 0.207353 89.488619 0 0 0 0 0 0
$definebone "Cylinder_release" "Python" 0.025545 -1.273521 1.753302 0 0 0 0 0 0 0 0 0
$definebone "Cylinder" "Cylinder_release" 0.001032 1.440445 -1.526781 0 0 0 0 0 0 0 0 0
$definebone "Bullet1" "Cylinder" 0.004413 0.696491 -1.430597 0 0 0 0 0 0 0 0 0
$definebone "Bullet2" "Cylinder" 0.58066 0.349014 -1.430597 0 0 0 0 0 0 0 0 0
$definebone "Bullet3" "Cylinder" 0.583443 -0.305904 -1.430599 0 0 0 0 0 0 0 0 0
$definebone "Bullet4" "Cylinder" 0.005317 -0.64872 -1.430599 0 0 0 0 0 0 0 0 0
$definebone "Bullet5" "Cylinder" -0.574618 -0.303123 -1.430601 0 0 0 0 0 0 0 0 0
$definebone "Bullet6" "Cylinder" -0.573714 0.348114 -1.430597 0 0 0 0 0 0 0 0 0

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


$sectionframes 30 120

$poseparameter "gun_spin" 0 360 loop 360

$weightlist "weights_gun_spinning" {
	 "ValveBiped.Bip01_Spine4" 0
	 "ValveBiped.Bip01_L_Clavicle" 0
	 "ValveBiped.Bip01_L_UpperArm" 0
	 "ValveBiped.Bip01_L_Forearm" 0
	 "ValveBiped.Bip01_L_Hand" 0
	 "ValveBiped.Bip01_L_Finger4" 0
	 "ValveBiped.Bip01_L_Finger41" 0
	 "ValveBiped.Bip01_L_Finger42" 0
	 "ValveBiped.Bip01_L_Finger3" 0
	 "ValveBiped.Bip01_L_Finger31" 0
	 "ValveBiped.Bip01_L_Finger32" 0
	 "ValveBiped.Bip01_L_Finger2" 0
	 "ValveBiped.Bip01_L_Finger21" 0
	 "ValveBiped.Bip01_L_Finger22" 0
	 "ValveBiped.Bip01_L_Finger1" 0
	 "ValveBiped.Bip01_L_Finger11" 0
	 "ValveBiped.Bip01_L_Finger12" 0
	 "ValveBiped.Bip01_L_Finger0" 0
	 "ValveBiped.Bip01_L_Finger01" 0
	 "ValveBiped.Bip01_L_Finger02" 0
	 "ValveBiped.Bip01_R_Clavicle" 0
	 "ValveBiped.Bip01_R_UpperArm" 0
	 "ValveBiped.Bip01_R_Forearm" 0
	 "ValveBiped.Bip01_R_Hand" 0
	 "ValveBiped.Bip01_R_Finger4" 0
	 "ValveBiped.Bip01_R_Finger41" 0
	 "ValveBiped.Bip01_R_Finger42" 0
	 "ValveBiped.Bip01_R_Finger3" 0
	 "ValveBiped.Bip01_R_Finger31" 0
	 "ValveBiped.Bip01_R_Finger32" 0
	 "ValveBiped.Bip01_R_Finger2" 0
	 "ValveBiped.Bip01_R_Finger21" 0
	 "ValveBiped.Bip01_R_Finger22" 0
	 "ValveBiped.Bip01_R_Finger1" 0
	 "ValveBiped.Bip01_R_Finger11" 0
	 "ValveBiped.Bip01_R_Finger12" 0
	 "ValveBiped.Bip01_R_Finger0" 0
	 "ValveBiped.Bip01_R_Finger01" 0
	 "ValveBiped.Bip01_R_Finger02" 0
	 "Python" 0
	 "Cylinder_release" 0
	 "Cylinder" 1
	 "Bullet1" 1
	 "Bullet2" 1
	 "Bullet3" 1
	 "Bullet4" 1
	 "Bullet5" 1
	 "Bullet6" 1
}

$weightlist "weights_adjustment" {
	 "ValveBiped.Bip01_Spine4" 0.5
	 "ValveBiped.Bip01_L_Clavicle" 0.5
	 "ValveBiped.Bip01_L_UpperArm" 0.5
	 "ValveBiped.Bip01_L_Forearm" 0.5
	 "ValveBiped.Bip01_L_Hand" 0.5
	 "ValveBiped.Bip01_L_Finger4" 0.5
	 "ValveBiped.Bip01_L_Finger41" 0.5
	 "ValveBiped.Bip01_L_Finger42" 0.5
	 "ValveBiped.Bip01_L_Finger3" 0.5
	 "ValveBiped.Bip01_L_Finger31" 0.5
	 "ValveBiped.Bip01_L_Finger32" 0.5
	 "ValveBiped.Bip01_L_Finger2" 0.5
	 "ValveBiped.Bip01_L_Finger21" 0.5
	 "ValveBiped.Bip01_L_Finger22" 0.5
	 "ValveBiped.Bip01_L_Finger1" 0.5
	 "ValveBiped.Bip01_L_Finger11" 0.5
	 "ValveBiped.Bip01_L_Finger12" 0.5
	 "ValveBiped.Bip01_L_Finger0" 0.5
	 "ValveBiped.Bip01_L_Finger01" 0.5
	 "ValveBiped.Bip01_L_Finger02" 0.5
	 "ValveBiped.Bip01_R_Clavicle" 0.5
	 "ValveBiped.Bip01_R_UpperArm" 0.5
	 "ValveBiped.Bip01_R_Forearm" 0.5
	 "ValveBiped.Bip01_R_Hand" 0.5
	 "ValveBiped.Bip01_R_Finger4" 0.5
	 "ValveBiped.Bip01_R_Finger41" 0.5
	 "ValveBiped.Bip01_R_Finger42" 0.5
	 "ValveBiped.Bip01_R_Finger3" 0.5
	 "ValveBiped.Bip01_R_Finger31" 0.5
	 "ValveBiped.Bip01_R_Finger32" 0.5
	 "ValveBiped.Bip01_R_Finger2" 0.5
	 "ValveBiped.Bip01_R_Finger21" 0.5
	 "ValveBiped.Bip01_R_Finger22" 0.5
	 "ValveBiped.Bip01_R_Finger1" 0.5
	 "ValveBiped.Bip01_R_Finger11" 0.5
	 "ValveBiped.Bip01_R_Finger12" 0.5
	 "ValveBiped.Bip01_R_Finger0" 0.5
	 "ValveBiped.Bip01_R_Finger01" 0.5
	 "ValveBiped.Bip01_R_Finger02" 0.5
	 "Python" 0.5
	 "Cylinder_release" 0.5
	 "Cylinder" 0.5
	 "Bullet1" 0.5
	 "Bullet2" 0.5
	 "Bullet3" 0.5
	 "Bullet4" 0.5
	 "Bullet5" 0.5
	 "Bullet6" 0.5
}

$animation "gun_spin0_corrective_animation" "c_357_anims\gun_spin0_corrective_animation.smd" {
}

$animation "gun_spin120_corrective_animation" "c_357_anims\gun_spin120_corrective_animation.smd" {
}

$animation "gun_spin240_corrective_animation" "c_357_anims\gun_spin240_corrective_animation.smd" {
}

$animation "a_adjustment_corrective_animation" "c_357_anims\a_adjustment_corrective_animation.smd" {
}

$animation "neutral" "c_357_anims\neutral.smd" {
	fps 30
}

$animation "gun_spin0" "c_357_anims\gun_spin0.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "gun_spin0_corrective_animation" 0
	weightlist "weights_gun_spinning"
}

$animation "gun_spin120" "c_357_anims\gun_spin120.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "gun_spin120_corrective_animation" 0
	weightlist "weights_gun_spinning"
}

$animation "gun_spin240" "c_357_anims\gun_spin240.smd" {
	fps 30
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "gun_spin240_corrective_animation" 0
	weightlist "weights_gun_spinning"
}

$animation "a_adjustment_ref" "c_357_anims\a_adjustment_ref.smd" {
	fps 30
}

$animation "a_adjustment" "c_357_anims\a_adjustment.smd" {
	fps 1
	loop
	// Crowbar writes this subtract option to prevent jigglebone and poseparameter problems when delta sequences are recompiled.
	subtract "a_adjustment_corrective_animation" 0
	weightlist "weights_adjustment"
}

$sequence "idle01" {
	"c_357_anims\idle01.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
	loop
}

$sequence "fire" {
	"c_357_anims\fire.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event AE_MUZZLEFLASH 0 "357 muzzle" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$sequence "draw" {
	"c_357_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	snap
	fps 30
}

$sequence "reload" {
	"c_357_anims\reload.smd"
	activity "ACT_VM_RELOAD" 1
	{ event AE_CL_PLAYSOUND 28 "Weapon_357.OpenLoader" }
	{ event AE_CL_PLAYSOUND 39 "Weapon_357.RemoveLoader" }
	{ event 3015 55 "" }
	{ event AE_CL_PLAYSOUND 67 "Weapon_357.ReplaceLoader" }
	{ event AE_CL_PLAYSOUND 92 "Weapon_357.Spin" }
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
}

$sequence "holster" {
	"c_357_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
}

$sequence "idletolow" {
	"c_357_anims\idletolow.smd"
	activity "ACT_VM_LOWERED_TO_IDLE" 1
	fadein 0.2
	fadeout 0.2
	transition "Ready" "Low"
	fps 30
}

$sequence "lowtoidle" {
	"c_357_anims\lowtoidle.smd"
	activity "ACT_VM_IDLE_TO_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	transition "Low" "Ready"
	fps 30
}

$sequence "lowidle" {
	"c_357_anims\lowidle.smd"
	activity "ACT_VM_IDLE_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	node "Low"
	fps 30
	loop
}

$sequence "reload_start" {
	"c_357_anims\reload_start.smd"
	activity "ACT_VM_RELOAD_START" 1
	{ event AE_CL_PLAYSOUND 28 "Weapon_357.OpenLoader" }
	{ event AE_CL_PLAYSOUND 39 "Weapon_357.RemoveLoader" }
	{ event 3015 55 "" }
	{ event AE_CL_PLAYSOUND 67 "Weapon_357.ReplaceLoader" }
	fadein 0.2
	fadeout 0
	node "Ready"
	fps 40
}

$sequence "reload_loop" {
	"c_357_anims\reload_loop.smd"
	activity "ACT_VM_RELOAD_LOOP" 1
	{ event AE_CL_PLAYSOUND 9 "Weapon_357.RemoveLoader" }
	fadein 0
	fadeout 0
	node "Ready"
	fps 40
}

$sequence "reload_end" {
	"c_357_anims\reload_end.smd"
	activity "ACT_VM_RELOAD_END" 1
	{ event AE_CL_PLAYSOUND 22 "Weapon_357.Spin" }
	fadein 0
	fadeout 0.2
	node "Ready"
	fps 30
}

$sequence "gun_spinning" {
	"gun_spin0"
	"gun_spin120"
	"gun_spin240"
	"gun_spin0"
	autoplay
	blend "gun_spin" 0 360
	blendwidth 4
	delta
	fadein 0.2
	fadeout 0.2
	hidden
}

$sequence "adjustment" {
	"a_adjustment"
	autoplay
	delta
	fadein 0.2
	fadeout 0.2
	hidden
}
