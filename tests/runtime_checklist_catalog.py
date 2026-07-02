#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# runtime_checklist_catalog.py — 런타임 테스트 체크리스트 (반)자동화 **1단계**.
#
# 정본 설계: docs/TEST_CHECKLIST_AUTOMATION_DESIGN.md (CODE 0, Codex 조건부 GO + 보강 3 + A-D).
#   이 파일은 그 §7 「1단계 (즉시가치·정적·빌드 0)」의 구현이다:
#     - layout-메타: 17 jaso_layout 인스턴스 universe enumerate(absence-is-false 맹점 방어)
#       + 양성 `.field=` 기제 수집.
#     - id-registry: 5-소스 UNION(kJasoBuiltin·libhangul순정9·임베드XML·외부XML·표시명)
#       + known-dropped 상수(DECISIONS §32/§39/§46 출처).
#     - 셀렉터: 변경셋 → 영향 자판(표시명·id)·기제·출하상태 + 추천 라운드/앵커.
#   ★expected(입력→기대 한글)는 아직 *자동 생성하지 않는다*. RUNTIME_TEST_CHECKLIST.md 의
#     *검증된 행을 사람이 인용*한다(생성=날조 = 설계 §3 함정, 2단계 instrument-and-run 전까지 금지).
#
# 이 도구는 **정적 스캔만**(빌드 0·게이트 무접촉·순수 additive). run_all_gates 를 건드리지 않는다.
#
# 사용:
#   python runtime_checklist_catalog.py catalog      # 전체 카탈로그(레이아웃×기제 매트릭스 + id-registry) 덤프
#   python runtime_checklist_catalog.py select FILE.. # 변경 파일 → 영향 자판/기제/라운드 추천 (없으면 git diff)
#   python runtime_checklist_catalog.py census        # 보강2: canonical selftest ∩ 빌드 manifest 감사
#   python runtime_checklist_catalog.py check         # 보강1: known-dropped 재등장 fail + 로스터 sync(selftest_roster.h↔.bat)
#
# 표기 규약(AGENTS §2.5): 입력=QWERTY 그대로 / 기대=한글 / 자판=표시명(id). 진행 보고=한국어.
#
# exit: 0=정상 / 1=드리프트·감사 실패(check/census) / 2=소스 파싱 실패(구조 변경 감지).
import sys, os, re, glob, subprocess, json

# --------------------------------------------------------------------------
# 경로 (repo 루트 = 이 스크립트의 shared/engine/tests/ 로부터 3단계 위)
# --------------------------------------------------------------------------
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", "..", ".."))

CANON     = os.path.join(ROOT, "cleanroom", "impl", "clean", "canonical")
LAYOUT_H  = os.path.join(CANON, "jaso_layout.h")
V2BACKEND = os.path.join(ROOT, "cleanroom", "impl", "promote-shinp2", "v2backend.cpp")
NABI_CPP  = os.path.join(ROOT, "windows", "tsf", "NabiCloud.cpp")
KB_NAMES  = os.path.join(ROOT, "shared", "data", "keyboards", "keyboard-names.txt")
KB_XML    = os.path.join(ROOT, "shared", "data", "keyboards")
BUILD_BAT = os.path.join(HERE, "build_cleanroom_selftests.bat")
BUILD_EMIT = os.path.join(HERE, "build_one_emit.cmd")
ROSTER_H  = os.path.join(HERE, "selftest_roster.h")   # selftest 로스터 X-macro 단일 정본
NSI_BASE  = os.path.join(ROOT, "windows", "installer", "NabiCloud-v2.nsi")
RUNTIME_MD = os.path.join(ROOT, "docs", "RUNTIME_TEST_CHECKLIST.md")
CORE_SRC  = "engine-jaso-core.c"   # build manifest 의 %CORE% 확장

def rel(p):
    return os.path.relpath(p, ROOT).replace("\\", "/")

# --------------------------------------------------------------------------
# 큐레이션 상수 (id 공간서 기계발견 *불가* — DECISIONS 출처 명시. 보강1/보강3.)
#   ★이 상수들은 라이브 소스에서 추론 불가한 *정책* 이라 수기다. 각 항목 = 출처 §.
#   check 커맨드가 known-dropped 를 라이브 레지스트리와 sync-검사(hard-fail)한다.
# --------------------------------------------------------------------------

# libhangul 순정 0.2.0 빌트인 9 (NabiCloud.cpp s_keyboards · golden_pristine9_anchor.py 와 동일 집합).
PRISTINE9 = {"2", "2y", "39", "3f", "3s", "3y", "32", "ro", "ahn"}

# 본체 기본 출하 외부 XML (DECISIONS §46, f0bcd9cf) — NSI 파싱 실패 시 폴백 상수.
#   ★정상 경로는 NabiCloud-v2.nsi 의 `File …\keyboards\X.xml` 라이브 파싱(parse_base_shipped_xml).
#     자판이 본체로 승격되면 NSI File 한 줄 추가 → ships 자동 반영(수기 상수 갱신 불요).
BASE_EXTERNAL_XML_FALLBACK = {"3-314", "3-314-banja", "3-ahn-v2"}

# known-dropped: 출하 안 함·보류 확정. id 공간(소스)엔 안 남아 기계발견 불가 → 수기 상수.
#   3-91-noshift = finalsun 보류(DECISIONS §32/§39) · 3-14-proposal = V2 미지원 중복판(§39/§42)
#   · 3-2015-metal = .ist 후순위·미승급(§39). check 가 라이브 레지스트리에 재등장 시 fail.
KNOWN_DROPPED = {
    "3-91-noshift": "DECISIONS §32/§39 (finalsun 보류·출하 안 함)",
    "3-14-proposal": "DECISIONS §39/§42 (V2-replaced 중복판·숨김)",
    "3-2015-metal": "DECISIONS §39 (.ist 후순위·미승급)",
}

# 메뉴 숨김(존재하나 노출 안 함) — DECISIONS §42/§46. 참고용(에러 아님).
HIDDEN = {
    "3-ahn": "§42 V2-replaced(3-ahn-v2 로 대체)",
    "3shin-1995": "§42 근사판(XML 존재·애드온 미배포)",
    "3shin-2003": "§42 근사판(XML 존재·애드온 미배포)",
}

# 빌트인(kJasoBuiltin) 밖이나 *생성 외부 XML* 로 실제 출하되는 레이아웃-백킹 자판.
#   layout → (id, ships, 출처). _gen_*.c 생성기가 C 정본 레이아웃에서 XML 을 뽑는다.
#   ★GONGDONG/YETDEMO 는 여기 없음 — 공동=어떤 출하/애드온에도 미등록(RUNTIME L-5·설계 §8),
#     yetdemo=테스트 데모. 둘은 정직하게 "미등록"으로 남긴다.
GENERATED_XML_LAYOUTS = {
    "JASO_LAYOUT_318NA": ("318na", "애드온", "_gen_318na_xml.c → addons/318na/keyboards/318na.xml"),
}

# --------------------------------------------------------------------------
# 기제(mechanism) universe — jaso_layout 구조체의 기제-담지 필드.
#   ★universe enumerate = absence-is-false 맹점 방어: 레이아웃이 필드를 *안 쓰면*
#     designated-init zero default 로 기제 OFF 인데, 그게 *주석 없는 빈 칸* 으로 보여야 한다.
#   각 항목 = (필드명, 표시 기제명, jaso_strat 관련 층/phase, RUNTIME 라운드 힌트).
#   phase = 셀렉터 보강3(공유 코드 변경 시 승격 대상) 판별에 쓴다. 정본 = jaso_layout.h.
# --------------------------------------------------------------------------
MECHANISM_FIELDS = [
    # field,            display,                 phase,          round hint
    ("role_flip",       "role_flip(타입 본질)",   "classify/L1",  "C 대표 + 타입 교차"),
    ("has_right_oua",   "right-oua(문맥중성)",    "right-oua/L2", "L-2 (가상경유+직입력 양경로)"),
    ("virtual_units",   "가상중성 단위표",         "right-oua/L2", "L-2 (backspace 복원 포함)"),
    ("onsetless_virtual","onset없는 가상중성",     "right-oua/L2", "L-2 (빈음절 왼쪽 ㅗㅜ)"),
    ("extended",        "EXTENDED(부호열)",       "L3 lifetime",  "L-1 (무장→트리거→치환 + backspace)"),
    ("shkey_trigger",   "SHKEY(순아래 자동받침)", "L3 lifetime",  "Q: 받침有/받침無 짝"),
    ("moa",             "공동 moa(이어치기)",     "MOA/L2",       "L-5 (미배선=표면없음 주의)"),
    ("rules",           "조건부-규칙(GD_RULES)",  "SLOT_FILL/CONDITIONAL/L2", "L-3 fire/non-fire 짝 (미배선 주의)"),
    ("shift_require_onset","SHIN_SHIFT M(onset요구)","classify dual","L-2/L-3 (2015↔M 델타)"),
    ("dual_onsetless_combine","dual onsetless-combine(참신D 좌수낱자)","classify dual","L-2/L-3 (X-술어 fire/non-fire 짝 + 빈음절 중성)"),
    ("yet",             "옛한글(첫가끝 출력)",     "render",       "yet: conjoining + NFC 짝, preedit 확인"),
    ("fill_jong",       "fill-jong(두겹이 단키겹받침)","query",    "Q: 이중모음 우선 vs 겹받침 짝 (미배선 주의)"),
]
MECH_BY_FIELD = {m[0]: m for m in MECHANISM_FIELDS}

# 기제 아닌 구조적 필드(키맵·결합표·카운트 등) — 그 자체론 테스트 축 아님.
#   MECHANISM_FIELDS ∪ STRUCTURAL_FIELDS 밖의 구조체 멤버가 생기면 = 미분류 신규 필드 →
#   drift 경고(새 기제일 수 있음 → 분류 강제). "조용히 누락" 방지(설계 §6 M3b 정신).
STRUCTURAL_FIELDS = {
    "keymap", "cho", "n_cho", "jung", "n_jung", "jong", "n_jong",
    "ext_onset", "ext_triggers", "n_ext_triggers", "symbols", "n_symbols",
    "n_virtual_units", "n_moa", "shkey", "n_shkey", "n_rules", "n_fill_jong",
    # keymap 값 도메인 표지(2026-07-02 리뷰 (b)안): 기제 아님 — 동작 토글 없는 *계약 선언*
    #   (compat U+31xx / conjoining U+11xx). 표지↔실측 일치는 selftest_combine_integrity
    #   (c)와 로더(mixed=decline)·stdxml/ahn_chord 앵커가 잠근다. 정본 = jaso_layout.h.
    "keymap_domain",
}

def mechanism_drift():
    """구조체 멤버 ⊖ (MECHANISM ∪ STRUCTURAL) → 미분류 신규 필드 목록 + 사라진 기제 필드 목록.
       비어야 정상. 새 기제 도입이 도구에 조용히 흡수되지 않고 *분류를 강제*하게 만든다."""
    members = parse_struct_field_universe()
    known = set(MECH_BY_FIELD) | STRUCTURAL_FIELDS
    unclassified = sorted(members - known)                    # 구조체엔 있는데 도구 미분류
    vanished = sorted(f for f in MECH_BY_FIELD if f not in members)  # 도구엔 있는데 구조체서 사라짐
    return unclassified, vanished

# --------------------------------------------------------------------------
# 파싱 — 전부 라이브 소스에서 도출(드리프트 없음). 구조 변경 감지 시 exit 2.
# --------------------------------------------------------------------------

def _read(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()

def parse_struct_field_universe():
    """jaso_layout.h 의 jaso_layout 구조체 멤버명 집합 (M3b 드리프트 방어 + universe 확인).
       MECHANISM_FIELDS 의 필드가 구조체에 실재하는지 교차확인한다(설계 표류 hard-signal)."""
    txt = _read(LAYOUT_H)
    end = re.search(r"\}\s*jaso_layout\s*;", txt)
    if not end:
        die("jaso_layout 구조체 정의를 못 찾음 — 헤더 구조 변경? %s" % rel(LAYOUT_H))
    # jaso_layout 은 헤더 마지막 struct → `} jaso_layout;` 직전의 *마지막* `typedef struct {` 가 시작.
    starts = list(re.finditer(r"typedef\s+struct\s*\{", txt[:end.start()]))
    if not starts:
        die("jaso_layout struct 여는 브레이스를 못 찾음 — %s" % rel(LAYOUT_H))
    body = txt[starts[-1].end():end.start()]
    # 멤버명: `... name;` 또는 `... name[128];` 의 마지막 식별자.
    members = set(re.findall(r"\b([a-z_][a-z0-9_]*)\s*(?:\[[^\]]*\])?\s*;", body))
    return members

def parse_layouts():
    """canonical/jaso_layout_*.c → [{name, file, mechanisms{field:val}}].
       한 파일이 여러 인스턴스를 가질 수 있다(galmadeuli = 2). 인스턴스 시작 위치로 영역을 가른다."""
    layouts = []
    files = sorted(glob.glob(os.path.join(CANON, "jaso_layout_*.c")))
    inst_re = re.compile(r"const\s+jaso_layout\s+(JASO_LAYOUT_\w+)\s*=\s*\{")
    for path in files:
        txt = _read(path)
        starts = [(m.start(), m.group(1)) for m in inst_re.finditer(txt)]
        for i, (pos, name) in enumerate(starts):
            end = starts[i + 1][0] if i + 1 < len(starts) else len(txt)
            region = txt[pos:end]
            mech = {}
            # role_flip(타입 본질) — 없으면 NONE(zero default).
            rm = re.search(r"\.role_flip\s*=\s*JASO_ROLE_(\w+)", region)
            mech["role_flip"] = rm.group(1) if rm else "NONE"
            # bool 기제.
            for fld in ("has_right_oua", "extended", "yet", "shift_require_onset", "onsetless_virtual"):
                mech[fld] = bool(re.search(r"\.%s\s*=\s*true" % fld, region))
            # 포인터/배열 기제(설정=ON).
            for fld in ("virtual_units", "moa", "rules", "fill_jong"):
                mech[fld] = bool(re.search(r"\.%s\s*=" % fld, region))
            # shkey_trigger: 비-0 이면 ON.
            sm = re.search(r"\.shkey_trigger\s*=\s*(0x[0-9a-fA-F]+|\d+)", region)
            mech["shkey_trigger"] = bool(sm and int(sm.group(1), 0) != 0)
            layouts.append({"name": name, "file": rel(path), "mech": mech})
    if len(layouts) != 17:
        warn("레이아웃 인스턴스 %d 개 (설계 기대 17). designated-init 변경?" % len(layouts))
    return layouts

def parse_kjaso_builtin():
    """v2backend.cpp kJasoBuiltin[] → {id: JASO_LAYOUT_*}. 빌트인 jaso id→layout 단일 정본."""
    txt = _read(V2BACKEND)
    m = re.search(r"kJasoBuiltin\[\]\s*=\s*\{(.*?)\};", txt, re.S)
    if not m:
        die("kJasoBuiltin 표를 못 찾음 — v2backend 구조 변경? %s" % rel(V2BACKEND))
    out = {}
    for id_, layout in re.findall(r'\{\s*"([^"]+)"\s*,\s*&(JASO_LAYOUT_\w+)', m.group(1)):
        out[id_] = layout
    return out

def parse_keyboard_names():
    """keyboard-names.txt → {id: 표시명}. '#'/';'/빈 줄 무시, `id = name`."""
    out = {}
    for line in _read(KB_NAMES).splitlines():
        s = line.strip()
        if not s or s[0] in "#;":
            continue
        if "=" in s:
            k, v = s.split("=", 1)
            out[k.strip()] = v.strip()
    return out

def parse_external_xml():
    """shared/data/keyboards/*.xml → {id: {name, type, file}}. 루트 = <hangul-keyboard id= type=>.
       name = 첫 <name>…</name> 엘리먼트 내용(한국어 primary). combination 데이터(id 없음) 제외."""
    out = {}
    for path in sorted(glob.glob(os.path.join(KB_XML, "*.xml"))):
        txt = _read(path)
        mroot = re.search(r"<hangul-keyboard\b([^>]*)>", txt)
        if not mroot:
            continue  # combination-only 등 (id 없음)
        attrs = mroot.group(1)
        mid = re.search(r"\bid\s*=\s*\"([^\"]+)\"", attrs)
        if not mid:
            continue
        kid = mid.group(1)
        mty = re.search(r"\btype\s*=\s*\"([^\"]+)\"", attrs)
        mnm = re.search(r"<name(?:\s+[^>]*)?>([^<]+)</name>", txt)  # 첫 <name> = 한국어 primary
        out[kid] = {"name": mnm.group(1).strip() if mnm else "",
                    "type": mty.group(1) if mty else "", "file": rel(path)}
    return out

def parse_embed_xml_ids():
    """nabicloud_builtin_*_xml.h → 임베드(컴파일인) V2 자판 id 집합."""
    ids = set()
    for path in glob.glob(os.path.join(CANON, "nabicloud_builtin_*_xml.h")):
        txt = _read(path)
        m = re.search(r"\bid\s*=\s*(?:\\?\")([^\"\\]+)", txt)
        if m:
            ids.add(m.group(1))
    return ids

def parse_base_shipped_xml():
    """NabiCloud-v2.nsi 의 `File "..\\keyboards\\X.xml"` → 본체 출하 외부 XML id 집합(라이브).
       자판이 본체 승격되면 여기 한 줄 추가로 ships 가 자동 반영된다(수기 상수 불요)."""
    try:
        txt = _read(NSI_BASE)
    except OSError:
        return set(BASE_EXTERNAL_XML_FALLBACK)
    ids = set()
    for m in re.finditer(r'File\s+"[^"]*[\\/]keyboards[\\/]([^"\\/]+)\.xml"', txt):
        ids.add(m.group(1))
    return ids or set(BASE_EXTERNAL_XML_FALLBACK)

def parse_shell_meta():
    """NabiCloud.cpp s_keyboards[] → {id: {name, bucket, advanced}}. 순정 9 의 메뉴 표시명 정본.
       de-fork 예외 자판(3gs 등)은 별도 경로(step②)라 이 배열엔 없음(순정 9 만)."""
    txt = _read(NABI_CPP)
    m = re.search(r"s_keyboards\[\]\s*=\s*\{(.*?)\};", txt, re.S)
    if not m:
        warn("s_keyboards 배열을 못 찾음 — NabiCloud.cpp 구조 변경? (순정9 표시명 생략)")
        return {}
    out = {}
    row = re.compile(r'\{\s*"([^"]+)"\s*,\s*L"([^"]+)"\s*,\s*L"[^"]*"\s*,'
                     r'\s*CNabiCloud::(Bucket_\w+)\s*,\s*(true|false)\s*\}')
    for kid, name, bucket, adv in row.findall(m.group(1)):
        out[kid] = {"name": name, "bucket": bucket, "advanced": adv == "true"}
    return out

def parse_build_manifest():
    """build_cleanroom_selftests.bat 의 `call :build NAME src...` → {gate: [srcs]}.
       게이트 manifest = census(보강2)·layout→selftest 역인덱스의 정본."""
    out = {}
    for line in _read(BUILD_BAT).splitlines():
        m = re.match(r"\s*call :build\s+(\S+)\s+(.*)$", line)
        if m:
            out[m.group(1)] = m.group(2).split()
    return out

def parse_selftest_roster():
    """selftest_roster.h 의 `SELFTEST(stem, "kbid", "desc")` X-macro 행 → [(stem, kbid, desc)].
       로스터 단일 정본(2026-07-02 수렴): check 가 .bat manifest 와 양방향 대조(sync 게이트),
       verify 의 FILE_TO_ID(자판귀속)가 kbid 열에서 파생 — .py 수기 dict 사본 제거."""
    if not os.path.exists(ROSTER_H):
        die("selftest_roster.h 부재 — 로스터 정본 없음 (%s)" % rel(ROSTER_H))
    rows = re.findall(r'^\s*SELFTEST\(\s*(\w+)\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"\s*\)',
                      _read(ROSTER_H), re.M)
    if not rows:
        die("selftest_roster.h 파싱 실패(SELFTEST 행 0) — 행 형식/구조 변경?")
    return rows

def roster_file_to_id():
    """로스터 kbid 열 → {selftest파일.c: 자판id} (kbid "-" = 다중 레이아웃/미등록 → 귀속 제외)."""
    return {stem + ".c": kbid for stem, kbid, _d in parse_selftest_roster() if kbid != "-"}

# --------------------------------------------------------------------------
# 카탈로그 조립
# --------------------------------------------------------------------------

def build_registry():
    """5-소스 UNION id-registry. {id: {names(set of 표시명 후보), sources(set), layout, ships, dropped}}."""
    kjaso   = parse_kjaso_builtin()          # 소스1: 빌트인 jaso
    names   = parse_keyboard_names()         # 소스2: 표시명 오버라이드
    ext     = parse_external_xml()           # 소스3: 외부 XML
    embed   = parse_embed_xml_ids()          # 소스4: 임베드 XML
    shell   = parse_shell_meta()             # 소스5: 셸 메타(순정9 메뉴 표시명)
    base_xml = parse_base_shipped_xml()      # 본체 출하 외부 XML(NSI 라이브)
    # 소스6: libhangul 순정 9 멤버십(상수 PRISTINE9) · 소스7: 생성 XML(레이아웃 백킹)
    reg = {}

    def touch(kid):
        return reg.setdefault(kid, {"names": [], "sources": set(), "layout": None,
                                    "ships": None, "type": ""})

    for kid, layout in kjaso.items():
        e = touch(kid); e["sources"].add("kJasoBuiltin"); e["layout"] = layout
    for kid in PRISTINE9:
        e = touch(kid); e["sources"].add("libhangul순정9")
    for kid in embed:
        e = touch(kid); e["sources"].add("임베드XML")
    for kid, info in ext.items():
        e = touch(kid); e["sources"].add("외부XML")
        if info["name"]:
            e["names"].append(info["name"])
        e["type"] = info["type"]
    for kid, info in shell.items():
        e = touch(kid); e["sources"].add("셸메타"); e["names"].append(info["name"])
    for layout, (kid, ships, _src) in GENERATED_XML_LAYOUTS.items():  # 소스6: 생성 외부 XML(레이아웃 백킹)
        e = touch(kid); e["sources"].add("생성XML"); e["layout"] = layout
    for kid, nm in names.items():
        e = touch(kid); e["sources"].add("표시명")
        e["names"].insert(0, nm)   # 표시명 오버라이드 우선

    # 출하상태(ships) 정적 추정: 순정9 + 임베드4 + 본체XML3 = 본체(base). 그 외 구조적 소스보유 =
    #   애드온. ★인스톨러 File 목록 대조는 2단계(현재는 소스 존재 기반 추정 — 근사판 등은 HIDDEN 상수로 보정).
    STRUCTURAL = {"kJasoBuiltin", "외부XML", "임베드XML", "생성XML", "libhangul순정9"}
    for kid, e in reg.items():
        if kid in KNOWN_DROPPED:
            e["ships"] = "DROPPED"
        elif kid in HIDDEN:
            e["ships"] = "숨김(미배포)"
        elif kid in PRISTINE9 or kid in embed or kid in base_xml:
            e["ships"] = "본체"
        elif e["sources"] & STRUCTURAL:
            e["ships"] = "애드온"
        else:
            e["ships"] = "?(표시명만)"
    return reg

def display_name(reg, kid):
    e = reg.get(kid)
    if e and e["names"]:
        return e["names"][0]
    return "(표시명 미상)"

def layout_to_ids(kjaso):
    """JASO_LAYOUT_* → [id..] 역인덱스 (kJasoBuiltin + 생성 외부 XML)."""
    rev = {}
    for kid, layout in kjaso.items():
        rev.setdefault(layout, []).append(kid)
    for layout, (kid, _ships, _src) in GENERATED_XML_LAYOUTS.items():
        rev.setdefault(layout, []).append(kid)
    return rev

def layout_file_to_selftests(manifest):
    """jaso_layout_*.c (basename) → [gate..] : 그 레이아웃 소스를 컴파일하는 게이트들."""
    rev = {}
    for gate, srcs in manifest.items():
        for s in srcs:
            if s.startswith("jaso_layout_"):
                rev.setdefault(s, []).append(gate)
    return rev

# --------------------------------------------------------------------------
# 진단 헬퍼
# --------------------------------------------------------------------------
def warn(msg): sys.stderr.write("⚠ %s\n" % msg)
def die(msg):
    sys.stderr.write("✗ %s\n" % msg); sys.exit(2)

# --------------------------------------------------------------------------
# 커맨드: catalog
# --------------------------------------------------------------------------
def cmd_catalog(argv):
    layouts  = parse_layouts()
    kjaso    = parse_kjaso_builtin()
    reg      = build_registry()
    rev      = layout_to_ids(kjaso)

    # M3b-lite 드리프트: 미분류 신규 필드(새 기제일 수 있음) / 사라진 기제 필드 경고.
    unclassified, vanished = mechanism_drift()
    if unclassified:
        warn("jaso_layout 신규 미분류 필드: %s — 새 기제? MECHANISM_FIELDS 또는 STRUCTURAL_FIELDS 에 분류하라."
             % ", ".join(unclassified))
    if vanished:
        warn("MECHANISM_FIELDS 에 있으나 구조체서 사라진 필드: %s — 도구 갱신 필요." % ", ".join(vanished))

    print("# 런타임 체크리스트 카탈로그 (1단계 정적 메타)  — 정본 설계 docs/TEST_CHECKLIST_AUTOMATION_DESIGN.md\n")

    print("## 레이아웃 × 기제 매트릭스 (17 인스턴스 universe · 빈칸=기제 OFF[zero default])\n")
    hdr = ["layout", "자판(id)", "출하"] + [f for (f, *_r) in MECHANISM_FIELDS]
    print("| " + " | ".join(hdr) + " |")
    print("|" + "|".join(["---"] * len(hdr)) + "|")
    for L in layouts:
        ids = rev.get(L["name"], [])
        idcell = ", ".join(ids) if ids else "(미등록)"
        ships = "/".join(sorted({reg[i]["ships"] for i in ids})) if ids else "—"
        cells = [L["name"].replace("JASO_LAYOUT_", ""), idcell, ships]
        for (f, *_r) in MECHANISM_FIELDS:
            v = L["mech"].get(f)
            if f == "role_flip":
                cells.append(v)                      # 항상 값(타입 본질)
            else:
                cells.append("●" if v else "")       # ON=● / OFF=빈칸
        print("| " + " | ".join(cells) + " |")

    print("\n## id-registry (5-소스 UNION + known-dropped)\n")
    print("| id | 표시명 | 출하 | 소스 | layout |")
    print("|---|---|---|---|---|")
    for kid in sorted(reg):
        e = reg[kid]
        print("| %s | %s | %s | %s | %s |" % (
            kid, display_name(reg, kid), e["ships"],
            ",".join(sorted(e["sources"])),
            (e["layout"] or "").replace("JASO_LAYOUT_", "") or "—"))

    print("\n## known-dropped (수기 상수 · DECISIONS 출처)\n")
    for kid, src in sorted(KNOWN_DROPPED.items()):
        print("- `%s` — %s" % (kid, src))
    return 0

# --------------------------------------------------------------------------
# 커맨드: select
# --------------------------------------------------------------------------
def _git_changed():
    try:
        out = subprocess.check_output(["git", "-C", ROOT, "diff", "--name-only", "HEAD"],
                                      stderr=subprocess.DEVNULL).decode("utf-8", "replace")
        files = [l.strip() for l in out.splitlines() if l.strip()]
        return files
    except Exception:
        return []

def cmd_select(argv):
    files = argv if argv else _git_changed()
    if not files:
        print("변경 파일 없음 (인자 미지정 + git diff 비었음). 사용: select <파일..>")
        return 0
    layouts = parse_layouts()
    kjaso   = parse_kjaso_builtin()
    reg     = build_registry()
    rev     = layout_to_ids(kjaso)
    manifest = parse_build_manifest()
    lf2gate = layout_file_to_selftests(manifest)
    by_name = {L["name"]: L for L in layouts}
    # 레이아웃 name → 그 파일 basename
    file_layouts = {}
    for L in layouts:
        file_layouts.setdefault(os.path.basename(L["file"]), []).append(L)

    print("# 셀렉터 — 변경셋 → 런타임 검증 추천 (1단계)")
    print("> 표기(§2.5): 입력=QWERTY / 기대=한글 / 자판=표시명(id). expected 는 RUNTIME_TEST_CHECKLIST 검증행 *인용*(자동생성 아님).\n")

    for f in files:
        base = os.path.basename(f)
        print("## 변경: %s" % f)

        if base.startswith("jaso_layout_") and base.endswith(".c"):
            Ls = file_layouts.get(base, [])
            if not Ls:
                print("  ⚠ canonical 에 없는 레이아웃 파일 — 수동 확인.\n")
                continue
            for L in Ls:
                ids = rev.get(L["name"], [])
                _print_layout_targets(L, ids, reg)
            gates = lf2gate.get(base, [])
            print("  - 앵커(게이트): %s" % (", ".join(sorted(gates)) or "(빌드 manifest 부재 — census 확인)"))
            print()

        elif base in ("jaso_strat.c", "engine-jaso-core.c"):
            _print_strat_targets(base, layouts, rev, reg)
            print()

        elif base == "jaso_chord.c":
            print("  → chord 패밀리 횡단(세모이·두겹이/두줄이·안마태·3gs·공동).")
            print("    ★셸 동시타 상태기계는 골든/selftest 미커버 → **런타임 실입력 필수**(README §런타임 추가검증).")
            print("    대표: 안마태 `fjv`→각 · 3gs chord · 두겹이 겹받침. 도착순 순열 짝 의무.")
            print()

        elif base == "jaso_xml_loader.c":
            xml_ids = [k for k, e in reg.items() if "외부XML" in e["sources"]]
            print("  → 외부 XML 로드 경로 횡단 → 외부 XML 자판 전체(%d): %s"
                  % (len(xml_ids), ", ".join(sorted(xml_ids))))
            print("    대표 라운드: 314 표준/반자동·신세벌 XML·안마태. XML 설치+startup 로드 실확인(ships≠런타임).")
            print()

        elif base.endswith(".xml"):
            kid = _xml_file_to_id(f, reg)
            if kid:
                e = reg.get(kid, {})
                print("  → 외부 XML 자판: %s (%s) [%s]" % (kid, display_name(reg, kid), e.get("ships", "?")))
                print("    라운드: 그 자판 C 대표 + 로드/등록/표시명 확인.")
            else:
                print("  → XML(조합 데이터 or id 미상) — 수동 확인.")
            print()

        elif base in ("v2backend.cpp", "v2backend.h"):
            print("  → 라우팅/등록 횡단(kJasoBuiltin·whitelist·XML 등록). 라운드: 자판 목록 열거·라우팅(S절).")
            print()

        elif base in ("NabiCloud.cpp", "CompositionProcessorEngine.cpp", "KeyEventSink.cpp",
                      "SettingsWebView.cpp", "CompositionCore.cpp"):
            print("  → 셸/TSF 표면(S절 human-only): 설정UI·한/영·한자후보창·메뉴열거·preserved 키.")
            print("    골든 불가 → 템플릿 절차 수동 확인.")
            print()

        elif base.endswith(".nsi") or "installer" in f:
            print("  → 인스톨러(S절): 배포 payload·단독/표준/전체·재설치보존·자판 File 목록.")
            print("    라운드: 설치 매트릭스 수동(L-0 선례). UAC 필요(T.K.).")
            print()

        else:
            print("  → 매핑 규칙 없음 — 수동 판단. (셀렉터 규칙 추가 후보: docs 설계 §5)")
            print()

    print("---")
    print("★ 대칭짝 의무(TEST_DESIGN_GUIDELINES §1): 각 SUCCESS 마다 짝(역순·backspace·fire/non-fire·")
    print("  가상경유/직입력·yet conjoining/NFC·chord 도착순). 짝을 못 찾으면 '⚠ 짝 부재' 로 남기고 날조 금지.")
    return 0

def _print_layout_targets(L, ids, reg):
    on = [MECH_BY_FIELD[f] for (f, *_r) in MECHANISM_FIELDS
          if f != "role_flip" and L["mech"].get(f)]
    role = L["mech"].get("role_flip")
    if ids:
        for kid in ids:
            e = reg[kid]
            print("  → 자판: %s (%s) [%s]" % (kid, display_name(reg, kid), e["ships"]))
    else:
        print("  → 레이아웃 %s = **미등록**(kJasoBuiltin·XML 부재) → 런타임 표면 없음(selftest-only)."
              % L["name"].replace("JASO_LAYOUT_", ""))
        print("    (설계 §8 미배선 기제 정직표기 · RUNTIME L-5 선례)")
    print("  → 기제: role_flip=%s%s" % (role, "".join("; " + m[1] for m in on)))
    if on:
        print("  → 추천 라운드:")
        for m in on:
            print("     - [%s] %s → %s" % (m[2], m[1], m[3]))
    else:
        print("  → 추천 라운드: C 사슬-스모크 대표 1행 (role=%s 기본 슬롯)." % role)

def _print_strat_targets(base, layouts, rev, reg):
    if base == "engine-jaso-core.c":
        print("  → **커널 횡단** — 전 jaso 자판 영향. 골든 byte-불변이 1차 방벽(자동).")
        print("    런타임: 타입별 대표 스모크(두벌 `2`·세벌 `3f`·순아래 `3gs`·신세벌 `3shin-p2`).")
        return
    # jaso_strat.c : 보강3 — phase-local 기본 + 공유 helper/수명/backspace/render 는 related phase 승격.
    print("  → **jaso_strat.c 횡단** (보강3): 기본 phase-local, 단 공유 helper 변경은 related phase 승격.")
    print("    ★diff 가 `jaso_dispatch_l2` *특정 phase 블록만* 건드리면 그 phase 자판만;")
    print("      공유 helper(resolve_virtual·ctx_virtual·jaso_pre·jaso_post·jaso_backspace_step)/")
    print("      backspace/render 를 건드리면 **related phase 전체 승격**(과소 스코프로 횡단 회귀 놓침 방지).")
    print("    phase → 대표 자판:")
    # phase별로 그 기제를 켠 레이아웃의 id 를 모은다.
    phase_ids = {}
    for L in layouts:
        ids = rev.get(L["name"], [])
        if not ids:
            continue
        for (f, *_r) in MECHANISM_FIELDS:
            if f == "role_flip":
                continue
            if L["mech"].get(f):
                ph = MECH_BY_FIELD[f][2]
                phase_ids.setdefault(ph, set()).update(ids)
    for ph in sorted(phase_ids):
        print("      - %s: %s" % (ph, ", ".join(sorted(phase_ids[ph]))))

def _xml_file_to_id(f, reg):
    base = os.path.basename(f)
    for kid, info in parse_external_xml().items():   # 외부XML 파일경로→id (build_registry 밖 원천)
        if os.path.basename(info["file"]) == base:
            return kid
    return None

# --------------------------------------------------------------------------
# 커맨드: census (보강2) — canonical selftest ∩ 빌드 manifest
# --------------------------------------------------------------------------
def cmd_census(argv):
    manifest = parse_build_manifest()
    manifested_srcs = set()
    for srcs in manifest.values():
        manifested_srcs.update(s for s in srcs if s.startswith("selftest"))
    # canonical 의 tracked selftest*.c (파일 실재). ★`selftest.c`(galma·언더바 없음)도 포함.
    files = {os.path.basename(p) for p in glob.glob(os.path.join(CANON, "selftest*.c"))}

    # 명시적 제외(정당한 미-manifest): {basename: 사유}. 새 파일이 침묵 누락되지 않게 여기 등록 강제.
    EXCLUDE = {
        "selftest_jaso_gongdong_xml.c": "gongdong XML 왕복 — .bat 미배선(공동 미등록·승격 대기). 설계 §8/보강2.",
    }

    print("# census (보강2) — canonical selftest_*.c ∩ 빌드 manifest\n")
    fail = 0
    unmanifested = sorted(files - manifested_srcs)
    print("## 파일 실재하나 빌드 manifest 부재")
    for s in unmanifested:
        if s in EXCLUDE:
            print("  - %s  [제외 OK: %s]" % (s, EXCLUDE[s]))
        else:
            print("  - %s  ✗ 누락(빌드 안 됨·게이트 미커버) — .bat 에 추가하거나 EXCLUDE 등록" % s)
            fail += 1
    missing_files = sorted(s for s in manifested_srcs if s not in files)
    if missing_files:
        print("\n## manifest 참조하나 파일 부재")
        for s in missing_files:
            print("  - %s  ✗ 소스 없음(빌드 실패 예상)" % s)
            fail += 1
    print("\n%s" % ("CENSUS-OK" if fail == 0 else "CENSUS-FAIL=%d" % fail))
    return 1 if fail else 0

# --------------------------------------------------------------------------
# 커맨드: check (보강1) — known-dropped sync
# --------------------------------------------------------------------------
def cmd_check(argv):
    reg = build_registry()
    fail = 0

    print("# check — 드리프트 게이트 (보강1 known-dropped sync + M3b 기제필드 드리프트)\n")

    print("## 기제 필드 드리프트 (jaso_layout 구조체 ↔ 도구 분류)")
    unclassified, vanished = mechanism_drift()
    for f in unclassified:
        print("  - ✗ 미분류 신규 필드 `%s` — 새 기제? MECHANISM_FIELDS/STRUCTURAL_FIELDS 에 분류" % f); fail += 1
    for f in vanished:
        print("  - ✗ 사라진 기제 필드 `%s` — 도구 갱신 필요" % f); fail += 1
    if not unclassified and not vanished:
        print("  - OK (구조체 멤버 전부 분류됨)")

    print("\n## known-dropped id sync (라이브 레지스트리 재등장 여부)")
    for kid, src in sorted(KNOWN_DROPPED.items()):
        # 실 구조적 소스로 재등장하면 fail(표시명 override 만 있는 건 무해).
        e = reg.get(kid)
        live = e and (e["sources"] - {"표시명"})
        if live:
            print("  - `%s` ✗ 재등장(%s) — 출하 복귀? DECISIONS 재확인 필요 (%s)"
                  % (kid, ",".join(sorted(e["sources"])), src)); fail += 1
        else:
            print("  - `%s` OK (미등장) — %s" % (kid, src))

    # 로스터 sync (2026-07-02 수렴): selftest_roster.h(단일 정본) ↔ .bat manifest 양방향 대조
    #   + kbid 레지스트리 실재 검증. 목적 = 사본 간 *조용한 어긋남* 차단(정직게이트).
    print("\n## selftest 로스터 sync (selftest_roster.h ↔ build_cleanroom_selftests.bat)")
    roster = parse_selftest_roster()
    manifest = parse_build_manifest()
    man_files = set()
    for srcs in manifest.values():
        man_files.update(s for s in srcs if s.startswith("selftest"))
    stems = [stem for stem, _k, _d in roster]
    dups = sorted({s for s in stems if stems.count(s) > 1})
    for s in dups:
        print("  - ✗ 로스터 stem 중복 `%s` — 행 정리 필요" % s); fail += 1
    ros_files = {s + ".c" for s in stems}
    for f in sorted(man_files - ros_files):
        print("  - ✗ `%s` .bat manifest 에만 있음 — selftest_roster.h 에 행 추가" % f); fail += 1
    for f in sorted(ros_files - man_files):
        print("  - ✗ `%s` 로스터에만 있음 — .bat 미배선 or stem 오타" % f); fail += 1
    bad_kb = sorted((stem, k) for stem, k, _d in roster if k != "-" and k not in reg)
    for stem, k in bad_kb:
        print("  - ✗ 로스터 kbid `%s`(%s) 가 id-registry 에 없음 — 오타/드롭 id?" % (k, stem)); fail += 1
    if not (dups or (man_files - ros_files) or (ros_files - man_files) or bad_kb):
        print("  - OK (로스터 %d 행 ≡ manifest selftest %d · 귀속 kbid %d 전부 레지스트리 실재)"
              % (len(roster), len(man_files), sum(1 for _s, k, _d in roster if k != "-")))

    print("\n%s" % ("DRIFT-CHECK-OK" if fail == 0 else "DRIFT-CHECK-FAIL=%d" % fail))
    return 1 if fail else 0

# --------------------------------------------------------------------------
# 커맨드: collect (2단계 instrument-and-run) — 계측 selftest 를 -DEMIT_JSON 빌드+실행→방출 파싱
#   ★계측 파일 자동 감지(selftest_emit.h include) → build manifest 에서 srcs 조회 → build_one_emit.cmd.
#     새 파일 계측 시 .bat 수정 불요(자동 포함). expected 는 방출값(재유도 0) = 2단계 권위 원천.
# --------------------------------------------------------------------------
# golden 독립앵커(M2): out-of-band 하니스(libhangul 등)의 케이스 → {(kb_hint, keys): [want_cps]}.
#   selftest 방출 expected 를 이것과 교차 → 불일치=hard-fail(co-wrong expected 잡음, §6 M2).
GOLDEN_FILE_HINT = {"selftest_jaso_318na.c": "318na"}   # 계측 selftest file → golden kb hint

# in-C 동결 오라클 커버(§8 M2 보조 독립앵커): 이 파일의 레이아웃은 *독립 구현 오라클*과 oracle_diff
#   차등 게이트(exhaustive 길이1-3)로 이미 검증됨 → collect 가 직접 재대조하진 않지만(그건 in-C 게이트),
#   "golden부재"로 과소표기하지 않고 "in-C 오라클 커버"로 정직 표면화. (golden_318na=Python 직접교차와 구분.)
#   p2=oracle_shin_p2·dubeol(galmadeuli 전용)=oracle_galmadeuli. none 은 3f+galma 혼재라 제외(귀속 불가).
FILE_ORACLE = {"selftest_jaso_p2.c": "oracle_shin_p2", "selftest_jaso_dubeol.c": "oracle_galmadeuli"}

def parse_golden_anchors():
    anchors = {}
    gp = os.path.join(ROOT, "shared", "engine", "tests", "golden_318na.c")
    if os.path.exists(gp):
        # expect(id, "name", "keys", "U+XXXX U+YYYY ...")  — libhangul 독립 하니스 측정값
        for keys, want in re.findall(
                r'expect\(\s*\w+\s*,\s*"[^"]*"\s*,\s*"([^"]*)"\s*,\s*"([^"]*)"', _read(gp)):
            cps = [int(x[2:], 16) for x in want.split() if x.startswith("U+")]
            anchors[("318na", keys)] = cps
    return anchors

def _golden_check(rec, golden):
    """방출 케이스 rec 을 golden 앵커와 교차. 반환 (provenance, is_m2_fail)."""
    hint = GOLDEN_FILE_HINT.get(rec.get("file"))
    if not hint:
        return "lit-anchor·golden부재", False
    toks = rec.get("input", "").split()
    if not toks or not all(len(t) == 1 for t in toks):   # 제어 op(BS/RESET/FLUSH) 포함 = golden 키 아님
        return "lit-anchor·golden부재", False
    g = golden.get((hint, "".join(toks)))
    if g is None:
        return "lit-anchor·golden부재", False
    if g == rec.get("expected", []):
        return "✓validated(golden_%s)" % hint, False
    return "✗M2-FAIL golden=%s≠expected=%s" % (g, rec.get("expected")), True

def _instrumented_selftests():
    """selftest_emit.h 를 include 하는 canonical selftest*.c basename 집합(=계측됨)."""
    out = set()
    for p in glob.glob(os.path.join(CANON, "selftest*.c")):
        if "selftest_emit.h" in _read(p):
            out.add(os.path.basename(p))
    return out

def _decode(cps, altitude):
    """코드포인트 배열 → 사람 읽기. commit=한글 이어붙임, slots=[on,nu,co](0=∅)."""
    if altitude == "slots":
        return "[" + ",".join("∅" if c == 0 else "U+%04X %s" % (c, chr(c)) for c in cps) + "]"
    return "".join(chr(c) for c in cps if c) or "(빈 커밋)"

def _collect_records():
    """계측 selftest 를 -DEMIT_JSON 빌드+실행 → (records, build_fail, instrumented). collect/verify 공유."""
    manifest = parse_build_manifest()
    instrumented = _instrumented_selftests()
    targets = []
    for gate, srcs in manifest.items():
        exp = [CORE_SRC if s == "%CORE%" else s for s in srcs]
        first = next((s for s in exp if s.startswith("selftest")), None)
        if first in instrumented:
            targets.append((gate, exp))
    records, build_fail = [], 0
    for gate, srcs in sorted(targets):
        try:
            r = subprocess.run(["cmd", "/c", BUILD_EMIT, gate] + srcs,
                               capture_output=True, text=True, encoding="utf-8", errors="replace")
        except Exception as e:
            warn("build_one_emit 실행 실패(%s): %s" % (gate, e)); build_fail += 1; continue
        if r.returncode != 0:
            warn("emit 빌드 실패 %s (stderr 상단): %s" % (gate, (r.stderr or "").strip().splitlines()[:1]))
            build_fail += 1; continue
        for line in r.stdout.splitlines():
            line = line.strip()
            if line.startswith('{"emit"'):
                try:
                    records.append(json.loads(line))
                except json.JSONDecodeError:
                    warn("방출 JSON 파싱 실패: %s" % line[:80])
    return records, build_fail, instrumented

def cmd_collect(argv):
    instrumented = _instrumented_selftests()
    if not instrumented:
        print("계측된 selftest 없음(selftest_emit.h include 파일 0). 2단계 rollout 대기."); return 0
    print("# collect (2단계 instrument-and-run) — 계측 selftest 방출 (expected=엔진 실측, 재유도 0)")
    print("> 계측: %s\n" % ", ".join(sorted(instrumented)))
    records, build_fail, _ = _collect_records()

    # golden 독립앵커(M2): golden-커버 자판(현재 318na)이 계측되면 방출 expected 를 out-of-band
    #   golden 과 교차 → 불일치 hard-fail(co-wrong expected 방지, §6 M2). 나머지는 정직 "golden부재"(B3).
    golden = parse_golden_anchors()
    m2_fail, validated, oracle_backed = 0, 0, 0
    print("## 방출 케이스 (%d 건)\n" % len(records))
    print("| file | id | altitude | input(키) | expected(한글) | ok | provenance |")
    print("|---|---|---|---|---|---|---|")
    for r in records:
        prov, is_fail = _golden_check(r, golden)
        if is_fail:
            m2_fail += 1
        elif prov == "lit-anchor·golden부재":            # golden 부재 → in-C 오라클 커버면 표면화
            orc = FILE_ORACLE.get(r.get("file"))
            if orc:
                prov = "in-C 오라클:%s (oracle_diff)" % orc; oracle_backed += 1
        if prov.startswith("✓validated"): validated += 1
        print("| %s | %s | %s | %s | %s | %s | %s |" % (
            r.get("file", "?"), r.get("id", "?"), r.get("altitude", "?"),
            "`%s`" % r.get("input", ""), _decode(r.get("expected", []), r.get("altitude", "")),
            "✓" if r.get("ok") else "✗", prov))
    n_fail = sum(1 for r in records if not r.get("ok"))
    print("\n방출 %d 건 · self-fail %d · 빌드실패 %d · M2 golden-validated %d · in-C 오라클커버 %d · M2-FAIL %d"
          % (len(records), n_fail, build_fail, validated, oracle_backed, m2_fail))
    if m2_fail:
        print("✗ M2 독립앵커 불일치 — 방출 expected 가 golden(독립 하니스)과 어긋남(co-wrong 의심). 위 ✗M2-FAIL 행 확인.")
    else:
        print("★독립앵커: golden 교차 %d(318na) + in-C 오라클 %d(p2 shin_p2·dubeol galmadeuli, oracle_diff 게이트)."
              % (validated, oracle_backed))
        print("  나머지 = selftest-derived(독립앵커 없음·validated 주장 안 함=B3).")
    return 1 if (n_fail or build_fail or m2_fail) else 0

# --------------------------------------------------------------------------
# 커맨드: verify (2단계 검증패스) — RUNTIME_TEST_CHECKLIST 수기 행 ↔ 방출값 대조(§7 2단계)
#   1단계가 수기 인용하는 체크리스트 행(입력→기대 U+)을 2단계 엔진 방출과 교차 → 체크리스트
#   오타/드리프트 자가검출. 매칭=입력키 정규화(키보드-무관: 입력→기대가 *엔진 어딘가서* 나오는지).
# --------------------------------------------------------------------------
def _parse_checklist_rows():
    """RUNTIME_TEST_CHECKLIST.md 표 → [(id, input_norm, [cps], raw)]. 백틱 입력 + U+ 기대 있는 행만."""
    rows = []
    if not os.path.exists(RUNTIME_MD):
        return rows
    for ln in _read(RUNTIME_MD).splitlines():
        if not ln.lstrip().startswith("|"):
            continue
        cells = [c.strip() for c in ln.strip().strip("|").split("|")]
        if len(cells) < 3:
            continue
        # id = 자판 셀(cells[0])의 마지막 `token`; 입력 = 그 다음 셀의 백틱 토큰 이어붙임
        ids = re.findall(r"`([0-9A-Za-z][\w./+-]*)`", cells[0])
        if not ids:
            continue
        kid = ids[-1]
        # 입력 셀 후보: cells[1] 의 백틱 토큰(키). 전부 있어야.
        keytoks = re.findall(r"`([^`]+)`", cells[1])
        if not keytoks:
            continue
        inp = "".join(keytoks)
        if " " in inp or not inp:
            continue   # 백틱 밖 텍스트 섞인 입력은 스킵(모호)
        cps = [int(x, 16) for x in re.findall(r"U\+([0-9A-Fa-f]{4,6})", " ".join(cells[2:]))]
        if not cps:
            continue
        rows.append((kid, inp, cps, ln.strip()))
    return rows

# 단일-레이아웃 계측 selftest → keyboard id (귀속 확정 → 키보드-인지 드리프트 게이트 가능).
#   ★수기 dict(14종 사본)를 selftest_roster.h 의 kbid 열 파생으로 수렴(2026-07-02) —
#     다중-레이아웃 파일(none/shinshift/virtualunit/yet/dubeol)·미등록(gongdong)= kbid "-" 로 제외.
#     로스터 ↔ .bat manifest 드리프트는 check 커맨드가 게이트(불일치=FAIL).
#   사용처(cmd_verify)에서 roster_file_to_id() 호출 — import 시점 파일읽기 회피.

def cmd_verify(argv):
    if not _instrumented_selftests():
        print("계측된 selftest 없음 — verify 불가."); return 0
    rows = _parse_checklist_rows()
    if not rows:
        print("RUNTIME_TEST_CHECKLIST 에서 대조 가능(백틱입력+U+기대) 행 0."); return 0
    records, build_fail, _ = _collect_records()
    # 두 인덱스: (a) 키보드-인지 (id,입력)→{expected}  (단일-레이아웃 파일만·귀속 확정 → 진짜 드리프트)
    #           (b) 키보드-무관 입력→{expected}       (전 계측·약한 긍정만)
    kb_idx, any_idx = {}, {}
    file_to_id = roster_file_to_id()   # 로스터 kbid 열 파생(단일 정본 selftest_roster.h)
    for r in records:
        if r.get("altitude") != "commit":
            continue
        toks = r.get("input", "").split()
        if not toks or not all(len(t) == 1 for t in toks):
            continue   # 제어 op(BS/RESET/FLUSH) = QWERTY 매칭 대상 아님
        key = "".join(toks); exp = tuple(r.get("expected", []))
        any_idx.setdefault(key, set()).add(exp)
        kid = file_to_id.get(r.get("file"))
        if kid:
            kb_idx.setdefault((kid, key), set()).add(exp)

    print("# verify (2단계 검증패스) — RUNTIME_TEST_CHECKLIST 수기 행 ↔ 엔진 방출 대조")
    print("> 판정: **자판귀속**(단일-레이아웃 계측 자판=(id,입력) 정확대조 → 진짜 드리프트 하드-실패) /")
    print(">   **무관확인**(입력→기대 쌍이 엔진 어딘가 실재=약한 긍정) / **미대조**(자판 미계측·입력표기 상이).")
    print(">   같은 입력이 자판마다 달라 무관매칭은 긍정만 신뢰(설계 anti-false-authority). 대조행 %d.\n" % len(rows))
    print("| id | 입력 | 체크리스트 기대 | 판정 |")
    print("|---|---|---|---|")
    ok_kb = weak = uncov = drift = 0
    drift_rows = []
    for kid, inp, cps, raw in rows:
        want = "".join(chr(c) for c in cps)
        kbset = kb_idx.get((kid, inp))
        if kbset is not None:                       # 자판귀속: 정확 대조(건전)
            if tuple(cps) in kbset:
                verdict = "✓ 자판귀속 확인"; ok_kb += 1
            else:
                eng = " / ".join("".join(chr(c) for c in t) for t in sorted(kbset))
                verdict = "✗ 드리프트(자판귀속: 엔진=%s)" % eng; drift += 1
                drift_rows.append((kid, inp, want, eng))
        elif idx_any_has(any_idx, inp, cps):        # 무관: 약한 긍정
            verdict = "~ 무관확인(약)"; weak += 1
        else:
            verdict = "· 미대조"; uncov += 1
        print("| %s | `%s` | %s | %s |" % (kid, inp, want, verdict))
    print("\n자판귀속확인 %d · 무관확인 %d · 미대조 %d · **드리프트 %d** · 빌드실패 %d (총 %d 행)"
          % (ok_kb, weak, uncov, drift, build_fail, len(rows)))
    for kid, inp, want, eng in drift_rows:
        print("  ✗ %s `%s`: 체크리스트=%s ≠ 엔진=%s — 체크리스트 오타 or 엔진 회귀 재확인" % (kid, inp, want, eng))
    if drift == 0:
        print("★ 자판귀속 드리프트 0 — 귀속 대조된 체크리스트 행이 전부 엔진과 일치.")
    return 1 if (drift or build_fail) else 0

def idx_any_has(any_idx, inp, cps):
    s = any_idx.get(inp)
    return s is not None and tuple(cps) in s

# --------------------------------------------------------------------------
# 커맨드: pairs (대칭짝 자동조립) — 방출 케이스서 실측 대칭짝 검출(설계 §1·§5 SUCCESS+짝 원칙)
#   ① backspace 짝: 전진 케이스 X ↔ X+⌫ 케이스(공유 접두). ② fire/non-fire 짝: 트리거키 t 가
#   단독=리터럴(non-fire) vs 조합중=자모발화(fire). 날조 아니라 *방출된 실측 케이스만* 짝지음.
# --------------------------------------------------------------------------
def cmd_pairs(argv):
    if not _instrumented_selftests():
        print("계측된 selftest 없음 — pairs 불가."); return 0
    records, build_fail, _ = _collect_records()
    commits = [r for r in records if r.get("altitude") == "commit"]

    # file → {input_tokens_tuple: (input_str, expected)}  (전진=제어op 없는 케이스)
    fwd = {}
    for r in commits:
        toks = r["input"].split()
        if "BS" in toks or "RESET" in toks or "FLUSH" in toks:
            continue
        fwd.setdefault(r["file"], {})[tuple(toks)] = (r["input"], r.get("expected", []))

    # ① backspace 짝: 입력이 [prefix..., "BS"] (단일 후행 BS) 이고 prefix 전진 케이스가 방출됐으면 짝.
    bs_pairs = []
    for r in commits:
        toks = r["input"].split()
        if not toks or toks[-1] != "BS" or "BS" in toks[:-1]:
            continue
        prefix = tuple(toks[:-1])
        f = fwd.get(r["file"], {})
        if prefix in f:
            fin, fexp = f[prefix]
            bs_pairs.append((r["file"], fin, _decode(fexp, "commit"),
                             r["input"], _decode(r.get("expected", []), "commit")))

    # ② fire/non-fire 짝: 단일 토큰 t 가 리터럴 커밋(exp==[ord(t)])=non-fire, 조합중 발화=fire.
    fire_pairs = []
    for f, cases in fwd.items():
        literals = {}   # t -> (input, exp)
        for toks, (ins, exp) in cases.items():
            if len(toks) == 1 and len(toks[0]) == 1 and len(exp) == 1 and exp[0] == ord(toks[0]):
                literals[toks[0]] = (ins, exp)
        for t, (nin, nexp) in sorted(literals.items()):
            fire = None
            for toks, (ins, exp) in cases.items():
                if len(toks) > 1 and t in toks and not (len(exp) == 1 and exp[0] == ord(t)):
                    fire = (ins, exp); break
            if fire:
                fire_pairs.append((f, t, fire[0], _decode(fire[1], "commit"),
                                   nin, _decode(nexp, "commit")))

    print("# pairs (대칭짝 자동조립) — 방출 실측 케이스서 검출(§1 SUCCESS+짝; 날조 아님)\n")
    print("## ① backspace 짝 (전진 ↔ ⌫) — %d 쌍\n" % len(bs_pairs))
    print("| file | 전진 입력→기대 | ⌫ 입력→기대 |")
    print("|---|---|---|")
    for f, fin, fexp, bin_, bexp in bs_pairs:
        print("| %s | `%s`→%s | `%s`→%s |" % (f, fin, fexp, bin_, bexp))
    print("\n## ② fire/non-fire 짝 (조건부 트리거) — %d 쌍\n" % len(fire_pairs))
    print("| file | 트리거 | fire 입력→기대 | non-fire 입력→기대 |")
    print("|---|---|---|---|")
    for f, t, fin, fexp, nin, nexp in fire_pairs:
        print("| %s | `%s` | `%s`→%s | `%s`→%s |" % (f, t, fin, fexp, nin, nexp))
    print("\n요약: backspace짝 %d · fire/non-fire짝 %d · 빌드실패 %d" % (len(bs_pairs), len(fire_pairs), build_fail))
    print("★ 방출된 실측 케이스만 짝지음(없는 짝은 날조 안 함). 셀렉터 대칭짝 의무의 데이터-구동 근거.")
    return 1 if build_fail else 0

# --------------------------------------------------------------------------
def main():
    cmds = {"catalog": cmd_catalog, "select": cmd_select, "census": cmd_census,
            "check": cmd_check, "collect": cmd_collect, "verify": cmd_verify, "pairs": cmd_pairs}
    if len(sys.argv) < 2 or sys.argv[1] not in cmds:
        sys.stderr.write(__doc__ if __doc__ else "")
        sys.stderr.write("\n사용: python runtime_checklist_catalog.py {catalog|select|census|check|collect|verify|pairs} [args]\n")
        return 2
    return cmds[sys.argv[1]](sys.argv[2:])

if __name__ == "__main__":
    sys.exit(main())
