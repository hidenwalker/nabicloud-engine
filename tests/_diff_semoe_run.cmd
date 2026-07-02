@echo off
REM Semo-e cross-engine differential: NabiCloud port vs 3beol reference.
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
REM 3beol(yous/libhangul) 참조 트리. 머신마다 다르므로 외부화: 인자 1 또는 env REF 로 전달.
REM   사용:  set REF=<...\yous-libhangul\hangul>  &  _diff_semoe_run.cmd   (또는 _diff_semoe_run.cmd <path>)
if not defined REF set "REF=%~1"
if "%REF%"=="" (
  echo [SKIP] _diff_semoe: REF(3beol yous-libhangul\hangul 디렉터리) 미설정. env REF 또는 인자로 경로 지정.
  exit /b 0
)
if not exist "%REF%" (
  echo [SKIP] _diff_semoe: REF 경로 없음: %REF%
  exit /b 0
)

REM ---- NabiCloud (ours) build ----
pushd "%ROOT%"
> tests\_srcs_semoe.rsp echo tests\diff_semoe.c
for %%f in (hangul\*.c) do if /i not "%%~nxf"=="hanja.c" >> tests\_srcs_semoe.rsp echo %%f
if exist nabicloud for /r nabicloud %%f in (*.c) do >> tests\_srcs_semoe.rsp echo "%%f"
set "INC=/I hangul /I nabicloud /I nabicloud\engine /I nabicloud\layouts /I nabicloud\compatibility /I nabicloud\win32"
cl /nologo /W3 /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE !INC! /Fe:tests\diff_semoe_nc.exe /Fo:tests\ @tests\_srcs_semoe.rsp 1>tests\_build_semoe_nc.log 2>&1
if errorlevel 1 ( echo SEMOE_FAIL nc build error & type tests\_build_semoe_nc.log & popd & exit /b 1 )
tests\diff_semoe_nc.exe > tests\_semoe_nc.txt
popd

REM ---- 3beol reference build (copy harness into ref tree) ----
copy /y "%ROOT%\tests\diff_semoe.c" "%REF%\diff_semoe.c" >nul
pushd "%REF%"
cl /nologo /W3 /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_DEPRECATE /DINIT_IDS_LENGTH=10 /DLIBHANGUL_DATA_DIR=\".\" /FI_shim.h /I . /Fe:diff_semoe_ref.exe /Fo:_refobj\ diff_semoe.c hangulctype.c hangulinputcontext.c hangulkeyboard.c 1>"%ROOT%\tests\_build_semoe_ref.log" 2>&1
if errorlevel 1 ( echo SEMOE_FAIL ref build error & type "%ROOT%\tests\_build_semoe_ref.log" & popd & exit /b 1 )
.\diff_semoe_ref.exe > "%ROOT%\tests\_semoe_ref.txt"
popd

REM ---- diff ----
fc /n "%ROOT%\tests\_semoe_ref.txt" "%ROOT%\tests\_semoe_nc.txt" >"%ROOT%\tests\_diff_semoe.log"
if errorlevel 1 ( echo SEMOE_FAIL behavior diff: & type "%ROOT%\tests\_diff_semoe.log" & exit /b 1 )
echo SEMOE_PASS mismatch=0
exit /b 0
