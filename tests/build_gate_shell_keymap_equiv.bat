@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\windows\tsf" ( echo [SKIP] build_gate_shell_keymap_equiv: windows/tsf/ absent - body-level INTEG gate & exit /b 77 )
if not exist "%~dp0..\..\data\keyboards" ( echo [SKIP] build_gate_shell_keymap_equiv: shared/data/keyboards/ absent - body-level INTEG gate & exit /b 77 )
REM ==========================================================================
REM  F-3 R2-2 gate: build+run gate_shell_keymap_equiv -- assert the committed
REM  windows/tsf/nabicloud_builtin_keymaps.h still byte-equals the engine's
REM  hangul_keyboard_map_to_char. Regen header via dump_builtin_keymaps.bat.
REM  R4: builtin-only; compiles only libhangul 0.2.0 core files, with
REM  ENABLE_EXTERNAL_KEYBOARDS unset. cwd-independent (absolute %~dp0).
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
set "TSF=%~dp0..\..\..\windows\tsf"
pushd "%ROOT%"

> tests\_srcs_keymap_equiv.rsp echo tests\gate_shell_keymap_equiv.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_keymap_equiv.rsp echo %%f

set "INC=/I hangul /I "%TSF%""

cl /nologo /W3 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\gate_shell_keymap_equiv.exe /Fo:tests\ @tests\_srcs_keymap_equiv.rsp 1>tests\_build_keymap_equiv.log 2>&1
if errorlevel 1 (
  echo GATE_SHELL_KEYMAP_EQUIV_FAIL build error:
  type tests\_build_keymap_equiv.log
  popd
  exit /b 1
)

REM kbdir relative to engine ROOT (shared\engine): ..\data\keyboards (external 3-89/3sun-1990)
tests\gate_shell_keymap_equiv.exe "..\data\keyboards" > tests\_keymap_equiv.txt 2>&1
set "RC=!errorlevel!"
type tests\_keymap_equiv.txt
findstr /C:"GATE_SHELL_KEYMAP_EQUIV_PASS" tests\_keymap_equiv.txt >nul 2>&1 || set "RC=1"
popd & exit /b !RC!
