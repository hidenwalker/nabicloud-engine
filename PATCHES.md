# PATCHES.md — 상류 libhangul 대비 NabiCloud 차이 (기능별 provenance)

이 포크는 LGPL-2.1 §2(b)에 따라 *변경한 사실과 날짜*를 명시한다. 아래는 upstream
libhangul 0.2.0 대비 의도적 차이를 **기능 단위**로 정리한 것이다. 파일별 고지는 각
소스 상단 주석에도 있다. (재구조화 Steps 2~7 진행 시 코드가 이동하면 본 문서의
"이전 후 위치" 열을 갱신한다.)

| # | 기능 | 출처 계보 | 현재 위치 | 라이선스 | 비고 |
|---|------|-----------|-----------|----------|------|
| P1 | sunarae(순아래) 조합 엔진 (`TYPE_NABICLOUD=99`, 구 `_GUREUM`, process/push/backspace + 318na multikey 전처리) | gureum/libhangul (0.1.0 포크) | `nabicloud/engine/engine.c`(본체: `nabicloud_engine_process`/`_backspace`/`_push`/`_buffer_backspace`/`_multikey_proc`) + `hangul/engine.h` 선언, `hangul/hangulinputcontext.c`(얇은 디스패치 hook 3지점) | LGPL-2.1 (병합본) + BSD 고지(GUREUM-COPYING) | **Step4 완료(2026-06-15)**: 엔진을 `hangul/hangulinputcontext.c` 의 static `hangul_ic_process_gureum`/`hangul_ic_push_gureum`/`hangul_buffer_backspace_gureum` 에서 `nabicloud/engine/engine.c` 로 VERBATIM 이전(로직 무변). upstream 입력컨텍스트는 type==NABICLOUD 디스패치 hook(본처리 switch·backspace 분기·multikey 전처리)만 보유 |
| P2 | 3gs 정적 자판 테이블 + 조합 테이블 | gureum 자판 데이터 | `nabicloud/layouts/layout-3gs.c`(데이터) + `nabicloud/layouts/layout-3gs.h`(extern 선언) | LGPL-2.1 | **Step5 완료(2026-06-15)**: 생성 헤더 `hangulkeyboard_gureum_3gs.h` 의 정적 배열을 `layout-3gs.c` 로 이전(데이터 캡슐화). `hangulkeyboard.c` 의 static const `HangulKeyboard`/`HangulCombination` 래퍼는 그대로 두되 extern 배열을 가리키도록만 변경, 컴파일타임 크기는 `NABICLOUD_LAYOUT_3GS_COMBINATION_LEN` 매크로(.c 에서 실배열 길이 static_assert). `_HangulCombinationItem` 정의는 `hangulinternals.h` 로 노출 |
| P3 | 3gs 특수문자/단독모음 표시 보정 (ㅚ·ㅟ) | NabiCloud | `nabicloud/layouts/layout-3gs.c` (`translate_special`) | LGPL-2.1 | **Step5 완료(2026-06-15)**: 엔진 진입부의 인라인 분기(빈 조합 상태에서 ㅟ→`'`, ㅚ→`/`)를 `NabiCloudLayoutOps.translate_special` 콜백으로 분리. 공통 엔진(`nabicloud/engine/engine.c`)은 레이아웃별 특수처리를 갖지 않고 콜백만 호출. 동작 동일(골든 불변) |
| P4 | PUA 가상자모→첫가끝 정규화 (`hangul_vkeycode_to_keycode`) | navilera/NavilIME 적응 | `nabicloud/compatibility/vkeycode.c`(본체) + `hangul/hangulinputcontext.c`(엔진 경계 1회 fold) | LGPL-2.1 | **Step3 완료(2026-06-15)**: 본체를 `vkeycode.c` 로 이전, `hangulctype.c` 11곳 fold 제거→upstream 복원. fold 는 입력 엔진 진입부(`hangul_ic_process_gureum` 등 TU)에서 투명 wrapper(`IC_FOLD`)로 1회. 조합 테이블은 PUA-keyed 라 raw 값이 `hangul_keyboard_combine` 로 흐르는 경로는 유지 |
| P5 | 외부 자판 XML 로더 (expat 제거→내장 mini XML 파서) + win32 builtin 우선검색 | NavilIME + NabiCloud | `nabicloud/win32/keyboard-loader.c`(본체: 파서·디렉토리 스캔 `nabicloud_keyboard_load_from_path`·경로해석) + `hangul/hangulkeyboard.c`(빌트인 자판 테이블·전역 리스트 라이프사이클만) | LGPL-2.1 | **Step6 완료(2026-06-15)**: XML 파서·`hangul_keyboard_list_load_dir`·builtin-우선 검색을 `hangul/hangulkeyboard.c` 에서 `nabicloud/win32/keyboard-loader.c` 로 이전하며 K1~K4 하드닝 동반 적용. 공개 ABI(`hangul_keyboard_new_from_file`/`hangul_keyboard_list_load_dir`) 무변. ★정정(2026-07-01): P5 미니파서(`keyboard-loader.c`)는 ④에서 제거·순정복귀 완료. **libhangul 엔진 모드(opt-in) XML 로딩용으로 상류 expat 로더 재활성** = `ENABLE_EXTERNAL_KEYBOARDS` on + vendored expat R_2_6_4(`expat/`, MIT — NOTICE §7). libhangul.vcxproj 빌드옵션만(핵심3 무수정) |
| P6 | 한자 사전 파서 메모리 안전 하드닝 (`hanja_parse_line` 통합, 후보 상한, 빈 테이블 가드) | NabiCloud | `hangul/hanja.c` | LGPL-2.1 (원본 BSD-3 고지 별도 보존) | commit `0c28e94` (2026-06-14) |
| P7 | Windows 적응 선언 (`hangul_keyboard_list_load_dir` 등) | NavilIME | `hangul/hangul.h`, `hangul/hangulkeyboard.h` | LGPL-2.1 | |
| P8 | 갈마들이(좌우) 조합 엔진 + 두벌식 순아래/국규9256 자판 (`TYPE_GALMADEULI=1001`) | **3beol — yous/libhangul (branch `gureum-1.11.1`, HEAD `bb8fcb3`)** | 엔진: `nabicloud/engine/engine-galmadeuli.c`(`nabicloud_engine_galmadeuli_process`) + `.h` 선언, `nabicloud/engine/dispatch.c`(GALMADEULI case 1줄). 데이터: `nabicloud/layouts/layout-2noshift.c`(replace 표·default_2 조합·9256 키맵) + `.h`. 등록: `hangul/hangulkeyboard.c`(`2noshift`·`2n9256` 빌트인) | **LGPL-2.1-or-later (yous/libhangul lineage)** — 코어와 동일 계열·GPL 경계 불요(상류 COPYING=LGPL v2.1; 318na 만 GPL-3.0) | **S4 완료(2026-06-16)**: 3beol `hangul_ic_process_jamo_dubeol`(hangulinputcontext.c 1882~2053)을 충실 이식. 적응 2곳만: (1) 된소리 치환은 우리 `hangul_ic_combine`(combination[0] 전용)을 안 건드리고 `hangul_keyboard_combine(kbd,2,0,choseong)` 직접 호출로 replace 표(combination[2]) 조회, (2) LOOSE_ORDER flag 인프라 부재→`option_auto_reorder` 단독. replace 표·default_2 조합·9256 키맵은 3beol 원본과 byte 동일 검증. 차분검증: `tests/build_and_verify_galmadeuli.bat`(우리 출력 == 3beol 레퍼런스, 2noshift·2n9256 둘 다 `GALMADEULI_PASS`). 우리 `default` 조합표와 3beol `default_2` 가 다름을 실측 발견→`default_2` 별도 전사(추정 재사용 회피) |
| P10 | 신세벌(jaso-shin) 조합 엔진 — **[동작·LIVE]** (`TYPE_JASO_SHIN=1002`/`_SHIFT=1003`/`_YET=1004`) | **3beol — yous/libhangul (branch `gureum-1.11.1`, HEAD `bb8fcb3`)** | 엔진: `nabicloud/engine/engine-jasoshin.c`(`nabicloud_engine_shin_process`/`_shift_process`/`_yet_process`) + `.h`. dispatch: baram(산들바람) 라우팅(`baram.c`(옛 engine-ops.c) `kBaram_shin`/`_shift`/`_yet` → `dispatch.c` `nabicloud_baram_lookup`). type: `hangul/hangul.h`(1002~1004 정의). 데이터: `nabicloud/layouts/layout-3shin.h`(9종 자판). 등록: `hangul/hangulkeyboard.c` 빌트인 9종 + 골든 포함(golden_all idx 13~21) | **LGPL-2.1-or-later (yous/libhangul lineage)** — 코어와 동일 계열·GPL 경계 불요(상류 COPYING=LGPL v2.1) | **S5 [동작]**: 실측(3beol hangulinputcontext.c `hangul_ic_process_jaso_shin_sebeol` 2057~2405 / `_shift` 3398~3591 / `_yet` 3117~3396)을 충실 이식. 확장레이아웃 인프라(`buffer.right_oua`·IC 옵션 `option_extended_layout_*`·HangulKeyboard `flag[]`/`addon_*` 컬럼+접근자·ctype `hangul_ascii_to_symbol_shin`·_yet capslock 채널) 이식 완료. baram(산들바람) 라우팅으로 LIVE. 차분검증: `build_and_verify_jasoshin.bat`→`JASOSHIN_PASS` |
| P9 | 3finalsun(세벌식 3-91 Final 순아래) 조합 엔진 + 3-91-noshift 자판 (`TYPE_FINALSUN=1010`) | **3beol — yous/libhangul (branch `gureum-1.11.1`, HEAD `bb8fcb3`)** | 엔진: `nabicloud/engine/engine-finalsun.c`(`nabicloud_engine_finalsun_process`) + `.h` 선언, `nabicloud/engine/dispatch.c`(FINALSUN case 1줄, `ascii` 전달). 데이터: `nabicloud/layouts/layout-3finalsun.c`(3-91-noshift 키맵 128 + 조합표 243 active) + `.h`. 인프라: `hangulinternals.h` `_HangulBuffer` 에 trailing `ucschar shift` 1필드 추가 + `hangulinputcontext.c hangul_buffer_clear` 에 `shift=0`. 등록: `hangul/hangulkeyboard.c`(`3-91-noshift` 빌트인) | **LGPL-2.1-or-later (yous/libhangul lineage)** — 코어와 동일 계열·GPL 경계 불요(상류 COPYING=LGPL v2.1; 318na 만 GPL-3.0) | **S5 완료(2026-06-16)**: 3beol `hangul_ic_process_3finalsun`(hangulinputcontext.c 2407~2630)을 statement-for-statement 충실 이식. 적응 2곳만: (1) LOOSE_ORDER flag 인프라 부재→`option_auto_reorder` 단독(3-91-noshift LOOSE_ORDER=false라 등가), (2) upstream-private `FALSE` 매크로 대신 `!hangul_buffer_is_empty(...)`. 종성시프트 글쇠 `[`→SHKEY 센티넬(0x11ff)로 치환→combination[0]의 0x11ff* 쌍으로 겹받침 형성/회전. 표준 jamo 버퍼+combination[0]만 사용(갈마들이표 combination[2] 불요, `hangul_ic_combine` 본체 무수정). 키맵·조합표는 3beol 원본과 byte 동일 전사(정렬 1쌍 의도적 역순 0x11751171→0x1175116c·주석처리 3행 포함 그대로 — bsearch 동작 동일). 신규 의존은 `buffer.shift` 단일 필드뿐(trailing·C zero-fill→3gs/2/318na/갈마들이 골든 byte 불변). 차분검증: `tests/build_and_verify_finalsun.bat`(우리 출력 == 3beol 레퍼런스 `golden_finalsun.txt`, `FINALSUN_PASS`) |

## 미적용(재구조화 중 함께 고정 예정)

- **K1~K4 — 외부 자판 로더 메모리 안전 결함**(미수정): `hangulkeyboard.c` 의
  type NULL·item current_element NULL·include 경로순회·id `strdup(NULL)`.
  → **Step6** 에서 `win32/keyboard-loader.c` 로 옮기며 NULL-safe 헬퍼
  (`streq_safe`·`attr_or`·`path_is_safe`)로 일관 적용.
- **318na 자판**(미편입): GPL-3.0 경계. Step7 에서 `nabicloud/layouts/318na` 로
  데이터+정책 추가 시 라이선스 경계를 NOTICE 에 명시.

## 검증

모든 차이는 `tests/` 골든 게이트(`build_and_verify.bat` → `GOLDEN_PASS`)로 동작
불변을 보증한다. 자세한 기준점은 `UPSTREAM.md` 참조.

신규 패밀리(P8 갈마들이, P9 3finalsun)는 각자 별도 차분 게이트로 봉인한다:
- P8: `build_and_verify_galmadeuli.bat` → `GALMADEULI_PASS`. 베이스라인
  `tests/golden_galmadeuli.txt`·`golden_galmadeuli_2n9256.txt`(하니스 `golden_galmadeuli.c`).
- P9: `build_and_verify_finalsun.bat` → `FINALSUN_PASS`. 베이스라인
  `tests/golden_finalsun.txt`(하니스 `golden_finalsun.c`, 3-91-noshift).

베이스라인은 동일 하니스를 3beol 클론에 대해 빌드·실행해 동결한 *레퍼런스*다.
3beol 레퍼런스 빌드는 MSVC 적응이 필요했다(`strings.h` 셰임=`_stricmp`,
`/DLIBHANGUL_DATA_DIR`, `/DINIT_IDS_LENGTH=10`). 우리 트리 빌드는 적응 불필요.
