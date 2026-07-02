@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\cleanroom" ( echo [SKIP] build_v2backend_importzero: cleanroom/ absent - body-level INTEG gate & exit /b 77 )
REM build_v2backend_importzero.bat -- import-0 MECHANICAL gate (C3 / module split Step 2).
REM   Proves the public sandeulbaram static library can be consumed standalone
REM   with ZERO libhangul: no libhangul .obj/.lib on the consumer link line.
REM   Two mechanical proofs:
REM     [A] LINK PROOF: if sandeulbaram.lib carried any hangul_* / hic_* dependency,
REM         the libhangul-free consumer link would fail (LNK2019 unresolved external).
REM         Link success + 38/38 = import-0.
REM     [B] dumpbin ASSERTS: the final test exe imports no libhangul DLL, and
REM         sandeulbaram.lib has no undefined hangul_ / hic_ / hangul_ic symbol.
REM   exit /b 0 only if BOTH pass. run_all_gates.bat (errorlevel only) calls this.
setlocal
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
if errorlevel 1 (
  echo IMPORTZERO_FAIL [vcvars]
  exit /b 1
)

set "ROOT=%~dp0..\..\.."
set "IMPL=%ROOT%\cleanroom\impl"
set "PROMO=%IMPL%\promote-shinp2"
set "CAN=%IMPL%\clean\canonical"
set "PUB=%ROOT%\cleanroom\include"
set "SAND_LIB=%IMPL%\x64\Release\sandeulbaram.lib"
set "LOG=%PROMO%\_importzero.log"

msbuild "%IMPL%\sandeulbaram.vcxproj" /m:1 /nr:false /p:Configuration=Release /p:Platform=x64 /p:TrackFileAccess=false /v:minimal 1>"%LOG%" 2>&1
if errorlevel 1 (
  echo IMPORTZERO_FAIL [sandeulbaram.lib build]
  type "%LOG%"
  exit /b 1
)
if not exist "%SAND_LIB%" (
  echo IMPORTZERO_FAIL [sandeulbaram.lib missing]
  type "%LOG%"
  exit /b 1
)

REM [A] libhangul-FREE link (NO libhangul .obj/.lib in inputs) + run self-test.
pushd "%PROMO%"
cl /nologo /W4 /utf-8 /std:c++17 /MT /I . /I "%PUB%" /I "%CAN%" v2backend_test.cpp "%SAND_LIB%" /Fe:_v2importzero.exe 1>>_importzero.log 2>&1
if errorlevel 1 (
  echo IMPORTZERO_FAIL [consumer link or compile -- libhangul symbol leaked?]
  type _importzero.log
  popd
  exit /b 1
)
".\_v2importzero.exe" 1>>_importzero.log 2>&1
if errorlevel 1 (
  echo IMPORTZERO_FAIL [self-test 38/38]
  type _importzero.log
  popd
  exit /b 1
)

REM [B] dumpbin asserts: no imported libhangul DLL, no unresolved libhangul-style symbols in the static lib.
dumpbin /imports _v2importzero.exe 2>nul | findstr /I /C:"libhangul" >nul 2>&1
if not errorlevel 1 (
  echo IMPORTZERO_FAIL [dumpbin: libhangul import present in consumer exe]
  popd
  exit /b 1
)
dumpbin /symbols "%SAND_LIB%" 2>nul | findstr /C:UNDEF | findstr /R /C:"hangul_" /C:"hic_" /C:"hangul_ic" >nul 2>&1
if not errorlevel 1 (
  echo IMPORTZERO_FAIL [dumpbin: libhangul UNDEF symbol present in sandeulbaram.lib]
  popd
  exit /b 1
)

echo V2BACKEND_IMPORTZERO_OK
popd
exit /b 0
