@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-29 (F-3 R4 option 4): the engine XML loader was removed; this gate
REM   requires the old engine XML/editor path and is no longer meaningful for the target
REM   architecture; coverage = pristine-9 byte golden + V2/editor-shell gates. Kept as
REM   historical oracle/archive only; NOT in run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  NabiCloud Dubeolsik chord (두겹이) external-XML load verifier build+run.
REM  Builds tests\dubeol_xml_verify.c against the NabiCloud engine tree, then
REM  runs it: loads addons\dubeol\keyboards\dugyeobe.xml through the DLL loader
REM  and asserts it registers as DUBEOL_CHORD(1007) with the loose-order FLAG.
REM  Regression guard for the loader "dubeol-chord"->1007 mapping (keyboard-
REM  loader.c). Output is printed; tests\_dubeol_xml_verify.txt keeps a copy.
REM  Does NOT modify any builtin source.
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_dubeol_xml.rsp echo tests\dubeol_xml_verify.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_dubeol_xml.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_dubeol_xml.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\dubeol_xml_verify.exe /Fo:tests\ @tests\_srcs_dubeol_xml.rsp 1>tests\_build_dubeol_xml.log 2>&1
if errorlevel 1 (
  echo DUBEOL_XML_FAIL build error:
  type tests\_build_dubeol_xml.log
  popd ^& exit /b 1
)

REM addon keyboards dir relative to engine ROOT (shared\engine): ..\..\addons\dubeol\keyboards
tests\dubeol_xml_verify.exe "..\..\addons\dubeol\keyboards" > tests\_dubeol_xml_verify.txt 2>&1
set "RC=!errorlevel!"
type tests\_dubeol_xml_verify.txt
REM [2026-06-26 Codex] capture RC right after exe (not type) + require ALL_PASS token (block false-green).
findstr /C:"DUBEOL_XML_ALL_PASS" tests\_dubeol_xml_verify.txt >nul 2>&1 || set "RC=1"
popd & exit /b !RC!
