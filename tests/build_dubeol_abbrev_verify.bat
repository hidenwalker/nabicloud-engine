@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\windows\tsf\tests" ( echo [SKIP] build_dubeol_abbrev_verify: windows/tsf/tests/ absent - body-level INTEG gate & exit /b 77 )
REM build_dubeol_abbrev_verify.bat -- master-gate wrapper for the Dubeol (dugyeobe/dujul-e)
REM   abbreviation trigger engine (DubeolAbbrev) selftest. Real build+run lives in
REM   windows\tsf\tests\ (TSF-shell DubeolAbbrev.cpp + AbbrevDict.cpp); this thin wrapper lets
REM   run_all_gates discover it as a build_*.bat in HERE and aggregate its exit code (PASS->0/FAIL->1).
call "%~dp0..\..\..\windows\tsf\tests\build_dubeol_abbrev_selftest.bat"
exit /b %errorlevel%
