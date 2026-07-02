@echo off
REM ==========================================================================
REM  NabiCloud all-keyboards characterization gate (v2 W0).
REM  Builds tests\golden_all.c against the NabiCloud engine tree, runs it over
REM  every registered built-in keyboard x option axis, and diffs stdout against
REM  the frozen baseline tests\golden_all.txt.
REM
REM  First run (no baseline yet): creates tests\golden_all.txt and prints
REM    ALL_BASELINE_CREATED. Commit that file as the frozen oracle.
REM  Later runs: byte-diff. PASS -> ALL_PASS (exit 0); diff/build error ->
REM    ALL_FAIL (exit 1).
REM
REM  This baseline is the cross-engine equivalence oracle: when the future
REM  clean-slate engine is shadowed in, rebuild against it and this gate proves
REM  new-engine output == old-engine output for identical key sequences.
REM
REM  Sources auto-collected like build_and_verify_galmadeuli.bat:
REM    - tests\golden_all.c
REM    - hangul\*.c   EXCEPT hanja.c
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_all.rsp echo tests\golden_all.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_all.rsp echo %%f

set "INC=/I hangul"

cl /nologo /W3 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\golden_all.exe /Fo:tests\ @tests\_srcs_all.rsp 1>tests\_build_all.log 2>&1
if errorlevel 1 (
  echo ALL_FAIL build error:
  type tests\_build_all.log
  popd & exit /b 1
)

tests\golden_all.exe > tests\golden_all.new.txt

if not exist tests\golden_all.txt (
  copy /y tests\golden_all.new.txt tests\golden_all.txt >nul
  echo ALL_BASELINE_CREATED tests\golden_all.txt
  popd & exit /b 0
)

fc /n tests\golden_all.txt tests\golden_all.new.txt >tests\_diff_all.log
if errorlevel 1 (
  echo ALL_FAIL behavior diff ^(see tests\_diff_all.log^):
  type tests\_diff_all.log
  popd & exit /b 1
)

echo ALL_PASS
popd & exit /b 0
