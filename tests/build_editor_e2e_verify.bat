@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-29 (F-3 R4 option 4): the engine XML loader was removed; this gate
REM   requires the old engine XML/editor path and is no longer meaningful for the target
REM   architecture; coverage = pristine-9 byte golden + V2/editor-shell gates. Kept as
REM   historical oracle/archive only; NOT in run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  NabiCloud keyboard-editor END-TO-END runtime gate build+run.
REM  Builds tests\editor_e2e_verify.c against the NabiCloud engine tree, then
REM  runs it: serialize an oracle as 'editor output' -> write to a %TEMP% dir ->
REM  hangul_keyboard_list_load_dir (the loader WU1 uses) -> the keyboard becomes
REM  selectable -> hangul_ic compose -> expected syllable. No DLL registration,
REM  no data-file writes (repo stays clean; only %TEMP% touched, cleaned up).
REM  Output is printed; tests\_editor_e2e_verify.txt keeps a copy.
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_editor_e2e.rsp echo tests\editor_e2e_verify.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_editor_e2e.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_editor_e2e.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\editor_e2e_verify.exe /Fo:tests\ @tests\_srcs_editor_e2e.rsp 1>tests\_build_editor_e2e.log 2>&1
if errorlevel 1 (
  echo EDITOR_E2E_FAIL build error:
  type tests\_build_editor_e2e.log
  popd ^& exit /b 1
)

tests\editor_e2e_verify.exe > tests\_editor_e2e_verify.txt 2>&1
type tests\_editor_e2e_verify.txt
popd
exit /b 0
