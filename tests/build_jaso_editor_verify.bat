@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\cleanroom" ( echo [SKIP] build_jaso_editor_verify: cleanroom/ absent - body-level INTEG gate & exit /b 77 )
REM ==========================================================================
REM  V2 keyboard-editor bridge cross-verify gate (thin wrapper).
REM  Real build+run lives in cleanroom\impl\clean\canonical\build_jaso_editor_verify.cmd
REM  (editor C wiring jaso_xml_editor + v2backend are TSF/cleanroom-module sources, so the
REM  gate builds them there). Checks serialize/validate/preview + A-2 single-source table
REM  (getter==ctor). Exit code passed through (0 = PASS).
REM ==========================================================================
call "%~dp0..\..\..\cleanroom\impl\clean\canonical\build_jaso_editor_verify.cmd"
exit /b %errorlevel%
