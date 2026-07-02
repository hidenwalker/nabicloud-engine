@echo off
REM ==========================================================================
REM  NabiCloud master gate runner. Runs every engine build_and_verify_*.bat /
REM  build_*_verify.bat and aggregates results. Exits 0 only if ALL gates pass;
REM  otherwise exits with the failure count. Single entry point for "are all
REM  engine gates green?" — golden byte (all/baram) + functional clean-room
REM  (318na/3gs/galmadeuli/finalsun) + chord/XML (chord/dubeol_chord/dubeol_jong/
REM  anmate/dubeol_xml/semoe_xml/shin_xml). Each gate rebuilds (slow but thorough).
REM  On a FAIL, re-run that single .bat directly to see the assertion output.
REM ==========================================================================
setlocal enabledelayedexpansion
set "HERE=%~dp0"
set /a FAILS=0
REM (2026-06-21 통합) build_and_verify_baram / build_and_verify_3gs 은퇴 — 옛 baram
REM 병렬 impl(baram_process/baram_dispatch)을 직접 구동하던 byte-등가 골든. 병렬 impl
REM 을 baram.c(=ops vtable 통합)로 흡수-은퇴하면서 두 골든도 폐기(기능은 all/
REM galmadeuli/finalsun/chord 골든이 커버). golden_baram.c-golden_3gs_dual.c 는 git rm 예정.
REM (2026-06-25 V2 migration) build_and_verify_318na retired.
REM   318na now ships as V2 XML. golden_318na is an old-engine oracle and
REM   cannot read the V2 jaso-map body. Coverage moved to cleanroom
REM   selftest_jaso_318na plus selftest_jaso_roundtrip. The old bat/C files
REM   are kept only as historical old-engine oracle material.
REM 2026-06-26 F-2 libhangul fork chord engine removal: build_and_verify_chord /
REM   build_and_verify_dubeol_chord / build_anmate_chord_verify retired -- the old chord
REM   selftests directly compiled the removed engine-chord.c / engine-chord-dubeol.c dead anchor.
REM   chord is V2-only now; coverage moved to V2: cleanroom chord_live selftest 45/45 +
REM   G8 chord_oracle_parity 15/15 = V2==libhangul NFC. shin-oracle retirement precedent.
REM (2026-06-26) build_cleanroom_selftests = ??? jaso ?????(?? ?? _runtests ???,
REM   find_vcvars ? vcvars ??) ?? *??* ???. ahn_chord(f5de938b)�chord_family ?(c7584dc2)
REM   ? ??? ?? ??? ??(??? ?? ??).
REM (2026-06-27 F-3 R4 de-fork) build_and_verify_galmadeuli / build_and_verify_finalsun 은퇴 —
REM   포크 빌트인(2noshift/2n9256=galmadeuli, 3gs/3-91-noshift=finalsun)이 순정 libhangul 0.2.0
REM   재배이스로 제거됐다. 두 골든은 구엔진(libhangul) 빌트인 출력을 박은 것이라 빌트인 부재 시
REM   keyboard_count 변동으로 mismatch → GATES 에서 제거. 기능 커버리지는 V2 로 이전(이미 존재):
REM     - 2noshift/2n9256/3gs/3sun-2014 = 산들바람 컴파일인 빌트인(임베드 XML + kJasoBuiltin, 외부파일0).
REM     - build_cleanroom_selftests = sel_galma-sel_3gs-sel_3sun14-sel_chordf-sel_dubeol(V2 jaso_layout_*)
REM       + ★sel_r4xml(selftest_jaso_r4xml) = C 정본 ≡ in-memory 왕복 + embed≡C(4종 컴파일인 빌트인) + 디스크 XML byte-동치(shin 7).
REM   3-91-noshift(finalsun)=드롭(순정 9 부재 = 바닐라 미지원)이라 V2 대체 없음. golden_galmadeuli.c-
REM   golden_finalsun.c-.bat 는 구엔진(libhangul) 빌트인을 hangul_ic_new 로 열던 것이라 빌트인 제거
REM   후엔 *재현 불가*(dead archive) — 삭제하지 않고 이력 보존만 하되 GATES-재현 용도 아님. 318na/chord 은퇴 선례.
REM (2026-06-27 F-3 R5 shin-oracle 디태치) build_shin_xml_verify 은퇴 — 이 게이트는 신세벌 빌트인
REM   오라클(hangulkeyboard.c nabicloud_shin_builtin_by_id)로 배포 3shin-1995/2003.xml 을 GEN+byte-diff
REM   했으나, R5 에서 그 오라클(shin 9 static 정의)이 제거됐다(de-fork). 커버리지 이전:
REM     - 1995/2003 = V2/editor shell path self-roundtrip coverage.
REM     - 7 V2-shin(p2-p-2012-2015-m-p-yet-p2-yet) = build_cleanroom_selftests/sel_r4xml on-disk
REM       (배포 XML ≡ V2 canonical dump(JASO_LAYOUT_*)) + sel_rt(C≡XML in-memory).
REM     - 등록 검증 = shell registry + V2Backend gates. shin_xml_verify.c-.bat 는 이력 보존(미빌드).
REM (2026-06-29 F-3 R8 engine-layer drop) build_and_verify_off retired -- golden_off.c
REM   linked baram.c (nabicloud_set_baram_enabled / nabicloud_baram_enabled) to assert
REM   "engine off == pristine libhangul" byte-golden. R8 dropped the 8 old engine-layer TUs
REM   (engine/dispatch/combine/finalsun/galmadeuli/jasoshin/jaso-sebeol/baram.c); with baram.c
REM   gone the toggle no longer exists, so the gate cannot link. Coverage kept by
REM   build_and_verify_all (pristine-9 byte golden) + V2 cleanroom selftests. golden_off.c /
REM   build_and_verify_off.bat preserved as dead archive (not built). off/finalsun/318na precedent.
REM (2026-06-29 F-3 R2-2/R4) build_gate_shell_keymap_equiv = shell-owned builtin keymap byte-equiv.
REM   windows/tsf/nabicloud_builtin_keymaps.h == engine builtin map_to_char for pristine9.
REM   External XML moved to V2Backend/cleanroom gates; this gate compiles no engine XML loader.
REM (2026-06-29 F-3 R4) build_gate_editor_preview_equiv = legacy standard-XML preview fallback removed.
REM   3-89 and 3sun-1990 are V2 XML; build_jaso_editor_verify covers preview composition.
REM (2026-06-29 F-3 R4 option 4) engine XML loader removed. The remaining aggregate
REM   keeps pristine-9 byte coverage and V2/editor-shell coverage only. Retired
REM   historical gates: build_and_verify_dubeol_jong, build_dubeol_xml_verify,
REM   build_semoe_xml_verify, build_editor_roundtrip_verify, build_editor_e2e_verify,
REM   build_editor_jsc_diff. They require the old engine XML/editor path and are
REM   no longer meaningful for the target architecture.
REM ★게이트 분류 (2026-07-02, 3REPO openDecision #5 / DECISIONS §49; 2026-07-03 카운트 정정):
REM   CORE = build_and_verify_all (엔진 트리 자기완결 — 공개 nabicloud-engine 단독 클론서 실행 가능).
REM   INTEGRATION = 나머지 12(루프) + WU9(JS) = 13 (본체-레벨: cleanroom/ · windows/ · shared\data\keyboards
REM     픽스처 필요) — 각 .bat 상단 가드가 fixture 부재 시 exit 77 graceful-SKIP(아래 [SKIP] 표시, FAIL 아님).
REM     WU9 는 별도 블록(node 스크립트라 exit 77 관례 아님)이나 windows\settings\tests fixture 부재 시
REM     동일하게 SKIP 처리(아래 :run_wu9 앞 가드). 본체 superproject(전 서브모듈 마운트)에선 전량 실행.
REM     SKIP 코드 77 = cl.exe exit 2 와 충돌 방지.
set "GATES=build_and_verify_all build_jaso_editor_verify build_editor_jsc_diff_v2 build_vm_editor_verify build_cleanroom_selftests build_abbrev_chord_v2_verify build_dubeol_abbrev_verify build_v2backend_importzero build_gate_standalone_runtime build_gate_shell_keymap_equiv build_gate_editor_preview_equiv build_gate_shell_include_boundary build_drift_check"
echo == NabiCloud all engine gates ==
for %%G in (%GATES%) do (
  if exist "%HERE%%%G.bat" (
    call "%HERE%%%G.bat" >nul 2>&1
    if "!ERRORLEVEL!"=="77" ( echo   [SKIP] %%G  ^(INTEG fixture absent - run from superproject^) ) else if !ERRORLEVEL! GEQ 1 ( echo   [FAIL] %%G  ^(re-run tests\%%G.bat for detail^) & set /a FAILS+=1 ) else ( echo   [PASS] %%G )
  ) else ( echo   [MISS] %%G.bat not found & set /a FAILS+=1 )
)
REM WU9 [INTEG] — 편집기 코어 JS 단위테스트. windows/settings/tests 는 본체 superproject
REM 전용(공개 nabicloud-engine 단독 클론엔 없음) — 다른 INTEG 게이트와 동일하게 fixture
REM 부재 시 SKIP(게이트 실패 아님). node 미설치도 SKIP.
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
REM node --test 는 디렉터리 인자를 모듈로 오인 → 폴더로 이동 후 cwd glob 으로 돈다.
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
