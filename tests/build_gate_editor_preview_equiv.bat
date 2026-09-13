@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\data\keyboards" ( echo [SKIP] build_gate_editor_preview_equiv: shared/data/keyboards/ absent - body-level INTEG gate & exit /b 77 )
REM ==========================================================================
REM  F-3 R4 gate: build+run gate_editor_preview_equiv -- prove the legacy
REM  standard-XML editor preview fallback is gone. The shipped 3-89 / 3sun-1990
REM  files must be V2 XML, and real preview composition is then covered by
REM  build_jaso_editor_verify. No engine XML-loader source is compiled here.
REM ==========================================================================
setlocal enabledelayedexpansion
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "ROOT=%~dp0.."
set "OUT=%ROOT%\_build\tests\preview_equiv"
if not exist "%OUT%" mkdir "%OUT%"
if not exist "%OUT%" exit /b 1
pushd "%OUT%" || exit /b 1

cl /nologo /W3 /D_CRT_SECURE_NO_WARNINGS "%ROOT%\tests\gate_editor_preview_equiv.c" /Fe:gate_editor_preview_equiv.exe /Fo:"%OUT%\\" 1>"%OUT%\build.log" 2>&1
if errorlevel 1 (
  echo GATE_EDITOR_PREVIEW_EQUIV_FAIL build error:
  type "%OUT%\build.log"
  popd
  exit /b 1
)

REM kbdir relative to engine ROOT (shared\engine): ..\data\keyboards
gate_editor_preview_equiv.exe "%ROOT%\..\data\keyboards" > "%OUT%\result.txt" 2>&1
set "RC=!errorlevel!"
type "%OUT%\result.txt"
findstr /C:"GATE_EDITOR_PREVIEW_EQUIV_PASS" "%OUT%\result.txt" >nul 2>&1 || set "RC=1"
if "!RC!"=="0" (
  call "%~dp0build_jaso_editor_verify.bat" >nul 2>&1
  if errorlevel 1 (
    echo GATE_EDITOR_PREVIEW_EQUIV_FAIL: build_jaso_editor_verify failed
    set "RC=1"
  ) else (
    echo   [ok] build_jaso_editor_verify covers V2 preview composition
  )
)
popd & exit /b !RC!
