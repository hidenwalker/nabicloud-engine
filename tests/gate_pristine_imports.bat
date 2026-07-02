@echo off
REM R0 Pristine Import Gate -- audit NabiCloud.dll's libhangul.dll imports vs de-fork allow/forbid sets.
REM   Audit mode (default): report forbidden non-pristine imports, exit 0 (R0~R3 progress tracking).
REM   Enforce mode (pass ENFORCE arg; used at R4): exit 1 if any forbidden import remains.
REM   Forbidden = the 18 non-pristine ABI being cut R1~R3: load_dir / process_with_capslock /
REM   has_convertible_syllable / table_load_wide / map_to_char / get_type / combine / get_flag /
REM   nabicloud_* / editor_*. Goal: B' shell imports only standard hangul_*/hanja_* + new_from_file/
REM   register/unregister/delete -> pristine/variant libhangul.dll drop-in (S6b).
setlocal
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
set "DLL=%~dp0..\..\..\windows\tsf\x64\Release\NabiCloud.dll"
if not exist "%DLL%" set "DLL=%~dp0..\..\..\windows\tsf\Win32\Release\NabiCloud.dll"
if not exist "%DLL%" (
  echo GATE_PRISTINE_IMPORTS_FAIL: NabiCloud.dll not found ^(build NabiCloud.sln first^)
  exit /b 1
)
dumpbin /imports "%DLL%" > "%~dp0_pristine_imports.txt" 2>&1
set "ENF="
if /I "%~1"=="ENFORCE" set "ENF=-Enforce"
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0gate_pristine_imports.ps1" "%~dp0_pristine_imports.txt" %ENF%
exit /b %errorlevel%
