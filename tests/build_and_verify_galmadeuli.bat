@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-27 (F-3 R4 de-fork): fork builtins 2noshift/2n9256 (galmadeuli) were
REM   removed by the pristine libhangul 0.2.0 rebase, so this old-engine golden is
REM   unreproducible; coverage moved to V2 compiled-in builtins + build_cleanroom_selftests.
REM   Kept as historical oracle/archive only; NOT in run_all_gates GATES. See
REM   tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  libhangul-nabicloud galmadeuli (Dubeolsik Noshift / North-9256) gate:
REM  build the galmadeuli differential harness against the NabiCloud engine
REM  tree, run it for 2noshift and 2n9256, and diff each against the 3beol
REM  reference baselines frozen in tests\golden_galmadeuli*.txt.
REM
REM  The reference baselines were produced by compiling THE SAME tests\
REM  golden_galmadeuli.c against the upstream 3beol clone (yous/libhangul,
REM  gureum-1.11.1) and are committed. This gate proves our port's output ==
REM  the 3beol reference output for identical key sequences.
REM
REM  PASS  -> prints GALMADEULI_PASS, exit code 0
REM  FAIL  -> prints GALMADEULI_FAIL (build error or behavior diff), exit code 1
REM
REM  Sources are auto-collected like build_and_verify.bat:
REM    - tests\golden_galmadeuli.c
REM    - hangul\*.c   EXCEPT hanja.c
REM    - nabicloud\**\*.c
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_galm.rsp echo tests\golden_galmadeuli.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_galm.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_galm.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\golden_galmadeuli.exe /Fo:tests\ @tests\_srcs_galm.rsp 1>tests\_build_galm.log 2>&1
if errorlevel 1 (
  echo GALMADEULI_FAIL build error:
  type tests\_build_galm.log
  popd & exit /b 1
)

REM --- 2noshift ---
tests\golden_galmadeuli.exe 2noshift > tests\golden_galmadeuli.new.txt
if not exist tests\golden_galmadeuli.txt (
  echo GALMADEULI_FAIL no baseline tests\golden_galmadeuli.txt
  popd & exit /b 1
)
fc /n tests\golden_galmadeuli.txt tests\golden_galmadeuli.new.txt >tests\_diff_galm.log
if errorlevel 1 (
  echo GALMADEULI_FAIL 2noshift behavior diff vs 3beol reference ^(see tests\_diff_galm.log^):
  type tests\_diff_galm.log
  popd & exit /b 1
)

REM --- 2n9256 ---
tests\golden_galmadeuli.exe 2n9256 > tests\golden_galmadeuli_2n9256.new.txt
if not exist tests\golden_galmadeuli_2n9256.txt (
  echo GALMADEULI_FAIL no baseline tests\golden_galmadeuli_2n9256.txt
  popd & exit /b 1
)
fc /n tests\golden_galmadeuli_2n9256.txt tests\golden_galmadeuli_2n9256.new.txt >tests\_diff_galm_9256.log
if errorlevel 1 (
  echo GALMADEULI_FAIL 2n9256 behavior diff vs 3beol reference ^(see tests\_diff_galm_9256.log^):
  type tests\_diff_galm_9256.log
  popd & exit /b 1
)

echo GALMADEULI_PASS
popd & exit /b 0
