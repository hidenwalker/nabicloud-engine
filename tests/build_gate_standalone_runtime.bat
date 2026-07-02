@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\cleanroom" ( echo [SKIP] build_gate_standalone_runtime: cleanroom/ absent - body-level INTEG gate & exit /b 77 )
if not exist "%~dp0..\..\..\windows\tsf" ( echo [SKIP] build_gate_standalone_runtime: windows/tsf/ absent - body-level INTEG gate & exit /b 77 )
REM build_gate_standalone_runtime.bat -- Phase 3.5 runtime proof.
REM   Builds x64 NabiCloud.dll, copies NabiCloud.dll plus the clean base Hanja
REM   dictionary to a temporary directory with no libhangul.dll sibling, then proves
REM   the DLL can be loaded and HanjaDict can run from that standalone payload.
REM   If libhangul.dll is still a hard loader dependency, LoadLibraryExW fails.
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
if errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [vcvars]
  exit /b 1
)

set "ROOT=%~dp0..\..\.."
set "SLN=%ROOT%\windows\NabiCloud.sln"
set "DLL=%ROOT%\windows\tsf\x64\Release\NabiCloud.dll"
set "DICT_SRC=%ROOT%\shared\data\dictionary"
set "LOG=%~dp0_standalone_runtime.log"
set "PROBE=%~dp0_standalone_load_probe.exe"
set "HANJA_PROBE=%~dp0standalone_hanja_probe.exe"
set "STAND_DIR=%TEMP%\NabiCloudStandaloneGate_%RANDOM%%RANDOM%"

msbuild "%SLN%" /m:1 /nr:false /p:Configuration=Release /p:Platform=x64 /p:TrackFileAccess=false /v:minimal > "%LOG%" 2>&1
if errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [NabiCloud x64 build]
  type "%LOG%"
  exit /b 1
)
if not exist "%DLL%" (
  echo STANDALONE_RUNTIME_FAIL [NabiCloud.dll missing]
  type "%LOG%"
  exit /b 1
)

mkdir "%STAND_DIR%" >> "%LOG%" 2>&1
if errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [temp dir]
  type "%LOG%"
  exit /b 1
)

cl /nologo /W4 /EHsc /utf-8 /std:c++17 /MT /Fo"%STAND_DIR%\\" "%~dp0standalone_load_probe.cpp" /Fe:"%PROBE%" >> "%LOG%" 2>&1
if errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [probe build]
  type "%LOG%"
  call :cleanup
  exit /b 1
)

mkdir "%STAND_DIR%\dictionary" >> "%LOG%" 2>&1
if errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [dictionary temp dir]
  type "%LOG%"
  call :cleanup
  exit /b 1
)
copy /Y "%DLL%" "%STAND_DIR%\NabiCloud.dll" >> "%LOG%" 2>&1
if errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [copy NabiCloud.dll]
  type "%LOG%"
  call :cleanup
  exit /b 1
)
copy /Y "%DICT_SRC%\10-hanja-wiki.txt" "%STAND_DIR%\dictionary\" >> "%LOG%" 2>&1
if errorlevel 1 goto copydictfail
copy /Y "%DICT_SRC%\40-mssymbol-msime.txt" "%STAND_DIR%\dictionary\" >> "%LOG%" 2>&1
if errorlevel 1 goto copydictfail
copy /Y "%DICT_SRC%\hanja_meta.tsv" "%STAND_DIR%\dictionary\" >> "%LOG%" 2>&1
if errorlevel 1 goto copydictfail
copy /Y "%DICT_SRC%\90-user.txt.template" "%STAND_DIR%\dictionary\" >> "%LOG%" 2>&1
if errorlevel 1 goto copydictfail

cl /nologo /W4 /EHsc /utf-8 /std:c++17 /MT /wd4996 ^
  /I"%ROOT%\windows\tsf" /I"%ROOT%\cleanroom\include" /Fo"%STAND_DIR%\\" ^
  "%~dp0standalone_hanja_probe.cpp" ^
  "%ROOT%\windows\tsf\HanjaDict.cpp" ^
  "%ROOT%\windows\tsf\HanjaLearn.cpp" ^
  "%ROOT%\cleanroom\impl\clean\canonical\baram_hanja.c" ^
  /Fe:"%HANJA_PROBE%" >> "%LOG%" 2>&1
if errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [hanja probe build]
  type "%LOG%"
  call :cleanup
  exit /b 1
)

dumpbin /imports "%HANJA_PROBE%" | findstr /I "libhangul" >nul 2>&1
if not errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [hanja probe imports libhangul]
  type "%LOG%"
  call :cleanup
  exit /b 1
)
if exist "%STAND_DIR%\libhangul.dll" (
  echo STANDALONE_RUNTIME_FAIL [unexpected libhangul.dll in temp dir]
  call :cleanup
  exit /b 1
)

"%PROBE%" "%STAND_DIR%\NabiCloud.dll" >> "%LOG%" 2>&1
if errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [LoadLibraryExW without libhangul]
  type "%LOG%"
  call :cleanup
  exit /b 1
)

"%HANJA_PROBE%" "%STAND_DIR%\dictionary" >> "%LOG%" 2>&1
if errorlevel 1 (
  echo STANDALONE_RUNTIME_FAIL [clean HanjaDict standalone]
  type "%LOG%"
  call :cleanup
  exit /b 1
)

call :cleanup
echo STANDALONE_RUNTIME_OK
exit /b 0

:copydictfail
echo STANDALONE_RUNTIME_FAIL [copy clean base dictionary]
type "%LOG%"
call :cleanup
exit /b 1

:cleanup
if defined STAND_DIR (
  if exist "%STAND_DIR%" rmdir /s /q "%STAND_DIR%" >nul 2>&1
)
exit /b 0
