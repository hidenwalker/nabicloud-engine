/* selftest_roster.h — 클린룸 selftest 로스터 **단일 정본** (X-macro).
 *
 * SELFTEST(stem, kbid, desc)
 *   stem = canonical selftest 소스 basename(.c 제외; cleanroom/impl/clean/canonical/)
 *   kbid = 단일-레이아웃 귀속 자판 id ("-" = 다중 레이아웃/미등록 → 귀속 불가)
 *   desc = "게이트명 — 요약" (게이트명 = build_cleanroom_selftests.bat 의 :build NAME)
 *
 * 소비자·동기화 (정직게이트 — 조용한 어긋남 차단):
 *   - build_cleanroom_selftests.bat: 컴파일 대상·srcs·실행 순서의 *실행 정본*은 여전히 .bat
 *     (행마다 srcs 목록이 달라 X-macro 3필드로 표현 불가·cmd 파싱 취약 → .bat 직소비 안 함).
 *   - runtime_checklist_catalog.py `check`: 이 로스터 ↔ .bat manifest **양방향 대조**(불일치=FAIL)
 *     + kbid 가 id-registry 에 실재하는지 검증. build_drift_check(run_all_gates) 편입.
 *   - runtime_checklist_catalog.py `verify`: kbid 열이 FILE_TO_ID(자판귀속 드리프트 게이트)의 원천
 *     (과거 .py 수기 dict 14종을 이 파일로 수렴 — 2026-07-02).
 *   - census(디스크 selftest*.c ↔ .bat) 와 합쳐 디스크·.bat·로스터 삼각 폐쇄.
 *
 * ★새 selftest 추가 = .bat `call :build` 행 + 여기 SELFTEST 행 동시 (한쪽 누락 → check/census FAIL).
 *   .py 는 이 파일을 regex 로 파싱하므로 행 형식(한 줄 = SELFTEST(stem, "kbid", "desc")) 유지.
 */
#ifndef SELFTEST
#error "define SELFTEST(stem, kbid, desc) before including selftest_roster.h"
#endif

SELFTEST(selftest,                     "-",           "sel_galma — galmadeuli 동결 오라클(oracle_galmadeuli)")
SELFTEST(selftest_shinp2,              "-",           "sel_shinp2 — 신세벌 P2 동결 오라클(oracle_shin_p2)")
SELFTEST(selftest_jaso_2012,           "3shin-2012",  "sel_2012 — 신세벌식 2012 jaso_strat")
SELFTEST(selftest_jaso_390,            "39",          "sel_390 — 세벌식 390 jaso_strat")
SELFTEST(selftest_jaso_chord,          "-",           "sel_chord — 세벌최종 chord(동시타)")
SELFTEST(selftest_jaso_dubeol_chord,   "-",           "sel_dchord — 두벌 표준 chord")
SELFTEST(selftest_jaso_filljong,       "-",           "sel_filljong — 채움종성 XML 로더 경로")
SELFTEST(selftest_jaso_dubeol,         "-",           "sel_dubeol — galmadeuli 레이아웃 vs 동결 오라클(2noshift/2n9256 다중 id)")
SELFTEST(selftest_jaso_dubeol_std,     "2",           "sel_dbstd — 두벌 표준 jaso_strat")
SELFTEST(selftest_romaja,              "-",           "sel_romaja — 로마자 음역(romaja_translit)")
SELFTEST(selftest_baram_hanja,         "-",           "sel_hanja — 한자 사전(baram_hanja)")
SELFTEST(selftest_hanja_diff,          "-",           "sel_hdiff — 한자 사전 차분")
SELFTEST(selftest_jamo_compat,         "-",           "sel_jamo — 자모 호환 변환(jamo_compat)")
SELFTEST(selftest_combine_integrity,   "-",           "sel_integ — 결합표 무결성(전 레이아웃 정적)")
SELFTEST(selftest_combine_engine,      "-",           "sel_cmbeng — P0-a 결합표 엔진구동 4경로 + 보편-조합 오라클")
SELFTEST(selftest_backspace_symmetry,  "-",           "sel_bsp — P0-b backspace 대칭(역함수) 전 자판")
SELFTEST(selftest_oracle_diff,         "-",           "sel_odiff — P0-c 동결 오라클 vs jaso_strat 전수 차분")
SELFTEST(selftest_jaso_3sun2014,       "3sun-2014",   "sel_3sun14 — 3-순아래 2014 jaso_strat")
SELFTEST(selftest_jaso_314_xml,        "3-314",       "sel_314xml — 3-314 외부 XML")
SELFTEST(selftest_jaso_314banja_xml,   "3-314-banja", "sel_314ban — 3-314 반자 외부 XML")
SELFTEST(selftest_jaso_stdxml,         "-",           "sel_stdxml — 표준 XML 로더")
SELFTEST(selftest_jaso_ahn_chord,      "-",           "sel_ahnchd — 안마태 chord XML")
SELFTEST(selftest_jaso_shinhs_xml,     "3shin-hs",    "sel_shinhs — 신세벌 HS 외부 XML")
SELFTEST(selftest_jaso_s2015_xml,      "3-2015",      "sel_s2015 — 3-2015 외부 XML")
SELFTEST(selftest_jaso_s2015p_xml,     "3-2015P",     "sel_s2015p — 3-2015P 외부 XML")
SELFTEST(selftest_jaso_s2015m_xml,     "3-2015M",     "sel_s2015m — 3-2015M 외부 XML")
SELFTEST(selftest_jaso_chamshind_xml,  "chamshin-d",  "sel_chamd — 참신세벌식 D 외부 XML(.ist 유도, §50)")
SELFTEST(selftest_jaso_fourz,          "-",           "sel_fourz — 4z vm_strat(타자연습)")
SELFTEST(selftest_jaso_vmxml,          "-",           "sel_vmxml — vm XML 로더")
SELFTEST(selftest_jaso_gongdong,       "-",           "sel_gongd — 공동체 자판(미등록·selftest-only)")
SELFTEST(selftest_jaso_gongdong_xml,   "-",           "sel_gongdx — 공동 XML 왕복 트윈(moa coda 술어 직렬화 잠금, §52)")
SELFTEST(selftest_jaso_3x,             "-",           "sel_3x — 슬롯-균일 가상단위(3x 흡수 §54): C직접+XML왕복 이중구동")
SELFTEST(selftest_jaso_none,           "-",           "sel_none — none 전략(3f+P2+galma 혼재)")
SELFTEST(selftest_jaso_p2,             "3shin-p2",    "sel_p2 — 신세벌 P2 jaso_strat(+동결 오라클)")
SELFTEST(selftest_jaso_shinp,          "3shin-p",     "sel_shinp — 신세벌 P jaso_strat")
SELFTEST(selftest_jaso_shinshift,      "-",           "sel_shinsh — shin-shift 횡단(다중 레이아웃)")
SELFTEST(selftest_jaso_virtualunit,    "-",           "sel_vunit — 가상유닛 횡단(다중 레이아웃)")
SELFTEST(selftest_jaso_yet,            "-",           "sel_yet — 옛한글(다중 레이아웃)")
SELFTEST(selftest_jaso_multiemit,      "-",           "sel_multi — 다중 방출 코어")
SELFTEST(selftest_jaso_chord_family,   "-",           "sel_chordf — chord 패밀리 횡단")
SELFTEST(selftest_jaso_chord_live,     "-",           "sel_chordlv — chord 라이브(XML 포함)")
SELFTEST(selftest_jaso_3gs,            "3gs",         "sel_3gs — 3gs(순아래·chord) jaso_strat")
SELFTEST(selftest_jaso_318na,          "318na",       "sel_318na — 318na jaso_strat(golden 독립앵커)")
SELFTEST(selftest_jaso_roundtrip,      "-",           "sel_rt — C 정본 ≡ XML 왕복(다중 레이아웃)")
SELFTEST(selftest_jaso_r4xml,          "-",           "sel_r4xml — F-3 R4 외부 V2 XML 이관 충실성")
SELFTEST(selftest_jaso_r4_legacy_parity, "-",         "sel_r4par — R4 legacy-oracle ≡ V2 XML 해시 패리티")
