@echo off
REM build_one_emit.cmd -- build+run ONE instrumented selftest with -DEMIT_JSON (emission collect, diagnostic).
REM   usage: build_one_emit.cmd <NAME> <src...>   (src = the source list from build_cleanroom_selftests.bat)
REM   stdout = selftest run output (PASS/FAIL + {"emit":1,...} JSON lines). Caller (runtime_checklist_catalog.py
REM   collect) parses only emit lines. NOT a gate -- run_all_gates does not call this (built separately, no EMIT_JSON).
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
pushd "%~dp0..\..\..\cleanroom\impl\clean\canonical"
if not exist _build mkdir _build
set "NAME=%~1"
shift
set "SRCS="
:collect
if "%~1"=="" goto compile
set "SRCS=%SRCS% %~1"
shift
goto collect
:compile
cl /nologo /std:c11 /W4 /utf-8 /MD /DEMIT_JSON /I..\..\..\include %SRCS% /Fe:_build\emit_%NAME%.exe /Fo:_build\ >_build\emit_%NAME%.buildlog 2>&1
if errorlevel 1 goto fail
_build\emit_%NAME%.exe
popd
exit /b 0
:fail
echo EMIT-BUILD-FAIL %NAME% 1>&2
type _build\emit_%NAME%.buildlog 1>&2
popd
exit /b 1
