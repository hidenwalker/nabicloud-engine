@echo off
REM ==========================================================================
REM  F-3 R2-2 de-fork: regenerate windows/tsf/nabicloud_builtin_keymaps.h from
REM  the engine's hangul_keyboard_map_to_char (build-time only). Golden-frozen:
REM  the header is committed; gate_shell_keymap_equiv asserts it still matches.
REM  Does NOT modify any builtin source. cwd-independent (absolute via %~dp0).
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
set "OUT=%~dp0..\..\..\windows\tsf\nabicloud_builtin_keymaps.h"
pushd "%ROOT%"

> tests\_srcs_dump_keymaps.rsp echo tests\dump_builtin_keymaps.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_dump_keymaps.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_dump_keymaps.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\dump_builtin_keymaps.exe /Fo:tests\ @tests\_srcs_dump_keymaps.rsp 1>tests\_build_dump_keymaps.log 2>&1
if errorlevel 1 (
  echo DUMP_KEYMAP_FAIL build error:
  type tests\_build_dump_keymaps.log
  popd ^& exit /b 1
)

tests\dump_builtin_keymaps.exe > "%OUT%" 2>tests\_dump_keymaps.err
set "RC=!errorlevel!"
type tests\_dump_keymaps.err
if !RC! NEQ 0 ( echo DUMP_KEYMAP_FAIL run error & popd ^& exit /b !RC! )
echo DUMP_KEYMAP_WROTE "%OUT%"
popd & exit /b 0
