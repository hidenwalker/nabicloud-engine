# tests/ — libhangul-nabicloud 골든(특성화) 게이트

`nabicloud/libhangul` 재구조화의 **동작 동등성 합격선**. 동작 보존 단계는 이 게이트가
`GOLDEN_PASS` 를 유지해야 한다.

## 실행

```
tests\run_all_gates.bat      REM ★전 게이트 일괄(아래 인벤토리 전부) — ALL_GATES_PASS(exit0)/GATES_FAIL(n)
tests\build_and_verify.bat   REM 단일 게이트(3gs 기본 골든)
```

## 런타임 체크리스트 (반)자동화 — jaso 카탈로그 (빌드 0·정적)

변경셋에서 **실측-런타임이 필요한 자판/기제만** 추려 [RUNTIME_TEST_CHECKLIST.md](../../../docs/RUNTIME_TEST_CHECKLIST.md)
라운드를 (반)자동 조립하는 1단계 정적 도구. 정본 설계 = [docs/TEST_CHECKLIST_AUTOMATION_DESIGN.md](../../../docs/TEST_CHECKLIST_AUTOMATION_DESIGN.md) §11.
게이트를 **건드리지 않는다**(순수 additive, run_all_gates 무관). 데이터는 전부 라이브 소스 도출(드리프트 없음).

★**T5(59번, 2026-07-23) 흡수**: 실제 구현은 `raindrop_testsys/jaso_catalog.py`(공용 테스트 패키지)로
이동했다 — **정본은 거기, 아래 `runtime_checklist_catalog.py` 는 기존 호출부(`build_drift_check.bat`·
gates.json `engine.drift_check`) 하위호환용 위임 shim**(출력/exit code byte-동치 검증됨). 새 사용은
아래 두 방식 어느 쪽이든 동일:

```
python -m raindrop_testsys select <변경파일..> --root <repo>   REM 정본 진입점(단일 driver 와 동일 --root 관례)
python runtime_checklist_catalog.py select <변경파일..>         REM 이 디렉터리에서 그대로(shim, 동일 출력)
```

커맨드 7종(둘 다 동일하게 지원): `select`(변경→영향 자판·기제·라운드·게이트앵커, 없으면 git diff) ·
`catalog`(레이아웃×기제 매트릭스+id-registry UNION 덤프) · `census`(selftest∩빌드manifest 감사) ·
`check`(드리프트 게이트: known-dropped sync+기제필드 분류) · `collect`(2단계: 계측 selftest -DEMIT_JSON
빌드+실행→방출 파싱→한글 디코드) · `verify`(2단계 검증패스: RUNTIME_TEST_CHECKLIST 수기 행 ↔ 엔진 방출 대조) ·
`pairs`(대칭짝 자동조립: backspace짝·fire/non-fire짝 검출, §1).

★새 자판(외부 XML·빌트인 레이아웃·NSI 본체 승격)은 **도구 무수정 자동 반영**(glob+정규식). 유일한 수기
터치포인트 = 신규 *기제*(jaso_layout 새 필드) — `check` 의 M3b 드리프트가 미분류를 fail 로 강제(조용한 누락 방지).

★**`build_drift_check.bat` = run_all_gates 편입(build 0)**: 매 게이트 실행 시 `check`(known-dropped sync +
기제필드 분류) + `census`(selftest∩manifest) 를 자동 실행 → 레지스트리/기제 드리프트를 시끄러운 게이트 실패로
승격(M3). python 부재 시 skip(exit 0). collect/verify(cl 빌드 필요)는 게이트 미편입 — 수동/후속.

**2단계 instrument-and-run(착수)**: `selftest_emit.h`(공유 방출 훅 — `-DEMIT_JSON` off=완전 no-op·게이트 무영향)를
selftest 에 얹으면 각 케이스가 `{id,input(실제 키),expected[],got[],altitude}` JSON 을 방출한다. `collect` 가 계측
파일을 자동 감지해 `build_one_emit.cmd` 로 빌드+수집한다. 현재 계측 **19 파일 = 563 케이스**(jaso 커밋-오라클
계열 대부분: p2·shinp·none·virtualunit·shinshift·318na·2012·390·gongdong·3sun2014·dubeol·dubeol_std·yet +
XML 로더 314·314banja·s2015·s2015p·s2015m·shinhs). control-char(`/`=reset·`.`=flush·`<`=backspace)는 `EMIT_OP`.
id 를 드라이버가 모르는 파일(yet)은 `EMIT_STASH`(out_is)→`EMIT_FLUSH`(report) 지연 방출로 호출부 무수정.
★**M2 독립앵커 활성**: 318na g01~g12 는 `golden_318na.c`(libhangul 독립 하니스)와 교차검증되어 `collect` 에서
`✓validated(golden_318na)` — 방출 expected 가 golden 과 어긋나면(co-wrong 오타) **hard-fail**(exit 1). golden 미커버
행은 `lit-anchor·golden부재`(selftest-derived, validated 주장 안 함=B3). ★배치는
**CRLF+ASCII**(cmd.exe 는 LF label/goto 를 오파싱). 정본 설계 = [docs/TEST_CHECKLIST_AUTOMATION_DESIGN.md](../../../docs/TEST_CHECKLIST_AUTOMATION_DESIGN.md) §12.

## 게이트 인벤토리 (run_all_gates.bat 가 호출)

| 게이트 bat | PASS 토큰 | 커버 | 런타임 추가검증 |
|---|---|---|---|
| `build_and_verify_all` | `ALL_PASS` | 빌트인 byte 골든(종합) — 기본 ON 출력이 동결 baseline 과 byte-등가 | — |
| `build_and_verify_off` | `OFF_PASS` | 마스터 게이트 OFF=vanilla 봉인(표준 ON==OFF / 3gs ON!=OFF) | — |
| `build_and_verify_318na` / `_galmadeuli` / `_finalsun` | `318NA_PASS`/`GALMADEULI_PASS`/`FINALSUN_PASS` | 기능(clean-room·승격) 동치 | — |
| `build_and_verify_chord` | `CHORD_SELFTEST_PASS` | 세벌 chord compose(세모이) | — |
| `build_and_verify_dubeol_chord` | `DUBEOL_CHORD_SELFTEST_PASS` | 두벌 V-start compose(겹받침 연음 분할 포함) | — |
| `build_and_verify_dubeol_jong` | `DUBEOL_JONG_SELFTEST_PASS` | 두겹이 겹받침 자동전환(`<key-context fill-jong>` 데이터주도) | — |
| `build_anmate_chord_verify` | `ANMATE_CHORD_VERIFY_PASS` | 안마태 3-ahn(type 1006+loose-order, fjv→각) | ★셸 동시타 상태기계 |
| `build_dubeol_xml_verify` / `_semoe_xml` / `_shin_xml` | `*_XML_ALL_PASS` 등 | 외부 XML 로드(type·flag·등록) | — |

★ "런타임 추가검증"= 셸 chord 상태머신(동시타 수집·타이밍·게이트)은 골든·셀프테스트가 못 덮음 → 머신 실입력 필요(DECISIONS §29 / DUBEOL_CHORD_DESIGN §10).

VS2022 BuildTools 환경(`vcvars64.bat`)을 자동 호출해 엔진 3파일
(`hangulinputcontext.c`·`hangulctype.c`·`hangulkeyboard.c`)과 `golden.c` 를 빌드,
출력을 `golden.txt` 와 byte 비교한다.

- `GOLDEN_PASS` (exit 0) → 동작 불변.
- `GOLDEN_FAIL` (exit 1) → 빌드 오류 또는 동작 회귀. `tests\_diff.log` 확인.

## 파일

- `golden.c` — 3gs 엔진 특성화 하니스. ASCII-only(BOM/CP949 함정 회피).
  전 키 단건·전 키쌍 스윕 + 큐레이션 음절(많/얹/삶/괘·반복모음·모드키) +
  backspace/reset/flush-mid 시나리오. commit/preedit 를 U+XXXX hex 로 덤프.
- `golden.txt` — **동결 베이스라인(현재 코드 기준). 손편집 금지.**
- `build_and_verify.bat` — 빌드+실행+diff 게이트.

## 골든 갱신이 정당한 경우

리팩토링은 동작 보존이므로 golden.txt 를 바꾸면 **안 된다**. 의도적으로 동작이
바뀌는 변경(버그 수정 등)일 때만, 그 사유를 커밋 메시지에 명시하고
`golden.exe > golden.txt` 로 재생성한다.

---

## 이중 게이트 모델 (산들바람 통합 후, 2026-06-21 재기준)

골든은 **두 종류**가 공존하며 역할이 다르다. 둘은 **누적·병행**이지 대체가 아니다.

산들바람 통합(0bfdfd6)에서 마스터 게이트 기본값이 `false→true`(기본 ON)로 바뀌었고,
옛 병렬 BaramPolicy automata(`baram_process_*`/`baram_dispatch`)·그 동치 증명용
`golden_baram.c`·`golden_3gs_dual.c` 는 **은퇴(git rm)**했다. 이제 NabiCloud family
라우팅은 `baram.c` 의 ops vtable 단일 경로(`nabicloud_baram_lookup(type)→ops->process`)뿐이다.

### (1) byte 골든 — 내부 리팩토링 무회귀
빌트인 자판을 **같은 키 시퀀스**로 돌려 commit/preedit 의 **낱자(jamo) 코드포인트까지
byte 동일**을 요구한다. 내부 구조 변경이 관측 동작을 한 비트도 바꾸지 않았음을 증명한다.

- `golden_all.{c,txt}` + `build_and_verify_all.bat` → `ALL_PASS`
  (전 빌트인 자판 × 옵션축 def/ar1/ds1/nc1). 이제 **기본 ON** 으로 돌며, 그 출력이
  동결 baseline(=옛 OFF 경로 출력)과 **byte-등가**임을 증명한다. **동결 .txt 무수정.**
- `golden_off.c` + `build_and_verify_off.bat` → `OFF_PASS`
  (마스터 게이트 OFF=vanilla 봉인). 동결 baseline 없이 **자체검증 불변식** 2개:
  표준 자판("2"/JAMO)은 ON 출력==OFF 출력 byte-동일(게이트가 vanilla type 엔 no-op),
  NabiCloud 자판("3gs"/NABICLOUD 99)은 ON 출력!=OFF 출력(게이트가 OFF 모드서 실제로
  NabiCloud 엔진을 끈다). 둘이 합쳐 OFF==순수 vanilla 를 증명한다.

### (2) 기능(약) 골든 — clean-room·승격 동치
외부 XML 자판처럼 byte 골든이 못 덮는 경로를, **공개 스펙에서 유도한 기대값** 또는
**코드경로 vs 데이터경로 동치**로 검증한다. 입력은 물리 키 시퀀스, 기대는 NFC 음절/동치.
이 계열은 모두 마스터 게이트 **항상 ON** 으로 돌며(baram ops 단일 경로), 그 출력이
각자의 동결 기대값과 같음을 본다.

- `golden_318na.c` + `build_and_verify_318na.bat` → `318NA_PASS`
  (외부 318na.xml 의 키시퀀스→NFC 음절을 스펙 유도값과 대조).
- `golden_finalsun.c` + `build_and_verify_finalsun.bat` → `FINALSUN_PASS`
  (FINALSUN 포트 출력 == 동결 golden_finalsun.txt == 3beol 레퍼런스). g_new 는
  무해 잔존(옛 old/new 등가축 폐기, ops 단일 경로). 게이트 소스 상단 배너 참고.

### 공유 자산
- `golden_util.h` — `fmt()`(ucschar→"U+XXXX") **1회 추출**. golden_all 등이 include.
  (golden_318na 는 누적-then-render 구조가 달라 inline 유지 — 강제 공유 금지.)
  각 파일의 suite/expect/main 은 의도적으로 **파일별 유지**(over-engineering 가드).

## 마스터 게이트 기본 ON (산들바람 = 정상 엔진)

마스터 게이트 플래그 `s_baram_enabled` 는 **기본 true(기본 ON)**다 — NabiCloud(산들바람)가
정상 엔진이고 vanilla(기존 libhangul 엔진)는 명시적 opt-out 이다. 프로덕션 TSF 는 자판 활성화 시
`use_old_engine`(==0 → ON)으로 set 한다. 골든 하네스도 기본 ON 으로 돌고(golden_all/
golden_finalsun 등), OFF 경로는 `golden_off` 가 봉인한다. `NABICLOUD_BARAM` env override
는 `#ifndef NDEBUG` 디버그 한정.

## golden_all 미커버 자판의 별도 동결

byte 골든(golden_all)은 **빌트인만** 베이스라인으로 동결한다. 다음은 별도 게이트로
**누적-commit byte 동결**한다:

- 옛한글(yethangul) 계열·318na 등 **외부/특수 자판** — golden_318na 처럼 전용 기능
  게이트가 누적 byte/NFC 를 잡는다.

## 옛한글 / NFC 정직 처리

기본 입력 모드는 SYLLABLE 이라 NFC 정규화(`hangul_jamos_to_syllables`)는
**일반 현대음절에는 사실상 no-op**(이미 NFC 음절)이고, **옛한글(U+11xx 첫가끝 자모)**
은 통합 음절이 없어 **자모 그대로 통과** → 약골든의 표현형이 **byte 와 동일**해진다.
즉 옛한글에 한해 NFC 토큰은 byte 토큰과 같은 것을 본다(추가 흡수 없음).

따라서 NFC 약골든의 **핵심 가치는 NFC 가 실제 타이밍 차이를 흡수하는** 경우 —
현대자판 + 외부자판 + 승격 대상 — 에 한정한다. 전 빌트인에 NFC 흡수를 일괄
기대하지 않는다(옛한글은 raw 자모=byte 동일이 정직한 결과).
