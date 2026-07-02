@echo off
REM ==========================================================================
REM  libhangul-nabicloud golden gate: build the 3gs characterization harness,
REM  run it, and diff against the frozen tests\golden.txt baseline.
REM
REM  PASS  -> prints GOLDEN_PASS, exit code 0
REM  FAIL  -> prints GOLDEN_FAIL (build error or behavior diff), exit code 1
REM
REM  Sources are auto-collected so the restructure can add nabicloud\**\*.c
REM  without editing this script:
REM    - tests\golden.c
REM    - hangul\*.c   EXCEPT hanja.c (not needed; uses strtok_r)
REM    - nabicloud\**\*.c   (once that tree exists)
REM  Every behavior-preserving step MUST keep this PASS.
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

REM --- collect sources into a response file (handles paths with spaces) ---
> tests\_srcs.rsp echo tests\golden.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\golden.exe /Fo:tests\ @tests\_srcs.rsp 1>tests\_build.log 2>&1
if errorlevel 1 (
  echo GOLDEN_FAIL build error:
  type tests\_build.log
  popd & exit /b 1
)

tests\golden.exe > tests\golden.new.txt
if not exist tests\golden.txt (
  echo GOLDEN_FAIL no baseline tests\golden.txt
  popd & exit /b 1
)

fc /n tests\golden.txt tests\golden.new.txt >tests\_diff.log
if errorlevel 1 (
  echo GOLDEN_FAIL behavior diff ^(see tests\_diff.log^):
  type tests\_diff.log
  popd & exit /b 1
)

echo GOLDEN_PASS
popd & exit /b 0
