@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-25 (V2 migration): 318na now ships as V2 XML and golden_318na is an old-
REM   engine oracle that cannot read the V2 jaso-map body; coverage moved to cleanroom
REM   selftest_jaso_318na + selftest_jaso_roundtrip. Kept as historical oracle/archive only;
REM   NOT in run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  NabiCloud 318na FUNCTIONAL golden gate (clean-room, 2026-06-20).
REM  Builds tests\golden_318na.c against the NabiCloud engine tree, loads the
REM  EXTERNAL 318na XML keyboard, drives fixed key sequences, and checks each
REM  composed NFC syllable against the spec-derived expected value (codepoints
REM  baked into golden_318na.c).
REM
REM  PASS -> prints golden_318na PASS + 318NA_PASS, exit 0.
REM  FAIL -> prints the failing case(s) + 318NA_FAIL, exit 1.
REM
REM  ★[2026-06-25 V2 이관 — RETIRED from CI] 318na 는 이제 V2 확장 XML(engine="v2",
REM    role-flip="jongcycle")로 keyboards\318na.xml 에서 산들바람 V2 엔진으로 구동된다.
REM    이 게이트는 *구엔진*(libhangul + nabicloud jongseong_cycle 전처리) 오라클이라
REM    V2 본문(<jaso-map>)을 읽지 못한다 → run_all_gates.bat GATES 에서 제거(은퇴).
REM    기능 커버리지는 V2 로 이전: cleanroom selftest_jaso_318na(73 checks, oracle-anchored,
REM    이 골든 12케이스 전부 포함) + selftest_jaso_roundtrip(C≡XML). 이 .bat 는 구엔진
REM    클린룸 오라클 *재현기*로 보존하되 KBDIR 을 보존본(addons\318na\oracle\318na.xml,
REM    libhangul-body 원본)으로 가리켜 수동 재구동만 가능하게 둔다.
REM
REM  Sources auto-collected like build_and_verify_all.bat:
REM    - tests\golden_318na.c
REM    - hangul\*.c   EXCEPT hanja.c
REM    - nabicloud\**\*.c
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

REM ★V2 이관 후 keyboards\318na.xml 은 V2 확장 XML 이라 구엔진 오라클이 못 읽는다.
REM   보존된 libhangul-body 원본(addons\318na\oracle\318na.xml)을 가리킨다(은퇴 재현기).
set "KBDIR=%ROOT%\..\..\addons\318na\oracle"

> tests\_srcs_318na.rsp echo tests\golden_318na.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_318na.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_318na.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\golden_318na.exe /Fo:tests\ @tests\_srcs_318na.rsp 1>tests\_build_318na.log 2>&1
if errorlevel 1 (
  echo 318NA_FAIL build error:
  type tests\_build_318na.log
  popd & exit /b 1
)

tests\golden_318na.exe "%KBDIR%"
if errorlevel 1 (
  echo 318NA_FAIL functional golden mismatch
  popd & exit /b 1
)

echo 318NA_PASS
popd & exit /b 0
