@ECHO OFF
set vpk="D:\SteamLibrary\steamapps\common\Source SDK Base 2013 Multiplayer\bin\x64\vpk.exe"
set /p name=<name.txt
set publickey=%name%.publickey.vdf
set privatekey=%name%.privatekey.vdf

if not exist "%CD%\%privatekey%" (%vpk% generate_keypair %name%)
if not exist "%CD%\%publickey%" (%vpk% generate_keypair %name%)

%vpk% -M -k %publickey% -K %privatekey% "%CD%\%name%"
%vpk% -k %publickey% checksig "%CD%\%name%.vpk"
pause