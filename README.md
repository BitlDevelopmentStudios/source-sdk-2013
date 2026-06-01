# Source SDK 2013 (BDS Version)
<img src="https://github.com/BitlDevelopmentStudios/source-sdk-2013/blob/master/bdsbase.png" alt="Logo" width="450" height="450">

This is a Source SDK 2013 fork made for the purpose of giving a reliable mod base for currently updated and future Bitl Development Studio (BDS) mod projects.
This was based off the TF2/64-bit SDK release, and will be updated as the SDK updates.

This will feature sources for some currently updated and future Bitl Development Studio (BDS) mod projects. 
This base is specific to multiplayer mod projects.

LOOKING FOR THE OLD BDS BASE WITH QUIVER FORTRESS' SOURCE CODE? Go here: https://github.com/BitlDevelopmentStudios/source-sdk-2013-bds-base

## Projects using this base:
- SURVIVOR II (Based on HL2DM, Shelved)
- ANTICITIZEN ONE (Based on HL2DM, Active Development, anticitizen-one branch.) 

## Features:
- Implements various pull requests from the master repo, allowing for a stable and reliable mod base. Each pull request is squished and merged, ensuring credit is given to each author.
- Implements Tony Sergi's HL2MP changes which implement improved player animations and better code consistency (https://github.com/tonysergi/source-sdk-2013/).
- Implements changes from Saul's repository, allowing for features like autocomplete for the "give" command, colored messages in chat, and FGD fixes for compilers (https://github.com/saul/source-sdk-2013).
- Implements changes from the Source Engine Co-Operative Base Modification (SecobMod), which includes Winston's multiplayer NPC support (https://developer.valvesoftware.com/wiki/Co-Operative_Base_(Mod)).
- Implemented Discord RPC support (based off #1803).
- Built-in c_arms support. Just add `$Include "$SRCDIR\game\shared\c_arms_include.vpc"` to the top of your VPC file and set it up! Based on https://developer.valvesoftware.com/wiki/Hand_Viewmodels
- reset.bat file in each mod folder, used for cleaning up config/temporary files for easy mod distribution.
- Server Admin tools support (based off #948) with extendable module-based command interface.
- Animated Avatars support (based off #1380)
- Serverfinder/Quick Search: A quick and easy way for players to start and join servers.
- Supports Xbox 360 achievement notifications.
- Full support with the HL2 Survivor animation set.
- LUX Shaders are pre-implemented, which includes fixes and improvements for all shaders. Read README_LUX.md for more information. (https://github.com/LUX-Shaders-Team/LUX-Shaders)

## Additional Credits;
The Mapbase team for MP fixes as well as their implementation of Tony Sergi's, Saul's, and SecobMod's code as well as fixes for aformentioned code.
The SecobMod implementation uses code from dutchmega's Collaborate mod (https://developer.valvesoftware.com/wiki/Collaborate)
Iván Bravo Bravo for the NPC AI node generation code.
Sirmasters from BSNOOCH, for making the Mapadd code. (Part of SMMOD)
Zeldaboy14 and ficool2 for their LUX Shaders fix.
This repository implements fixes the VDC, including:
https://developer.valvesoftware.com/wiki/General_SDK_Snippets_%26_Fixes
https://developer.valvesoftware.com/wiki/Detail_props/Aspect_ratio_fix
https://developer.valvesoftware.com/wiki/Env_projectedtexture/fixes

## Setup:
Read Autumn/Misyl's setup guide at README_FROG.md for detailed setup.
You may also read a more detailed guide here:
https://developer.valvesoftware.com/wiki/Source_SDK_2013