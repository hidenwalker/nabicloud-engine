@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-29 (F-3 R4 option 4): the engine XML loader was removed; this gate
REM   requires the old engine XML/editor path and is no longer meaningful for the target
REM   architecture; coverage = pristine-9 byte golden + V2/editor-shell gates. Kept as
REM   historical oracle/archive only; NOT in run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  NabiCloud Dugyeob-e auto cluster-jong self-test build+run (DUBEOL_CHORD s4).
REM  Builds tests\dubeol_jong_selftest.c against the NabiCloud engine tree, loads
REM  the real Dugyeob-e keyboard (addons\dubeol\keyboards\dugyeobe.xml, type 1007)
REM  and drives ASCII keys through hangul_ic_process -> ops_dubeol_chord_process to
REM  check the I/J/K/L -> cluster-jong auto-transform (diphthong-priority included).
REM  PASS -> DUBEOL_JONG_SELFTEST_PASS (exit 0); build/assert error -> exit 1.
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_dubeol_jong.rsp echo tests\dubeol_jong_selftest.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_dubeol_jong.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_dubeol_jong.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\dubeol_jong_selftest.exe /Fo:tests\ @tests\_srcs_dubeol_jong.rsp 1>tests\_build_dubeol_jong.log 2>&1
if errorlevel 1 (
  echo DUBEOL_JONG_BUILD_FAIL:
  type tests\_build_dubeol_jong.log
  popd ^& exit /b 1
)

tests\dubeol_jong_selftest.exe "..\..\addons\dubeol\keyboards"
set "RC=!errorlevel!"
popd & exit /b !RC!
