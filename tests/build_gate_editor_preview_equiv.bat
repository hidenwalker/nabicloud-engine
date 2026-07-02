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
pushd "%ROOT%"

cl /nologo /W3 /D_CRT_SECURE_NO_WARNINGS tests\gate_editor_preview_equiv.c /Fe:tests\gate_editor_preview_equiv.exe /Fo:tests\ 1>tests\_build_preview_equiv.log 2>&1
if errorlevel 1 (
  echo GATE_EDITOR_PREVIEW_EQUIV_FAIL build error:
  type tests\_build_preview_equiv.log
  popd ^& exit /b 1
)

REM kbdir relative to engine ROOT (shared\engine): ..\data\keyboards
tests\gate_editor_preview_equiv.exe "..\data\keyboards" > tests\_preview_equiv.txt 2>&1
set "RC=!errorlevel!"
type tests\_preview_equiv.txt
findstr /C:"GATE_EDITOR_PREVIEW_EQUIV_PASS" tests\_preview_equiv.txt >nul 2>&1 || set "RC=1"
if "!RC!"=="0" (
  call tests\build_jaso_editor_verify.bat >nul 2>&1
  if errorlevel 1 (
    echo GATE_EDITOR_PREVIEW_EQUIV_FAIL: build_jaso_editor_verify failed
    set "RC=1"
  ) else (
    echo   [ok] build_jaso_editor_verify covers V2 preview composition
  )
)
popd & exit /b !RC!
