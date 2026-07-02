@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\windows\installer\NabiCloud-v2.nsi" ( echo [SKIP] build_drift_check: windows/installer/NabiCloud-v2.nsi absent - body-level INTEG gate & exit /b 77 )
if not exist "%~dp0..\..\data\keyboards" ( echo [SKIP] build_drift_check: shared/data/keyboards/ absent - body-level INTEG gate & exit /b 77 )
REM build_drift_check.bat -- static drift gate (build 0): runtime_checklist_catalog check + census.
REM   check = known-dropped sync + mechanism-field drift (jaso_layout struct classified).
REM   census = canonical selftest*.c intersect build manifest. python absent = skip (exit 0).
python --version >nul 2>&1
if errorlevel 1 exit /b 0
python "%~dp0runtime_checklist_catalog.py" check >nul 2>&1
if errorlevel 1 exit /b 1
python "%~dp0runtime_checklist_catalog.py" census >nul 2>&1
if errorlevel 1 exit /b 1
exit /b 0
