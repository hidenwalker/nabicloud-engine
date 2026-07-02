@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-29 (F-3 R4 option 4): the engine XML loader was removed; this gate
REM   requires the old engine XML/editor path and is no longer meaningful for the target
REM   architecture; coverage = pristine-9 byte golden + V2/editor-shell gates. Kept as
REM   historical oracle/archive only; NOT in run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  NabiCloud Semo-e external-XML fidelity verifier build+run.
REM  Builds tests\semoe_xml_verify.c against the NabiCloud engine tree, then
REM  runs it: generates addons\semoe\keyboards\*.xml from each builtin Semo-e
REM  keyboard and diffs the reloaded XML keyboard against the builtin field by
REM  field. Output is printed; tests\_semoe_xml_verify.txt keeps a copy.
REM  Does NOT modify any builtin source.
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_semoe_xml.rsp echo tests\semoe_xml_verify.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_semoe_xml.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_semoe_xml.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\semoe_xml_verify.exe /Fo:tests\ @tests\_srcs_semoe_xml.rsp 1>tests\_build_semoe_xml.log 2>&1
if errorlevel 1 (
  echo SEMOE_XML_FAIL build error:
  type tests\_build_semoe_xml.log
  popd ^& exit /b 1
)

REM ★[2026-06-26 세모이 V2 이관] addons\semoe\keyboards 의 세모이는 이제 V2 확장 XML
REM   (engine="v2" type="sebeol-chord")이라 libhangul-body 오라클(table[0]/combination[0])이
REM   없다 → 이 읽기전용 검증을 보존된 libhangul-body 원본(addons\semoe\oracle)으로 repoint한다
REM   (V2 keyboards\ 무간섭, 오라클 byte-충실 게이트는 그대로 green). 런타임은 V2 로더가 구동.
tests\semoe_xml_verify.exe "..\..\addons\semoe\oracle" > tests\_semoe_xml_verify.txt 2>&1
set "RC=!errorlevel!"
type tests\_semoe_xml_verify.txt
REM [2026-06-26 Codex] exe may return 0 even on FAIL -> require ALL_PASS token (block false-green).
findstr /C:"SEMOE_XML_ALL_PASS" tests\_semoe_xml_verify.txt >nul 2>&1 || set "RC=1"
popd & exit /b !RC!
