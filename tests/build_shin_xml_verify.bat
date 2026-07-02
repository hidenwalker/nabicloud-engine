@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-27 (F-3 R5 shin-oracle detach): the shin builtin oracle
REM   (nabicloud_shin_builtin_by_id) was removed; coverage moved to V2/editor shell self-
REM   roundtrip + build_cleanroom_selftests sel_r4xml/sel_rt + shell registry/V2Backend gates.
REM   Kept as historical oracle/archive only; NOT in run_all_gates GATES. See
REM   tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  NabiCloud Sebeolsik Sin (신세벌) external-XML byte-fidelity verifier build+run.
REM  Builds tests\shin_xml_verify.c against the NabiCloud engine tree, then runs
REM  it: generates shared\data\keyboards\3shin-*.xml from each builtin Sin oracle
REM  and full-field byte-diffs the reloaded XML keyboard against the oracle.
REM  Output is printed; tests\_shin_xml_verify.txt keeps a copy.
REM  Does NOT modify any builtin source. The 9 oracle structs stay in
REM  hangulkeyboard.c (reachable via nabicloud_shin_builtin_by_id).
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_shin_xml.rsp echo tests\shin_xml_verify.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_shin_xml.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_shin_xml.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\shin_xml_verify.exe /Fo:tests\ @tests\_srcs_shin_xml.rsp 1>tests\_build_shin_xml.log 2>&1
if errorlevel 1 (
  echo SHIN_XML_FAIL build error:
  type tests\_build_shin_xml.log
  popd ^& exit /b 1
)

REM outdir relative to engine ROOT: ..\data\keyboards  (engine root = shared\engine ;
REM body keyboards live at repo root\shared\data\keyboards = ..\data\keyboards)
tests\shin_xml_verify.exe "..\data\keyboards" > tests\_shin_xml_verify.txt 2>&1
type tests\_shin_xml_verify.txt
popd
exit /b 0
