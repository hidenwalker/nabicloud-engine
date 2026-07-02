@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-29 (F-3 R4 option 4): the engine XML loader was removed; this gate
REM   requires the old engine XML/editor path and is no longer meaningful for the target
REM   architecture; coverage = pristine-9 byte golden + V2/editor-shell gates. Kept as
REM   historical oracle/archive only; NOT in run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  NabiCloud keyboard-editor round-trip gate build+run.
REM  Builds tests\editor_roundtrip_verify.c against the NabiCloud engine tree
REM  (which now includes nabicloud\win32\editor_serialize.c and the new
REM  nabicloud_xml_parse_buffer in keyboard-loader.c), then runs it: serialize
REM  each builtin Sin oracle -> parse the buffer -> field-diff vs oracle +
REM  re-serialize idempotence. Does NOT modify any builtin source or data file.
REM  Output is printed; tests\_editor_roundtrip_verify.txt keeps a copy.
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_editor_rt.rsp echo tests\editor_roundtrip_verify.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_editor_rt.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_editor_rt.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\editor_roundtrip_verify.exe /Fo:tests\ @tests\_srcs_editor_rt.rsp 1>tests\_build_editor_rt.log 2>&1
if errorlevel 1 (
  echo EDITOR_ROUNDTRIP_FAIL build error:
  type tests\_build_editor_rt.log
  popd ^& exit /b 1
)

tests\editor_roundtrip_verify.exe > tests\_editor_roundtrip_verify.txt 2>&1
type tests\_editor_roundtrip_verify.txt
popd
exit /b 0
