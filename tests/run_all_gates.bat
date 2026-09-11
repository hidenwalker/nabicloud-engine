@echo off
REM One all-suite request plus its existing WU9 action; execution owner is testsys.
setlocal
set "ROOT=%~dp0..\..\.."
pushd "%ROOT%\raindrop-runtime\sdk\test\python"
python -m raindrop_testsys run --suite all --arch x64 --tolerate-manual-evidence --with-wu9 --root "%ROOT%"
set "RESULT=%ERRORLEVEL%"
popd
if "%RESULT%"=="0" (echo ALL_GATES_PASS) else (echo GATES_FAIL: %RESULT%)
exit /b %RESULT%
