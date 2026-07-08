@echo off
REM [INTEG] body-level integration gate (3REPO openDecision #5, DECISIONS 49):
REM   needs body-repo fixtures outside the engine tree. In a standalone engine-repo
REM   clone the fixture is absent -> graceful SKIP (exit 77; run_all_gates shows [SKIP]).
if not exist "%~dp0..\..\..\cleanroom" ( echo [SKIP] build_cleanroom_selftests: cleanroom/ absent - body-level INTEG gate & exit /b 77 )
REM build_cleanroom_selftests.bat -- 클린룸 jaso 셀프테스트 일괄 빌드+실행 *정직* 게이트.
REM   회귀를 커밋 시점에 잡는다(과거 로컬 _runtests.cmd 의 추적본; 하드코딩 경로 없음=find_vcvars).
REM   exit = 빌드 OR 런타임(셀프테스트 assert) 실패 수 — 각 .exe 는 return failures?1:0 →
REM   :build 가 errorlevel 로 런타임 실패도 집계(은폐 방지). run_all_gates 가 호출(>nul, 코드만).
REM   ★이 게이트가 있었으면 ahn_chord(f5de938b)·chord_family 또(c7584dc2) 회귀를 즉시 잡았다.
REM ★로스터 단일 정본 = selftest_roster.h (같은 폴더, X-macro: stem·kbid귀속·설명) — 이 .bat 의
REM   call :build 목록과 catalog `check` 가 양방향 대조(불일치=FAIL, build_drift_check→run_all_gates 편입).
REM   새 selftest 추가 = 아래 :build 행 + roster 행 *동시*(한쪽 누락 → check/census FAIL). srcs·실행순서
REM   실행정본은 여전히 이 .bat(행마다 srcs 상이 → cmd 가 헤더를 직소비하지 않음 = 게이트 회귀 0).
setlocal
call "%~dp0find_vcvars.cmd" 64 >nul 2>&1
pushd "%~dp0..\..\..\cleanroom\impl\clean\canonical"
if not exist _build mkdir _build
set CORE=engine-jaso-core.c
set CFLAGS=/nologo /std:c11 /W4 /utf-8 /MD /I..\..\..\include
set FAIL=0

call :build sel_galma   selftest.c %CORE% oracle_galmadeuli.c
call :build sel_shinp2  selftest_shinp2.c %CORE% oracle_shin_p2.c
call :build sel_2012    selftest_jaso_2012.c %CORE% jaso_strat.c jaso_layout_shin2012.c
call :build sel_390     selftest_jaso_390.c %CORE% jaso_strat.c jaso_layout_sebeol390.c
call :build sel_chord   selftest_jaso_chord.c %CORE% jaso_strat.c jaso_chord.c jaso_layout_sebeol_final.c
call :build sel_dchord  selftest_jaso_dubeol_chord.c %CORE% jaso_strat.c jaso_chord.c jaso_layout_dubeol_std.c
call :build sel_filljong selftest_jaso_filljong.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_dubeol  selftest_jaso_dubeol.c %CORE% jaso_strat.c oracle_galmadeuli.c jaso_layout_galmadeuli.c
call :build sel_dbstd   selftest_jaso_dubeol_std.c %CORE% jaso_strat.c jaso_layout_dubeol_std.c
call :build sel_romaja  selftest_romaja.c %CORE% jaso_strat.c jaso_layout_dubeol_std.c romaja_translit.c
call :build sel_hanja   selftest_baram_hanja.c baram_hanja.c
call :build sel_hdiff  selftest_hanja_diff.c baram_hanja.c
call :build sel_jamo   selftest_jamo_compat.c jamo_compat.c
call :build sel_integ  selftest_combine_integrity.c jaso_layout_318na.c jaso_layout_3gs.c jaso_layout_3sun2014.c jaso_layout_dubeol_std.c jaso_layout_galmadeuli.c jaso_layout_gongdong.c jaso_layout_p2.c jaso_layout_p2yet.c jaso_layout_sebeol390.c jaso_layout_sebeol_final.c jaso_layout_shin2012.c jaso_layout_shin2015.c jaso_layout_shinm.c jaso_layout_shinp.c jaso_layout_shinp_yet.c jaso_layout_yetdemo.c
REM P0-a(2/2): 결합표 *엔진구동* 4경로 게이트(정순/역순/거듭/직접키) + 독립 보편-조합 오라클(result 오타 차단). yet=P0-c 이관.
call :build sel_cmbeng selftest_combine_engine.c %CORE% jaso_strat.c jaso_layout_318na.c jaso_layout_3gs.c jaso_layout_3sun2014.c jaso_layout_dubeol_std.c jaso_layout_galmadeuli.c jaso_layout_gongdong.c jaso_layout_p2.c jaso_layout_p2yet.c jaso_layout_sebeol390.c jaso_layout_sebeol_final.c jaso_layout_shin2012.c jaso_layout_shin2015.c jaso_layout_shinm.c jaso_layout_shinp.c jaso_layout_shinp_yet.c jaso_layout_yetdemo.c
REM P0-b: backspace 대칭(역함수) 게이트 — 매 키 후 슬롯(+nucleus_unit) S0..Sm 기록 → backspace 가 Sm-1..S0 정확복원+종단 false. 가상중성 복원 핵심. yet 포함 전 자판.
call :build sel_bsp    selftest_backspace_symmetry.c %CORE% jaso_strat.c jaso_layout_318na.c jaso_layout_3gs.c jaso_layout_3sun2014.c jaso_layout_dubeol_std.c jaso_layout_galmadeuli.c jaso_layout_gongdong.c jaso_layout_p2.c jaso_layout_p2yet.c jaso_layout_sebeol390.c jaso_layout_sebeol_final.c jaso_layout_shin2012.c jaso_layout_shin2015.c jaso_layout_shinm.c jaso_layout_shinp.c jaso_layout_shinp_yet.c jaso_layout_yetdemo.c
REM P0-c: 동결 오라클 vs jaso_strat 전수 교차차분(길이1-3 전수+결정fuzz) — 전략 구현 패리티 상시잠금. P2/galmadeuli(동결 오라클 보유), 그 외 변종=후속.
call :build sel_odiff  selftest_oracle_diff.c %CORE% jaso_strat.c oracle_shin_p2.c oracle_galmadeuli.c jaso_layout_p2.c jaso_layout_galmadeuli.c
call :build sel_3sun14  selftest_jaso_3sun2014.c %CORE% jaso_strat.c jaso_layout_3sun2014.c
call :build sel_314xml  selftest_jaso_314_xml.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_314ban  selftest_jaso_314banja_xml.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_stdxml  selftest_jaso_stdxml.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_ahnchd  selftest_jaso_ahn_chord.c %CORE% jaso_strat.c jaso_chord.c jaso_xml_loader.c jamo_compat.c
call :build sel_shinhs  selftest_jaso_shinhs_xml.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_s2015   selftest_jaso_s2015_xml.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_s2015p  selftest_jaso_s2015p_xml.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_s2015m  selftest_jaso_s2015m_xml.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_chamd   selftest_jaso_chamshind_xml.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_fourz   selftest_jaso_fourz.c %CORE% vm_strat.c vm_layout_4z.c
call :build sel_vmxml   selftest_jaso_vmxml.c vm_xml.c vm_layout_4z.c
call :build sel_gongd   selftest_jaso_gongdong.c %CORE% jaso_strat.c jaso_layout_gongdong.c
call :build sel_gongdx  selftest_jaso_gongdong_xml.c %CORE% jaso_strat.c jaso_layout_gongdong.c jaso_xml_loader.c jamo_compat.c
REM 슬롯-균일 가상단위(3x 흡수 §54): C 직접 3x 레이아웃 + jaso_xml dump→load 왕복 이중구동(slot/rule-cho/술어/state6).
call :build sel_3x      selftest_jaso_3x.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c
call :build sel_none    selftest_jaso_none.c %CORE% jaso_strat.c jaso_layout_sebeol_final.c jaso_layout_p2.c jaso_layout_galmadeuli.c
call :build sel_p2      selftest_jaso_p2.c %CORE% jaso_strat.c oracle_shin_p2.c jaso_layout_p2.c
call :build sel_shinp   selftest_jaso_shinp.c %CORE% jaso_strat.c jaso_layout_shinp.c
call :build sel_shinsh  selftest_jaso_shinshift.c %CORE% jaso_strat.c jaso_layout_shin2015.c jaso_layout_shinm.c jaso_layout_p2.c jaso_layout_galmadeuli.c jaso_layout_sebeol_final.c
call :build sel_vunit   selftest_jaso_virtualunit.c %CORE% jaso_strat.c jaso_layout_p2.c jaso_layout_shinp.c jaso_layout_shin2015.c jaso_layout_shinm.c jaso_layout_galmadeuli.c jaso_layout_sebeol_final.c
call :build sel_yet     selftest_jaso_yet.c %CORE% jaso_strat.c jaso_layout_yetdemo.c jaso_layout_sebeol_final.c jaso_layout_shinp_yet.c jaso_layout_p2yet.c
call :build sel_multi   selftest_jaso_multiemit.c %CORE%
call :build sel_chordf  selftest_jaso_chord_family.c %CORE% jaso_strat.c jaso_chord.c jaso_layout_galmadeuli.c jaso_layout_gongdong.c jaso_layout_shin2015.c
call :build sel_chordlv selftest_jaso_chord_live.c %CORE% jaso_strat.c jaso_chord.c jaso_xml_loader.c jaso_layout_sebeol_final.c jaso_layout_dubeol_std.c jaso_layout_gongdong.c jaso_layout_3gs.c jamo_compat.c
call :build sel_3gs     selftest_jaso_3gs.c %CORE% jaso_strat.c jaso_chord.c jaso_layout_3gs.c
call :build sel_318na   selftest_jaso_318na.c %CORE% jaso_strat.c jaso_layout_318na.c
call :build sel_rt      selftest_jaso_roundtrip.c %CORE% jaso_strat.c jaso_xml_loader.c jaso_layout_p2.c jaso_layout_shinp.c jaso_layout_shin2012.c jaso_layout_shin2015.c jaso_layout_shinm.c jaso_layout_shinp_yet.c jaso_layout_p2yet.c jaso_layout_318na.c jamo_compat.c
REM F-3 R4 de-fork: 3gs(chord+SHKEY)·2noshift/2n9256(galmadeuli)·3sun-2014(jaso-sebeol) 외부 V2 XML 이관 충실성 게이트(C 정본 ≡ XML 왕복).
call :build sel_r4xml   selftest_jaso_r4xml.c %CORE% jaso_strat.c jaso_chord.c jaso_xml_loader.c jaso_layout_3gs.c jaso_layout_galmadeuli.c jaso_layout_3sun2014.c jaso_layout_p2.c jaso_layout_shinp.c jaso_layout_shin2012.c jaso_layout_shin2015.c jaso_layout_shinm.c jaso_layout_shinp_yet.c jaso_layout_p2yet.c jamo_compat.c
REM F-3 R4 option 4: 3-89 and 3sun-1990 legacy-oracle == V2 XML hash parity.
call :build sel_r4par  selftest_jaso_r4_legacy_parity.c %CORE% jaso_strat.c jaso_xml_loader.c jamo_compat.c

echo ============================================================
if %FAIL%==0 (echo CLEANROOM-SELFTESTS-OK) else (echo CLEANROOM-FAILURES=%FAIL% ^(build or runtime^))
popd
endlocal & exit /b %FAIL%

:build
set NAME=%1
shift
set SRCS=
:collect
if "%1"=="" goto compile
set SRCS=%SRCS% %1
shift
goto collect
:compile
cl %CFLAGS% %SRCS% /Fe:_build\%NAME%.exe /Fo:_build\ >_build\%NAME%.buildlog 2>&1
if errorlevel 1 (
  echo [BUILD-FAIL] %NAME%
  set /a FAIL+=1
  exit /b 0
)
_build\%NAME%.exe >_build\%NAME%.runlog 2>&1
if errorlevel 1 (
  echo [TEST-FAIL] %NAME%
  findstr /C:"TOTAL-SUMMARY" _build\%NAME%.runlog
  set /a FAIL+=1
  exit /b 0
)
exit /b 0
