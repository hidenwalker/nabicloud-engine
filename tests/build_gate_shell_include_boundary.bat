@echo off
REM build_gate_shell_include_boundary.bat -- shell-to-engine include boundary gate (2026-07-02 arch review).
REM   Production shell TUs (windows\tsf top-level; tests\ excluded by non-recursive match) must consume
REM   the Sandeulbaram engine ONLY through the public surface (cleanroom\include: sandeulbaram_public.h /
REM   IJasoBackend.h / HangulOutput.h / baram_hanja.h / jamo_compat.h) plus the embedded builtin XML
REM   *data* headers (nabicloud_builtin_*_xml.h). A direct #include of any engine-internal header below
REM   is a layering violation -> FAIL. Baseline at gate introduction = 0 violations (measured).
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs windows\tsf outside the engine tree -> standalone engine repo clone SKIPs (exit 77).
if not exist "%~dp0..\..\..\windows\tsf" ( echo [SKIP] build_gate_shell_include_boundary: windows/tsf/ absent - body-level INTEG gate & exit /b 77 )
setlocal enabledelayedexpansion
set "TSF=%~dp0..\..\..\windows\tsf"
set "SETT=%~dp0..\..\..\windows\settings"
set /a VIOL=0
REM   ist2xml.h added 2026-07-03 (M4 shell-embed): shell TUs drive the ist2xml convert core via an
REM   extern "C" declaration only (i2x_convert_alloc) -- a direct #include of ist2xml.h is a layering
REM   violation (it transitively pulls jaso_layout.h/jaso_xml_loader.h engine-internal headers).
REM   windows\settings added 2026-07-03 (adversarial review): SettingsWebView.cpp is a shell TU too
REM   (compiled into NabiCloud.dll) -- same boundary. tests\ excluded by non-recursive match.
for %%H in (engine-jaso-core.h jaso_strat.h jaso_layout.h jaso_chord.h jaso_xml_loader.h jaso_xml_editor.h vm_strat.h vm_xml.h vm_editor.h v2backend.h shin_p2_strat.h galmadeuli_strat.h fourz_strat.h ist2xml.h) do (
  set "HIT="
  findstr /R /C:"#include.*%%H" "%TSF%\*.cpp" "%TSF%\*.h" >nul 2>&1
  if not errorlevel 1 set "HIT=1"
  findstr /R /C:"#include.*%%H" "%SETT%\*.cpp" >nul 2>&1
  if not errorlevel 1 set "HIT=1"
  if defined HIT (
    echo   [VIOLATION] engine-internal header included by shell: %%H
    findstr /R /N /C:"#include.*%%H" "%TSF%\*.cpp" "%TSF%\*.h" "%SETT%\*.cpp" 2>nul
    set /a VIOL+=1
  )
)
if !VIOL! GTR 0 (
  echo SHELL-INCLUDE-BOUNDARY-FAIL ^(!VIOL! header^(s^)^)
  exit /b 1
)
echo SHELL-INCLUDE-BOUNDARY-OK
exit /b 0
