@echo off
REM find_vcvars.cmd -- MSVC 빌드 환경을 *하드코딩 경로 없이* 적용한다(로컬 경로 비커밋 원칙).
REM   사용: call "<path>\find_vcvars.cmd" [64^|32]   (인자1 = 아키텍처, 기본 64)
REM   ★반드시 call 로 호출 — setlocal 없음이라 vcvars 가 *호출자 cmd 세션*에 환경을 전파한다.
REM   탐색 우선순위:
REM     1) VCVARS_DIR 환경변수(있으면 "%VCVARS_DIR%\vcvarsNN.bat") — 비표준 설치 override
REM     2) vswhere -latest(표준 설치 위치) 로 VS 설치경로 발견 → VC\Auxiliary\Build\vcvarsNN.bat
REM   vcvars 미발견 시 비치명적(호출자 cl 단계가 실패로 잡는다).
if "%~1"=="" (set "_VA=64") else (set "_VA=%~1")
if defined VCVARS_DIR (
  call "%VCVARS_DIR%\vcvars%_VA%.bat"
  set "_VA="
  goto :eof
)
set "_VW=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%_VW%" set "_VW=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%v in (`"%_VW%" -latest -products * -property installationPath 2^>nul`) do call "%%v\VC\Auxiliary\Build\vcvars%_VA%.bat"
set "_VA="
set "_VW="
