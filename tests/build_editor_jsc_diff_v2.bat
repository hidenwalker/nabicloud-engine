@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs cleanroom/ (V2 engine + editor bridge sources) AND windows/settings
REM   (editor-core.js) AND node. Any absent -> graceful SKIP (exit 77;
REM   run_all_gates convention shows [SKIP], not FAIL).
if not exist "%~dp0..\..\..\cleanroom" ( echo [SKIP] build_editor_jsc_diff_v2: cleanroom/ absent - body-level INTEG gate & exit /b 77 )
if not exist "%~dp0..\..\..\windows\settings\tests\editor-core.js" ( echo [SKIP] build_editor_jsc_diff_v2: editor-core.js absent - body-level INTEG gate & exit /b 77 )
node --version >nul 2>&1
if errorlevel 1 ( echo [SKIP] build_editor_jsc_diff_v2: node not installed & exit /b 77 )
REM ==========================================================================
REM  JS<->C serializer differential gate, V2 edition (2026-07-02).
REM  Successor of RETIRED build_editor_jsc_diff.bat (old engine XML path, F-3 R4;
REM  see tests\RETIRED.md). Restores the "C-serialize -> JS-parse" cross-language
REM  drift gate for the V2 architecture:
REM  (1) Build editor_jsc_dump_v2.cpp against the LIVE V2 editor bridge
REM      (jaso_xml_editor.cpp + v2backend.cpp + jaso engine kernel, the exact
REM      source set of cleanroom build_jaso_editor_verify.cmd) -> dump each
REM      kJasoBuiltin V2 keyboard's canonical serialization + shell meta
REM      injection (SettingsWebView editorLoad replica) to tests\_jsc_v2\<id>.xml
REM      (ephemeral, not committed).
REM  (2) node tests\jsc-diff-v2.js checks the JS core (editor-core.js)
REM      parseKeyboardXml -> serializeKeyboard reproduces those bytes exactly
REM      (JS V2 format == C canonical format, all 14 builtins).
REM  NOT in run_all_gates GATES yet (standalone; enlisting = maintainer call).
REM  ASCII-only harness, mirroring build_jaso_editor_verify.cmd / retired
REM  build_editor_jsc_diff.bat. Engine/builtins/data untouched.
REM ==========================================================================
setlocal
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "HERE=%~dp0"
set "CAN=%HERE%..\..\..\cleanroom\impl\clean\canonical"
set "PROMO=%HERE%..\..\..\cleanroom\impl\promote-shinp2"
set "PUB=%HERE%..\..\..\cleanroom\include"
pushd "%CAN%"
if not exist _jscv2 mkdir _jscv2

REM Same C kernel set as build_jaso_editor_verify.cmd (editor bridge deps).
set CSRC=engine-jaso-core.c jaso_strat.c jaso_chord.c vm_strat.c jaso_xml_loader.c
set CSRC=%CSRC% jaso_layout_p2.c jaso_layout_shinp.c jaso_layout_shin2012.c jaso_layout_shin2015.c
set CSRC=%CSRC% jaso_layout_shinm.c jaso_layout_shinp_yet.c jaso_layout_p2yet.c jaso_layout_3gs.c
set CSRC=%CSRC% jaso_layout_sebeol390.c jaso_layout_sebeol_final.c jaso_layout_galmadeuli.c jaso_layout_dubeol_std.c jaso_layout_3sun2014.c
set CSRC=%CSRC% jamo_compat.c

set CFILES=
for %%F in (%CSRC%) do call set CFILES=%%CFILES%% "%CAN%\%%F"
cl /nologo /W4 /utf-8 /std:c11 /c /I "%CAN%." /I "%PUB%" %CFILES% /Fo:_jscv2\ 1>_jscv2\_build.log 2>&1
if errorlevel 1 ( echo JSC_DIFF_V2_FAIL [C kernel] & type _jscv2\_build.log & popd & exit /b 1 )

cl /nologo /W4 /utf-8 /std:c++17 /I "%CAN%." /I "%PUB%" /I "%PROMO%" ^
   "%PROMO%\v2backend.cpp" "%CAN%\jaso_xml_editor.cpp" "%HERE%editor_jsc_dump_v2.cpp" ^
   _jscv2\*.obj /Fe:_jscv2\_editor_jsc_dump_v2.exe /Fo:_jscv2\ 1>>_jscv2\_build.log 2>&1
if errorlevel 1 ( echo JSC_DIFF_V2_FAIL [C++ bridge] & type _jscv2\_build.log & popd & exit /b 1 )

if not exist "%HERE%_jsc_v2" mkdir "%HERE%_jsc_v2"
del /q "%HERE%_jsc_v2\*.xml" 2>nul
_jscv2\_editor_jsc_dump_v2.exe "%HERE%_jsc_v2"
if errorlevel 1 ( echo JSC_DIFF_V2_FAIL dump error & popd & exit /b 1 )
popd

node "%HERE%jsc-diff-v2.js"
if errorlevel 1 exit /b 1
exit /b 0
