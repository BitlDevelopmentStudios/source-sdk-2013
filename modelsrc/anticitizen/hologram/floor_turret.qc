// Created by Crowbar 0.74

$modelname "Combine_turrets\Floor_turret_hologram.mdl"

$bodygroup "studio"
{
	studio "floor_turret_Floor_reference.smd"
}

$lod 12
{
	replacemodel "floor_turret_Floor_reference.smd" "floor_turret_Floor_reference_lod1.smd"
	nofacial
}
$lod 15
{
	replacemodel "floor_turret_Floor_reference.smd" "floor_turret_Floor_reference_lod2.smd"
	nofacial
}
$lod 20
{
	replacemodel "floor_turret_Floor_reference.smd" "floor_turret_Floor_reference_lod3.smd"
	nofacial
}
$lod 30
{
	replacemodel "floor_turret_Floor_reference.smd" "floor_turret_Floor_reference_lod4.smd"
	nofacial
}

$surfaceprop "metal"

$contents "solid"

$illumposition 0.805 -1.718 29.958

$eyeposition 0 0 64

$cdmaterials "models\Combine_turrets\Floor_turret\"

$cbox 0 0 0 0 0 0

$bbox -16 -16 0 16 16 64

$definebone "Base" "" 0.748472 0.238418 17.889942 0 0 89.999982 0 0 0 0 0 0
$definebone "Leftright" "Base" 0.332457 15.529396 -0.220379 0 0 0 0 0 0 0 0 0
$definebone "Gun" "Leftright" 2.006938 17.841654 7.452806 0 0 0 0 0 0 0 0 0
$definebone "Barrel" "Gun" 1.671227 1.473717 3.726596 0 0 0 0 0 0 0 0 0
$definebone "cover" "Gun" 2.516212 0.478182 -0.870788 0 0 0 0 0 0 0 0 0

$sequence "idle" {
	"floor_turret_anims\idle.smd"
	activity "ACT_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$collisionmodel "floor_turret_physics.smd"
{
	$mass 100
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave
	$maxconvexpieces 5
}
