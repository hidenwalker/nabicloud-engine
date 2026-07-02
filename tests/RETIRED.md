# 은퇴 게이트 대장 (RETIRED gates ledger)

> 이 디렉터리에는 **은퇴한(retired) 게이트/골든 파일**이 이력 보존(dead archive) 목적으로 남아 있다.
> 은퇴 파일은 최상단에 `RETIRED` 배너(.bat=REM 블록, .c=`/* RETIRED ... */`)가 붙어 있으며,
> `run_all_gates.bat` 의 `GATES` 목록에 **포함되지 않는다**(= 살아있는 게이트가 아님).
> 은퇴 사유 원문은 `run_all_gates.bat` 상단 REM 이력 주석이 정본이고, 이 문서는 그 정리본이다.
> 현행 게이트 목록은 `run_all_gates.bat` 의 `set "GATES=..."` 한 줄이 유일 정본.

## 1. 파일이 남아 있는 은퇴 게이트 (배너 부착 완료, 2026-07-02)

| 파일 | 은퇴일 | 사유(한 줄) | 커버리지 이전처 |
|---|---|---|---|
| `build_and_verify_318na.bat` + `golden_318na.c` | 2026-06-25 | 318na 가 V2 XML 로 이관 — 구엔진 오라클은 V2 jaso-map 본문을 읽지 못함 | cleanroom `selftest_jaso_318na` + `selftest_jaso_roundtrip` |
| `build_shin_xml_oracle.bat` + `diff_shin_xml.c` | 2026-06-26 (전수정리) | libhangul-경로 신세벌 오라클 — 신세벌 XML V2 이관 후 chord 빈 출력(무의미) | `build_shin_xml_verify`(→이후 R5 에서 재은퇴) + cleanroom `selftest_jaso_shinp2` |
| `build_and_verify_galmadeuli.bat` + `golden_galmadeuli.c` | 2026-06-27 (F-3 R4 de-fork) | 포크 빌트인 2noshift/2n9256 이 순정 libhangul 0.2.0 재배이스로 제거 — 구엔진 출력 골든 재현 불가 | V2 컴파일인 빌트인(임베드 XML + kJasoBuiltin) + `build_cleanroom_selftests`(sel_galma·sel_r4xml 등) |
| `build_and_verify_finalsun.bat` + `golden_finalsun.c` | 2026-06-27 (F-3 R4 de-fork) | 포크 빌트인 3-91-noshift(finalsun) 제거 — 순정 9 부재로 드롭, 재현 불가 | **없음**(3-91-noshift 는 바닐라 미지원 = V2 대체 없이 드롭) |
| `build_shin_xml_verify.bat` + `shin_xml_verify.c` | 2026-06-27 (F-3 R5 shin-oracle 디태치) | 신세벌 빌트인 오라클(`nabicloud_shin_builtin_by_id`, shin 9 static)이 de-fork 로 제거 | 1995/2003=V2/editor 셸 self-roundtrip · 7 V2-shin=`build_cleanroom_selftests` sel_r4xml(on-disk)+sel_rt · 등록검증=shell registry+V2Backend 게이트 |
| `build_and_verify_off.bat` + `golden_off.c` | 2026-06-29 (F-3 R8 engine-layer drop) | baram.c(엔진 off 토글)를 링크해 "off==순정 libhangul" byte-골든 — R8 로 옛 엔진레이어 8 TU drop 후 링크 불가 | `build_and_verify_all`(순정-9 byte 골든) + V2 cleanroom selftests |
| `build_and_verify_dubeol_jong.bat` + `dubeol_jong_selftest.c` | 2026-06-29 (F-3 R4 option 4) | 엔진 XML 로더 제거 — 구 엔진 XML 경로 필요, 목표 아키텍처에서 무의미 | 순정-9 byte 골든 + V2/편집기-셸 게이트 |
| `build_dubeol_xml_verify.bat` + `dubeol_xml_verify.c` | 2026-06-29 (F-3 R4 option 4) | 상동 (엔진 XML 로더 제거) | 상동 (두겹이 chord 는 cleanroom chord_live + G8 chord_oracle_parity 가 커버) |
| `build_semoe_xml_verify.bat` + `semoe_xml_verify.c` | 2026-06-29 (F-3 R4 option 4) | 상동 (엔진 XML 로더 제거) | 상동 |
| `build_editor_roundtrip_verify.bat` + `editor_roundtrip_verify.c` | 2026-06-29 (F-3 R4 option 4) | 상동 (구 엔진 editor 경로 필요) | `build_jaso_editor_verify`(현행) 등 V2/편집기-셸 게이트 |
| `build_editor_e2e_verify.bat` + `editor_e2e_verify.c` | 2026-06-29 (F-3 R4 option 4) | 상동 | 상동 |
| `build_editor_jsc_diff.bat` + `editor_jsc_dump.c` | 2026-06-29 (F-3 R4 option 4) | 상동 | 상동 (JS 측은 WU9 editor-core JS 테스트) · ★2026-07-02 V2판 재신설 = `build_editor_jsc_diff_v2.bat`(`editor_jsc_dump_v2.cpp`+`jsc-diff-v2.js`) — C V2 직렬화(jaso_editor_serialize_by_id+셸 메타주입)↔JS(parseKeyboardXml→serializeKeyboard) byte-parity 복원(단독 게이트, GATES 미편입) |

주: `.c` 파일들은 어떤 현행 게이트에서도 컴파일되지 않는다(dead archive, 미빌드).
특히 `golden_galmadeuli.c`·`golden_finalsun.c`·`golden_off.c` 는 제거된 구엔진 빌트인/TU 에
의존하므로 **재현 자체가 불가** — 삭제하지 않고 이력 보존만 한다 (318na/chord 은퇴 선례).

## 2. 파일까지 삭제된 은퇴 게이트 (git 이력에만 존재)

| 게이트 | 은퇴일 | 사유(한 줄) | 커버리지 이전처 |
|---|---|---|---|
| `build_and_verify_baram` / `build_and_verify_3gs` (+`golden_baram.c`·`golden_3gs_dual.c`) | 2026-06-21 (통합) | 옛 baram 병렬 impl(baram_process/baram_dispatch) 직접 구동 byte-골든 — 병렬 impl 을 baram.c(ops vtable 통합)로 흡수하며 폐기 | (당시) all/galmadeuli/finalsun/chord 골든 |
| `build_and_verify_chord` / `build_and_verify_dubeol_chord` / `build_anmate_chord_verify` | 2026-06-26 (F-2 libhangul 포크 chord 엔진 제거) | 제거된 engine-chord.c / engine-chord-dubeol.c 를 직접 컴파일하던 셀프테스트 — chord 는 V2 전용화 | cleanroom `chord_live` selftest 45/45 + G8 `chord_oracle_parity` 15/15 (V2==libhangul NFC) |

## 3. 부록 — run_all_gates.bat 이력 주석 mojibake 복원 (줄 28~30)

`run_all_gates.bat` 줄 28~30 은 커밋 시점에 이미 인코딩이 깨진 채(???) 들어갔다
(커밋 `e441b7fc` 의 diff 자체가 mojibake — 원문 바이트는 git 에 없음). 같은 커밋의
커밋 메시지 [B2] 항목이 동일 내용을 정상 한글로 담고 있고, mojibake 의 어절 길이
패턴과 전부 일치하여 아래와 같이 복원한다. **이 항목은 은퇴 기록이 아니라
`build_cleanroom_selftests`(현행 GATES 소속 — 살아있는 게이트)의 신설 기록이다.**

> (2026-06-26) build_cleanroom_selftests = 클린룸 jaso 셀프테스트(과거 로컬 _runtests 추적본,
> find_vcvars 로 vcvars 탐색) 일괄 \*정직\* 게이트. ahn_chord(f5de938b)·chord_family 또(c7584dc2)
> 류 회귀를 커밋 시점에 차단(런타임 실패 집계).

(run_all_gates.bat 안의 mojibake 줄 자체는 인코딩 재손상 위험 때문에 수정하지 않는다.)

## 4. 비고 — 분류 보류

- `build_and_verify.bat` + `golden.c` (+ `golden.txt` 기준선): 최초(리스트럭처 이전) 3gs
  characterization 골든. `run_all_gates.bat` REM 이력에 **은퇴 기록이 없고** 현행 GATES 에도
  없다(2026-06-26 GATES 신설 시점부터 미포함 — `build_and_verify_all` 이 사실상 상위 호환).
  은퇴 여부 미확정이므로 배너를 달지 않고 보류한다.
- `_probe*.c`·`diff_shin.c`·`diff_sebeol.c`·`diff_semoe.c`·`_oracle_*.c` 등은 게이트가 아니라
  클린룸 provenance/프로브 자료이므로 이 대장의 대상이 아니다.
