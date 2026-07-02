@echo off
REM ============================ RETIRED (not a live gate) ============================
REM   Retired 2026-06-27 (F-3 R4 de-fork): fork builtin 3-91-noshift (finalsun) was removed by
REM   the pristine libhangul 0.2.0 rebase and is dropped (not in pristine 9), so there is no
REM   V2 replacement; dead archive. Kept as historical oracle/archive only; NOT in
REM   run_all_gates GATES. See tests/RETIRED.md.
REM ===================================================================================
REM ==========================================================================
REM  libhangul-nabicloud 3finalsun (Sebeolsik 3-91 Final Noshift) gate:
REM  build the 3finalsun differential harness against the NabiCloud engine
REM  tree, run it for 3-91-noshift, and diff against the 3beol reference
REM  baseline frozen in tests\golden_finalsun.txt.
REM
REM  The reference baseline was produced by compiling THE SAME tests\
REM  golden_finalsun.c against the upstream 3beol clone (yous/libhangul,
REM  gureum-1.11.1, HEAD bb8fcb3) and is committed. This gate proves our port's
REM  output == the 3beol reference output for identical key sequences.
REM
REM  PASS  -> prints FINALSUN_PASS, exit code 0
REM  FAIL  -> prints FINALSUN_FAIL (build error or behavior diff), exit code 1
REM
REM  Sources are auto-collected like build_and_verify.bat:
REM    - tests\golden_finalsun.c
REM    - hangul\*.c   EXCEPT hanja.c
REM    - nabicloud\**\*.c
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
pushd "%ROOT%"

> tests\_srcs_fs.rsp echo tests\golden_finalsun.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_fs.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_fs.rsp echo "%%f"

set "INC=/I hangul"
if exist nabicloud set "INC=!INC! /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"

cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\golden_finalsun.exe /Fo:tests\ @tests\_srcs_fs.rsp 1>tests\_build_fs.log 2>&1
if errorlevel 1 (
  echo FINALSUN_FAIL build error:
  type tests\_build_fs.log
  popd & exit /b 1
)

REM --- 3-91-noshift : (1) frozen 3beol-reference baseline (legacy gate) ---
REM  Default mode = "old" (baram flag OFF) -> nabicloud_engine_finalsun_process,
REM  the SAME path the frozen baseline was captured from. Byte-identical proof
REM  that our port still == the 3beol reference.
tests\golden_finalsun.exe 3-91-noshift old > tests\golden_finalsun.new.txt
if not exist tests\golden_finalsun.txt (
  echo FINALSUN_FAIL no baseline tests\golden_finalsun.txt
  popd & exit /b 1
)
fc /n tests\golden_finalsun.txt tests\golden_finalsun.new.txt >tests\_diff_fs.log
if errorlevel 1 (
  echo FINALSUN_FAIL 3-91-noshift behavior diff vs 3beol reference ^(see tests\_diff_fs.log^):
  type tests\_diff_fs.log
  popd & exit /b 1
)

REM --- 3-91-noshift : (2) Stage D-1 old-vs-new equivalence (raw jamo bytes) ---
REM  old = baram flag OFF -> nabicloud_engine_finalsun_process
REM  new = baram flag ON  -> baram_process_finalsun (production dispatcher route)
REM  Both via the production entry hangul_ic_process; identical => the Sandeulbaram
REM  FINALSUN automaton is byte-for-byte the old engine, incl. SHKEY park/unpark
REM  and (unhooked) backspace.
tests\golden_finalsun.exe 3-91-noshift old > tests\golden_finalsun_old.txt
tests\golden_finalsun.exe 3-91-noshift new > tests\golden_finalsun_new.txt
fc /n tests\golden_finalsun_old.txt tests\golden_finalsun_new.txt >tests\_diff_fs2.log
if errorlevel 1 (
  echo FINALSUN_FAIL Sandeulbaram finalsun output differs from old-engine output ^(see tests\_diff_fs2.log^):
  type tests\_diff_fs2.log
  popd & exit /b 1
)

echo FINALSUN_PASS

REM --- 3-91-noshift : (3) Stage D-1 ADDITIVE NFC syllable equivalence ---
REM  Same old-vs-new drive, folded to NFC via the PUBLIC hangul_jamos_to_syllables.
REM  Identical user-visible syllables => SHKEY cluster timing agrees old vs new.
tests\golden_finalsun.exe 3-91-noshift old nfc > tests\golden_finalsun_old.txt
tests\golden_finalsun.exe 3-91-noshift new nfc > tests\golden_finalsun_new.txt
fc /n tests\golden_finalsun_old.txt tests\golden_finalsun_new.txt >tests\_diff_fs2.log
if errorlevel 1 (
  echo FINALSUN_NFC_FAIL NFC syllables differ old vs new ^(see tests\_diff_fs2.log^):
  type tests\_diff_fs2.log
  popd & exit /b 1
)

echo FINALSUN_NFC_PASS
popd & exit /b 0
