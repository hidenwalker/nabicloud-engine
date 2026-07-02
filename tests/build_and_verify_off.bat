@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-29 (F-3 R8 engine-layer drop): golden_off.c linked baram.c to assert
REM   'engine off == pristine libhangul'; with the 8 old engine-layer TUs dropped the gate
REM   cannot link; coverage kept by build_and_verify_all + V2 cleanroom selftests. Kept as
REM   historical oracle/archive only; NOT in run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  NabiCloud master-gate OFF (gu-engine = vanilla) self-check.
REM  Builds tests\golden_off.c against the NabiCloud engine tree and runs it.
REM  Seals the OFF (nabicloud_baram_enabled()==false) path that the default-ON
REM  flip left untested: standard kbd ON==OFF (gate byte-neutral) + 3gs ON!=OFF
REM  (gate disables NabiCloud). (2026-06-21 verification D.)
REM  PASS -> OFF_PASS (exit 0); build/assert error -> exit 1.
REM
REM  Sources auto-collected like build_and_verify_dubeol_chord.bat:
REM    - tests\golden_off.c
REM    - hangul\*.c   EXCEPT hanja.c
REM    - nabicloud\**\*.c   (picks up baram.c automatically; engine-ops.c is gone)
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_off.rsp echo tests\golden_off.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_off.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_off.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\golden_off.exe /Fo:tests\ @tests\_srcs_off.rsp 1>tests\_build_off.log 2>&1
if errorlevel 1 (
  echo OFF_BUILD_FAIL:
  type tests\_build_off.log
  popd & exit /b 1
)

tests\golden_off.exe
set "RC=!errorlevel!"
popd & exit /b !RC!
