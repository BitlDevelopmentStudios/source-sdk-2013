// Created by Crowbar 0.74

$modelname "weapons/w_manhack.mdl"

$bodygroup "studio"
{
	studio "w_manhac_rb_w_manhac_ref_rb.smd"
}

$lod 35
{
	replacemodel "w_manhac_rb_w_manhac_ref_rb.smd" "w_manhac_rb_w_manhac_ref_rb_lod1.smd"
	nofacial
}
$lod 40
{
	replacemodel "w_manhac_rb_w_manhac_ref_rb.smd" "w_manhac_rb_w_manhac_ref_rb_lod2.smd"
	nofacial
}

$surfaceprop "flesh"

$contents "solid"

$illumposition 2.098 0.03 1.162

$cdmaterials "models\manhack\"

$cbox 0 0 0 0 0 0

$bbox -7.102 -5.841 -7.385 7.042 10.037 9.709

$definebone "ValveBiped" "" 0 0 0 0 0 89.999982 0 0 0 0 0 0
$definebone "ValveBiped.Bip01_R_Hand" "ValveBiped" -2.297683 3.432066 -2.858664 -72.842533 73.811232 19.400867 0 0 0 0 0 0
$definebone "ValveBiped.bone" "ValveBiped.Bip01_R_Hand" 1.047174 -1.639375 -1.296265 10.480142 -65.386699 -79.597242 0 0 0 0 0 0


$sequence "idle" {
	"w_manhac_rb_anims\idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
}

$collisionmodel "w_manhac_rb_w_manhac_ref_rb.smd"
{
	$mass 200
	$inertia 1
	$damping 1
	$rotdamping 0
	$rootbone "ValveBiped"
}