@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\cleanroom" ( echo [SKIP] build_vm_editor_verify: cleanroom/ absent - body-level INTEG gate & exit /b 77 )
REM ==========================================================================
REM  VM (register-machine / 4z) keyboard-editor bridge cross-verify gate (thin wrapper).
REM  Real build+run lives in cleanroom\impl\clean\canonical\build_vm_editor_verify.cmd
REM  (vm_editor + v2backend VM ctor/getter are TSF/cleanroom-module sources). Checks
REM  roundtrip-idempotence + serialize/validate/preview + 4z differential. Exit code passed through.
REM ==========================================================================
call "%~dp0..\..\..\cleanroom\impl\clean\canonical\build_vm_editor_verify.cmd"
exit /b %errorlevel%
