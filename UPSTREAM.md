# UPSTREAM.md — 상류(libhangul) 기준점

이 디렉토리는 **libhangul 포크**이며, 향후 `nabicloud/libhangul` 로 재정체화된다.
모든 재구조화·이전 작업의 *동작 동등성*은 아래 상류 기준점 대비 검증한다.

## 기준 상류

- **프로젝트**: libhangul — https://github.com/libhangul/libhangul
- **라이선스**: LGPL-2.1-or-later (루트 `COPYING`)
- **원작자**: Choe Hwanjin, Joon-cheol Park (`AUTHORS`)
- **베이스 릴리스 라인**: libhangul **0.2.0**
  - 조합 엔진 일부는 gureum/libhangul (libhangul **0.1.0** 포크) 계보에서 유래.
  - 우리 트리는 트리밍된 포크라 `.git`/`configure.ac`/`NEWS` 를 보존하지 않는다(`hangul/` 소스만).
    그래서 **상류 기준점을 본 문서에 고정한다**(서브모듈 핀과 동일한 역할):
    - **태그**: `libhangul-0.2.0`
    - **커밋**: `41c702f5d3581325b646ef6249f1f641b0427ae0`
      (annotated 태그 객체는 `20afc38922e3595ee3ed5b186f2ea05afe663763`, 역참조 `^{}` 커밋이 위 값. 2026-06-19 `git ls-remote` 확인.)
    - 조회: `git ls-remote --tags https://github.com/libhangul/libhangul` → `refs/tags/libhangul-0.2.0^{}`

## Vendored 의존 — Expat (libexpat) (2026-07-01)

libhangul 의 `ENABLE_EXTERNAL_KEYBOARDS` 외부 XML 자판 로더(`hangul/hangulkeyboard.c`, 순정 expat 경로)를 켜면서
상류 **libexpat** 를 `expat/` 에 vendored. libhangul 상류(위)와 **별개 프로젝트**다.
- **프로젝트**: libexpat — https://github.com/libexpat/libexpat
- **릴리스**: `R_2_6_4` · **라이선스**: MIT (`expat/COPYING`·`expat/AUTHORS` — 귀속 고지만 의무)
- **내용**: `expat/lib/` = 상류 `expat/lib/*` verbatim + 손작성 `expat/lib/expat_config.h`(Windows/MSVC). 상세 `expat/README.md`.
- **빌드**: `libhangul.vcxproj` 가 `XML_STATIC` + `ENABLE_EXTERNAL_KEYBOARDS` 로 정적 컴파일(별 DLL 아님).
- 갱신법: 상류 태그의 `expat/lib/*.{c,h}` + `COPYING`/`AUTHORS` 재복사, `expat/lib/expat_config.h` 유지.

## 재배이스(ingest) 상태 — 트랙 T1 / §42 ② (2026-06-26)

순정 상류 추종(de-fork) 트랙. 정본 계획 문서는 본체(private) 레포에서 관리.

- **최신 상류 = `libhangul-0.2.0` = 현 베이스 핀.** master HEAD 는 +5 커밋(cmake/po만, 엔진 변화 0)
  → "최신 ingest" = **버전 범프 아님, 순정 0.2.0 으로의 de-fork**(V2 대체 포크 패치 제거).
- **★레거시 7종(2y·3y·3s·32·ahn 빌트인 + 3-89·3sun-1990 외부 XML)은 전부 순정 타입·테이블이라
  포크 엔진 의존 0** — 순정에서 그대로 동작(전수 평가 `wf_56db3a82-3bc` 확인). 드롭 불필요.
- **★ingest 코어 제거는 셸 디엔탱글(windows/tsf, 내 레인 밖)에 게이트.** 적대검증 28/28 UNSAFE:
  `.def` 46 export·포크 엔진·디스패치가 전부 NabiCloud.dll(LibhangulBackend 등)에 라이브 링크.
  V2 경로는 engine-jaso-core/v2backend 를 직접 컴파일 → libhangul export 0 import. 순서 의존 상세 = 본체(private) 레포 INGEST_PLAN §1.

## 상류 대비 차이를 보는 법

이 포크는 upstream 과 byte-identical 이 **아니다**. 차이는 의도적이며 전부
`PATCHES.md` 에 기능별로 기록한다. 재구조화의 설계 원칙:

- `hangul/` 의 upstream 파일은 **얇은 dispatch hook 만** 남기고 95%+ 무수정 유지.
- NabiCloud 고유 로직(gureum 순아래 엔진·PUA vkeycode·win32 자판 로더·hanja
  하드닝)은 `nabicloud/` 계층으로 분리.
- 차이가 0 이 될 수 없는 지점(예: enum 값, mode 인자 시그니처)은 **검토 가능한
  영구 어댑터**로 고정하고 그 사유를 `PATCHES.md` 에 명시.

## 비공식 자판 type 값 — 1000+ 대역 예약 정책

`HangulKeyboardType` enum(`hangul/hangul.h`)은 upstream 이 0~4(JAMO/JASO/ROMAJA/
JAMO_YET/JASO_YET)를 쓴다. NabiCloud 의 비공식(상류 미존재) 자판 패밀리 type 은
**값 1000 이상**으로만 예약한다. enum 값 충돌은 컴파일타임에 검출되지 않으므로
(단순 정수 비교 디스패치) **사람이 본 표로 관리**한다. 새 type 추가 시 이 표와
`docs/LAYOUT_ARCHITECTURE.md` §5 를 함께 갱신한다.

| 상수 | 값 | 패밀리 | 상태 |
|------|----|--------|------|
| `HANGUL_KEYBOARD_TYPE_NABICLOUD` | 99 | sunarae(순아래) | 현행(역사적 값, alias `_GUREUM`) |
| `HANGUL_KEYBOARD_TYPE_GALMADEULI` | 1001 | 갈마들이 | **구현됨(S4)** — 2noshift·2n9256 |
| `HANGUL_KEYBOARD_TYPE_JASO_SHIN` | 1002~1004 | 신세벌(변종군) | ★**LIVE(구현됨)** — 1002 SHIN/1003 SHIFT/1004 YET 등록·라우팅(`#if 0` 아님), 확장레이아웃 인프라 전부 존재, 신세벌 9종 외부 XML(3shin-*.xml)로 동작. (옛 "스켈레톤·미동작" 무효.) 경계 상태 정본 = PATCHES.md P10 |
| `HANGUL_KEYBOARD_TYPE_SEBEOL_CHORD` | 1006 | sebeol-chord(세벌 모아치기) | **구현됨** — 세모이(참고)·안마태. plain jaso + sebeol compose. 정본명(2026-06-21 개명, 옛 `_SEMOE_CHORD`는 hangul.h 별칭으로 유지). 정본 DECISIONS §29 |
| `HANGUL_KEYBOARD_TYPE_DUBEOL_CHORD` | 1007 | dubeol-chord(두벌 모아치기) | **구현됨** — 두겹이(V-start). DECISIONS §29 / DUBEOL_CHORD_DESIGN |
| `HANGUL_KEYBOARD_TYPE_FINALSUN` | 1010 | 3finalsun | **구현됨(S5)** — 3-91-noshift |
| `HANGUL_KEYBOARD_TYPE_ADDON` | 1020 | addon | 예약(보류) |

- **99 는 예외**(이미 배포·골든 시드라 유지). 신규는 전부 1000+.
- 구획: 1000~1099 = 모아치기 패밀리, 1020 대 = 부가/실험 훅.
- upstream 이 향후 0~4 너머로 enum 을 확장해도 1000+ 와 절대 충돌하지 않도록
  넉넉한 간격을 둔다. 상류 enum 확장을 흡수할 때 본 정책을 재검토한다.

## 동작 동등성 검증 — 골든 게이트

상류·포크·재구조화 단계 간 동작 동등성은 `tests/` 의 특성화(characterization)
테스트로 보증한다.

```
tests\build_and_verify.bat   REM GOLDEN_PASS 면 동작 불변, GOLDEN_FAIL 면 회귀
```

- `tests/golden.c` — 3gs 엔진을 광범위하게 구동(전 키 단건·전 키쌍 스윕 +
  큐레이션 음절 + backspace/reset/flush 시나리오)하고 commit/preedit 를 hex 로 덤프.
- `tests/golden.txt` — **현재 코드 기준 동결 베이스라인**. 손으로 편집 금지.
- 재구조화 각 단계는 이 게이트가 PASS 를 유지해야 한다(동작 보존 단계의 합격선).
