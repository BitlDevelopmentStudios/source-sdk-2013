::===================== File of the LUX Shader Project =====================::
::  - Initial D.  : 03.04.2025                                              ::
::  - Last Change : 29.01.2026                                              ::
::==========================================================================::
@echo off

rem sourcedir = Where to compile Shaders to ( shaders\ )
rem targetdir = Where to copy compiled Shaders to ( game\..\shaders\ )
set sourcedir="shaders"
set targetdir="..\..\..\game\mod_tf\shaders"

rem SOURCE_DIR = Source Code Root ( src\ )
rem GAME_DIR = Mod Folder with GameInfo.txt
set SOURCE_DIR="..\..\"
set GAME_DIR="..\..\..\game\mod_tf"

rem Process Names we will try to call to ask for Shader reloads
rem Will check in Order
set PROCESS_LIST=

set PROCESS_LIST=%PROCESS_LIST% hl2_win64.exe
set PROCESS_LIST=%PROCESS_LIST% mod_tf_win64.exe
set PROCESS_LIST=%PROCESS_LIST% mod_hl2mp_win64.exe