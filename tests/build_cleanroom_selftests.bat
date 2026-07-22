@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\cleanroom" ( echo [SKIP] build_cleanroom_selftests: cleanroom/ absent - body-level INTEG gate & exit /b 77 )
REM build_cleanroom_selftests.bat -- T6 (59th session): thin wrapper delegating to the raindrop_testsys
REM   driver. The 46 sel_* selftests this used to build+run directly (`call :build NAME src...`) are now
REM   the cleanroom/impl/clean/canonical shard's leaf gates (T4, 58th session) -- this just runs that
REM   suite. jaso_catalog.py (T5's catalog/select/census/check/collect/verify/pairs, 59th session) reads
REM   that SAME gates.json shard directly for its source-list data, NOT this .bat's body -- so this file
REM   is no longer load-bearing for anything except its own direct-invocation callers.
setlocal
set "ROOT=%~dp0..\..\.."
pushd "%ROOT%\raindrop-runtime\sdk\test\python"
python -m raindrop_testsys run --suite cleanroom --arch x64 --tolerate-manual-evidence --root "%ROOT%"
set "_RC=%ERRORLEVEL%"
popd
if %_RC%==0 ( echo CLEANROOM-SELFTESTS-OK & exit /b 0 ) else ( echo CLEANROOM-FAILURES ^(see above^) & exit /b 1 )
