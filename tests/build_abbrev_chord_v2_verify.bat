@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\windows\tsf\tests" ( echo [SKIP] build_abbrev_chord_v2_verify: windows/tsf/tests/ absent - body-level INTEG gate & exit /b 77 )
REM build_abbrev_chord_v2_verify.bat -- master-gate wrapper for the Semoe UnitMix abbrev
REM   chord-fold V2-path equivalence selftest (Y2/G7, d770408f). Real build+run lives in
REM   windows\tsf\tests\ (needs AbbrevDict.cpp shell canon); this thin wrapper lets run_all_gates
REM   discover it as a build_*.bat in HERE and aggregate its exit code (PASS->0 / FAIL->1).
REM   Asserts: .dic full sweep refold + UnitMix tsv byte-equiv + UnitMixCount()==tsv rows (A6).
call "%~dp0..\..\..\windows\tsf\tests\build_abbrev_chord_v2_selftest.bat"
exit /b %errorlevel%
