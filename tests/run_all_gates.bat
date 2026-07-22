@echo off
REM run_all_gates.bat -- T6 (59th session): thin wrapper delegating to the single raindrop_testsys
REM   driver (superproject-wide "all" suite: engine+cleanroom+tsf+raindrop+foundation, 148 gates).
REM   Supersedes the old 13-loop + separate run_spellcheck_gates.bat/build_cleanroom_selftests.bat
REM   calls, which are now redundant subsets of --suite all (avoid double-execution -- those two files
REM   still work standalone for anyone/anything invoking them directly, they're just no longer called
REM   FROM here). Standalone public nabicloud-engine clone still works: manifest.load() tolerates a
REM   shard file being absent (cleanroom/windows/raindrop-runtime submodules not checked out there) and
REM   just runs whatever shards ARE present (CORE engine.golden_all etc.), so the old per-gate exit-77
REM   SKIP guards are no longer needed here -- the driver itself degrades gracefully.
REM   WU9 (windows/settings JS editor-core tests) has no gate-manifest shard yet (script_test kind is
REM   Python-only; node needs its own kind or a legacy_bat wrapper -- tracked as a T6 follow-up), so it
REM   stays as an inline check here exactly as before.
REM   --tolerate-manual-evidence: some gates (artifact_check, BLOCKER 2) are permanently
REM   MANUAL_EVIDENCE_REQUIRED by design (rd.secure_zero_asm, tsf.lex_mmap_ws_probe) -- without this the
REM   driver's fail-closed exit 3 would block this gate (and any CI release gate calling it) forever.
setlocal enabledelayedexpansion
set "HERE=%~dp0"
set "ROOT=%HERE%..\..\.."
set FAILS=0

echo == raindrop_testsys: suite=all (engine+cleanroom+tsf+raindrop+foundation) ==
pushd "%ROOT%\raindrop-runtime\sdk\test\python"
python -m raindrop_testsys run --suite all --arch x64 --tolerate-manual-evidence --root "%ROOT%"
if errorlevel 1 (
  echo   [FAIL] raindrop_testsys suite=all  ^(re-run: python -m raindrop_testsys run --suite all --arch x64 --root "%ROOT%" for detail^)
  set /a FAILS=1
) else (
  echo   [PASS] raindrop_testsys suite=all
)
popd

echo == WU9 editor-core JS (node) ==
set "WU9DIR=%HERE%..\..\..\windows\settings\tests"
if not exist "%WU9DIR%\sync-core.js" (
  echo   [SKIP] WU9 editor-core JS  ^(INTEG fixture absent - run from superproject^)
) else (
  node --version >nul 2>&1
  if errorlevel 1 (
    echo   [SKIP] WU9 node not installed
  ) else (
    call :run_wu9
  )
)

echo ----------------------------------------------------------------------
if %FAILS%==0 ( echo ALL_GATES_PASS & exit /b 0 ) else ( echo GATES_FAIL: %FAILS% & exit /b %FAILS% )
goto :eof

:run_wu9
REM node --test misreads a directory argument as a module -- cd into it and let cwd globbing run.
pushd "%WU9DIR%"
node sync-core.js --check >nul 2>&1
if errorlevel 1 (
  echo   [FAIL] WU9 sync ^(windows\settings\tests\sync-core.js^)
  set /a FAILS+=1
  popd
  exit /b 0
)
node --test >nul 2>&1
if errorlevel 1 (
  echo   [FAIL] WU9 editor-core JS tests
  set /a FAILS+=1
  popd
  exit /b 0
)
echo   [PASS] WU9 editor-core JS tests
popd
exit /b 0
