# libhangul-nabicloud (조합 엔진)

이 라이브러리의 정식 명칭은 **libhangul-nabicloud**다. 상류 **libhangul 0.2.0**
(태그 `libhangul-0.2.0`, 커밋 핀 = `UPSTREAM.md`)을 베이스로 한 포크로,
**순정 재배이스(2026-06-30, F-3)** 이후의 구성은 *순정 0.2.0 코어 + 필요분만
additive 재이식*(Windows 적응·버그픽스 — 기능별 provenance = `PATCHES.md`)이다.

폴더명은 `engine/`이지만 프로젝트·라이선스상의 명칭은 **libhangul-nabicloud**다.
(모노레포에서 윈도우·macOS·legacy가 공유하는 조합 엔진이라 `shared/engine`에 둔다.)

## 라이선스

- **LGPL-2.1-or-later** (전문: `COPYING`). 구성·저작권 상세는 `NOTICE` 참조.
  (역사적 gureum 유래 성분의 BSD 고지 = `GUREUM-COPYING` 보존 — 현 코어는 순정
  0.2.0 재배이스로, 구름 3gs 순아래 조합은 이 라이브러리 밖(본체 V2 엔진)으로 이전됐다.)
- expat (외부 XML 로더용) = MIT — 고지는 `NOTICE` §7.

## 상류 대비 차이

- 기능별 차이(provenance): 이 디렉토리의 `PATCHES.md` — LGPL §2(b) 변경 사실·날짜 명시.
- 기준 상류 커밋·버전 핀: `UPSTREAM.md` (fresh snapshot 발행에서도 이 핀으로 diff 재현 가능).

## 상류 업데이트 반영 방법

상류 libhangul 을 통째로 덮지 말고, `UPSTREAM.md` 기준점에서 상류 diff 를 검토한 뒤
`PATCHES.md` 의 재이식 목록(P#)이 유지되는지 확인하며 반영한다(additive 재이식 원칙).

> 참고: 이 폴더에는 상류 libhangul의 원본 `README`(빌드 안내)도 함께 보존되어 있다.
> 본 `README.md`가 NabiCloud 포크로서의 정체성·명칭·라이선스를 설명하는 정본이다.

## 공개 standalone 저장소 안내 (nabicloud-engine)

이 트리는 나비구름 모노레포의 `shared/engine/` 을 **fresh snapshot** 으로 발행한
공개 저장소(`nabicloud-engine`)로도 쓰인다. standalone 클론 시:

- **테스트 게이트**: `tests/run_all_gates.bat` 의 게이트는 **CORE / INTEGRATION** 2부류다.
  CORE(`build_and_verify_all` — 순정 빌트인 byte-골든)는 이 트리 단독으로 돈다.
  INTEGRATION(나머지 — 본체 모노레포의 `cleanroom/`·`windows/`·`shared/data/keyboards`
  픽스처를 검증)은 픽스처 부재 시 `[SKIP]`(exit 77)으로 안전 통과한다 — 실패가 아니다.
  전량 실행은 본체 superproject(전 서브모듈 마운트)에서 한다.
- **expat** (외부 XML 로더 `ENABLE_EXTERNAL_KEYBOARDS`): vendored `expat/`(모노레포 시절)
  또는 상류 서브모듈 `expat-upstream/`(공개 저장소 — libexpat **R_2_6_4** 핀) 구성.
  서브모듈 구성에선 `git clone --recurse-submodules` 필요. 자작 MSVC 설정 헤더 =
  `win32/expat_config.h`.
- **본체와의 관계**: 나비구름 본체(private)가 이 저장소를 `shared/engine` 서브모듈로
  마운트하고 `libhangul.vcxproj` 를 ProjectReference 로 빌드한다(LGPL-2.1 §6(b)
  동적링크, ABI 경계 = `libhangul.def`). 본체 없이도 이 저장소 단독으로
  `libhangul.dll` 빌드·CORE 게이트 실행이 가능하다.
