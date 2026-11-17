@ECHO OFF
pushd .
call vpk_build_shaders.bat
popd

set vpk="D:\SteamLibrary\steamapps\common\Source SDK Base 2013 Multiplayer\bin\x64\vpk.exe"
set name=bds_optional_assets_pak
set publickey=bds_base_pak.publickey.vdf
set privatekey=bds_base_pak.privatekey.vdf

if not exist "%CD%\%privatekey%" (%vpk% generate_keypair bds_base_pak)
if not exist "%CD%\%publickey%" (%vpk% generate_keypair bds_base_pak)

%vpk% -M -k %publickey% -K %privatekey% "%CD%\%name%"
%vpk% -k %publickey% checksig "%CD%\%name%.vpk"
pause