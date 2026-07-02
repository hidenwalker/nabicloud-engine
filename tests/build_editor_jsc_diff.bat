@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-29 (F-3 R4 option 4): the engine XML loader was removed; this gate
REM   requires the old engine XML/editor path and is no longer meaningful for the target
REM   architecture; coverage = pristine-9 byte golden + V2/editor-shell gates. Kept as
REM   historical oracle/archive only; NOT in run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  JS<->C serializer differential gate.
REM  (1) Build editor_jsc_dump.c against the engine tree -> dump each of the 9
REM      Sin builtin oracles' C canonical serialization (editor_serialize.c) to
REM      tests\_jsc\<id>.xml (ephemeral, not committed).
REM  (2) node tests\jsc-diff.js checks the JS core (editor-core.js) parseXml->
REM      serialize reproduces those bytes exactly (JS format == C format).
REM      node absent -> SKIP (not a failure). Engine/builtins/data untouched.
REM  ASCII-only harness, mirroring build_editor_roundtrip_verify.bat.
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "HERE=%~dp0"
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_jsc.rsp echo tests\editor_jsc_dump.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_jsc.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_jsc.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\editor_jsc_dump.exe /Fo:tests\ @tests\_srcs_jsc.rsp 1>tests\_build_jsc.log 2>&1
if errorlevel 1 (
  echo JSC_DIFF_FAIL build error:
  type tests\_build_jsc.log
  popd ^& exit /b 1
)

if not exist tests\_jsc mkdir tests\_jsc
del /q tests\_jsc\*.xml 2>nul
tests\editor_jsc_dump.exe
if errorlevel 1 ( echo JSC_DIFF_FAIL dump error & popd ^& exit /b 1 )

node --version >nul 2>&1
if errorlevel 1 ( echo   [SKIP] JSC diff ^(node not installed^) & popd ^& exit /b 0 )
node "%HERE%jsc-diff.js"
if errorlevel 1 ( popd ^& exit /b 1 )

popd
exit /b 0
