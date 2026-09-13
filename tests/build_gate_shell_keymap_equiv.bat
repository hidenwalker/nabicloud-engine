@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\windows\tsf" ( echo [SKIP] build_gate_shell_keymap_equiv: windows/tsf/ absent - body-level INTEG gate & exit /b 77 )
if not exist "%~dp0..\..\data\keyboards" ( echo [SKIP] build_gate_shell_keymap_equiv: shared/data/keyboards/ absent - body-level INTEG gate & exit /b 77 )
REM ==========================================================================
REM  F-3 R2-2 gate: build+run gate_shell_keymap_equiv -- assert the committed
REM  shared/input/include/nabicloud_builtin_keymaps.h still byte-equals the engine's
REM  hangul_keyboard_map_to_char. Regen header via dump_builtin_keymaps.bat.
REM  R4: builtin-only; compiles only libhangul 0.2.0 core files, with
REM  ENABLE_EXTERNAL_KEYBOARDS unset. cwd-independent (absolute %~dp0).
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
set "TSF=%~dp0..\..\input\include"
set "OUT=%ROOT%\_build\tests\keymap_equiv"
if not exist "%OUT%" mkdir "%OUT%"
if not exist "%OUT%" exit /b 1
pushd "%OUT%" || exit /b 1

> "%OUT%\sources.rsp" echo "%ROOT%\tests\gate_shell_keymap_equiv.c"
for %%f in ("%ROOT%\hangul\*.c") do if /i not "%%~nxf"=="hanja.c" >> "%OUT%\sources.rsp" echo "%%~ff"

set "INC=/I "%ROOT%\hangul" /I "%TSF%""

cl /nologo /W3 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:gate_shell_keymap_equiv.exe /Fo:"%OUT%\\" @"%OUT%\sources.rsp" 1>"%OUT%\build.log" 2>&1
if errorlevel 1 (
  echo GATE_SHELL_KEYMAP_EQUIV_FAIL build error:
  type "%OUT%\build.log"
  popd
  exit /b 1
)

REM kbdir relative to engine ROOT (shared\engine): ..\data\keyboards (external 3-89/3sun-1990)
gate_shell_keymap_equiv.exe "%ROOT%\..\data\keyboards" > "%OUT%\result.txt" 2>&1
set "RC=!errorlevel!"
type "%OUT%\result.txt"
findstr /C:"GATE_SHELL_KEYMAP_EQUIV_PASS" "%OUT%\result.txt" >nul 2>&1 || set "RC=1"
popd & exit /b !RC!
