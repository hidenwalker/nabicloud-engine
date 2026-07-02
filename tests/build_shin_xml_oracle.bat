@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-26 (sweep): libhangul-path shin oracle became meaningless after the shin
REM   XMLs moved to V2 (chord output empty); its successor build_shin_xml_verify was itself
REM   retired 2026-06-27 (F-3 R5). Kept as historical oracle/archive only; NOT in
REM   run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ★RETIRED (2026-06-26 전수정리) — libhangul-경로 신세벌 오라클. 신세벌 XML 이 V2(engine="v2")로
REM   이관된 뒤 chord 가 빈 출력 → 무의미. 현 정본 V2 신세벌-p2 오라클 = build_shin_xml_verify(게이트)
REM   + B2(build_cleanroom_selftests) 의 selftest_jaso_shinp2. provenance(CLEANROOM-baram.md) 보존
REM   위해 이동·재작성 안 함 — 실행 시 안내만 하고 종료(이하 원본은 보존, 미도달).
echo [RETIRED] build_shin_xml_oracle : libhangul-경로 오라클(신세벌 V2 이관 후 chord 빈출력).
echo [RETIRED]   현 정본 V2 오라클 = build_shin_xml_verify + cleanroom selftest_jaso_shinp2.
exit /b 0
REM ==========================================================================
REM  NabiCloud 신세벌 P2 behavioral oracle (XML-load path) build+run.
REM  Builds tests\diff_shin_xml.c against the NabiCloud engine tree, then runs it.
REM  The harness calls hangul_keyboard_list_load_dir("..\data\keyboards") FIRST so
REM  the external 3shin-*.xml register, then drives 3shin-p2 through the same suite
REM  as diff_shin.c (must reproduce shin-p2-port-fingerprint.txt) plus the spec
REM  anchors and the divergence-#2 lowercase right_oua probe.
REM  Output is printed; tests\_shin_xml_oracle.txt keeps a copy. Builds nothing
REM  into the engine; reads the committed (oracle-faithful) XML body keyboards.
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_shin_xml_oracle.rsp echo tests\diff_shin_xml.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_shin_xml_oracle.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_shin_xml_oracle.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\diff_shin_xml.exe /Fo:tests\ @tests\_srcs_shin_xml_oracle.rsp 1>tests\_build_shin_xml_oracle.log 2>&1
if errorlevel 1 (
  echo SHIN_XML_ORACLE_FAIL build error:
  type tests\_build_shin_xml_oracle.log
  popd ^& exit /b 1
)

tests\diff_shin_xml.exe "..\data\keyboards" "3shin-p2" > tests\_shin_xml_oracle.txt 2>&1
type tests\_shin_xml_oracle.txt
popd
exit /b 0
