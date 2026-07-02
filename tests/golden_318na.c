/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Old-engine 318na byte oracle;
   318na ships as V2 XML since 2026-06-25 and this oracle cannot read the V2 jaso-map body. */
/*
 * NabiCloud 318na functional golden (§27 약골든, clean-room, 2026-06-20).
 *
 * ★[2026-06-25 V2 이관 — RETIRED from CI] 318na 는 V2 확장 XML(engine="v2",
 *   role-flip="jongcycle")로 이관돼 keyboards\318na.xml 이 산들바람 V2 엔진으로 구동된다.
 *   이 골든은 *구엔진*(libhangul + nabicloud jongseong_cycle 전처리) 오라클이라 V2 본문
 *   (<jaso-map>)을 못 읽는다 → run_all_gates.bat GATES 에서 제거. 기능 커버리지는 V2 로
 *   이전: cleanroom selftest_jaso_318na(73 checks, oracle-anchored, 아래 12 케이스 전부 포함)
 *   + selftest_jaso_roundtrip(C≡XML). 이 파일은 구엔진 클린룸 오라클 *재현기*로 보존하며,
 *   build_and_verify_318na.bat 이 보존된 libhangul-body 원본(addons\318na\oracle\318na.xml)을
 *   KBDIR 로 가리켜 수동 재구동만 가능하다.
 *
 * The 3-18Na layout is an EXTERNAL XML keyboard, not a built-in, so the
 * cross-engine byte golden (golden_all) does not cover it.
 * This harness is a FUNCTIONAL golden instead: it drives a fixed list of physical
 * key sequences through the loaded 318na keyboard and asserts the resulting NFC
 * Hangul syllable (the composed preedit/commit string), comparing against
 * EXPECTED values DERIVED FROM THE PUBLIC SPEC (navilera.com 318Na page / KLDP),
 * not from any transcribed table. This is a behavior oracle for:
 *   - the data-driven jongseong-cycle pre-processor
 *     (nabicloud_engine_jongseong_cycle) and the unified preprocess gate;
 *   - the independently re-serialized 318na.xml (functional equivalence proof).
 *
 * Build/run: tests\build_and_verify_318na.bat (loads the XML from argv path).
 *
 * ASCII-only source (no UTF-8 BOM / CP949 trap). The expected syllables are
 * written as U+XXXX codepoints so the file stays ASCII.
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hangul.h"

/* Collect commit+preedit into one ucschar run, then to "U+XXXX ..." text. */
static void collect_text(HangulInputContext* hic, const char* keys,
                         char* out, size_t cap) {
    ucschar acc[64];
    size_t n = 0;
    acc[0] = 0;

    for (const char* p = keys; *p; ++p) {
        const ucschar* commit;
        hangul_ic_process(hic, (unsigned char)*p);
        commit = hangul_ic_get_commit_string(hic);
        for (; commit && *commit && n < 63; ++commit) acc[n++] = *commit;
    }
    {
        const ucschar* flush = hangul_ic_flush(hic);
        for (; flush && *flush && n < 63; ++flush) acc[n++] = *flush;
    }
    acc[n] = 0;

    out[0] = '\0';
    if (n == 0) { snprintf(out, cap, "(empty)"); return; }
    for (size_t i = 0; i < n; ++i) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%sU+%04X", (i ? " " : ""), (unsigned)acc[i]);
        strncat(out, tmp, cap - strlen(out) - 1);
    }
}

static int g_fail = 0;

/* One case: feed `keys` to a fresh 318na IC, expect text `want`. */
static void expect(const char* id, const char* name, const char* keys,
                   const char* want) {
    char got[256];
    HangulInputContext* hic = hangul_ic_new(id);
    if (hic == NULL) {
        printf("FAIL %-24s new==NULL (318na not loaded)\n", name);
        g_fail = 1;
        return;
    }
    collect_text(hic, keys, got, sizeof(got));
    hangul_ic_delete(hic);

    if (strcmp(got, want) == 0) {
        printf("PASS %-24s keys=%-10s -> %s\n", name, keys, got);
    } else {
        printf("FAIL %-24s keys=%-10s got=%s want=%s\n", name, keys, got, want);
        g_fail = 1;
    }
}

int main(int argc, char** argv) {
    const char* dir = (argc > 1) ? argv[1] : NULL;
    unsigned loaded = 0;
    const char* id = "318na";

    printf("# nabicloud golden_318na v1 (functional, do not edit by hand)\n");

    if (dir) {
        loaded = hangul_keyboard_list_load_dir(dir);
        printf("## loaded=%u from %s\n", loaded, dir);
    } else {
        printf("## no dir argument: relying on already-registered 318na\n");
    }

    /* Confirm the keyboard exists before the suite. */
    {
        const HangulKeyboard* kb = hangul_keyboard_list_get_keyboard(id);
        printf("## 318na present=%d\n", kb != NULL ? 1 : 0);
        if (kb == NULL) {
            printf("FAIL 318na keyboard not registered\n");
            return 1;
        }
    }

    printf("## cases\n");

    /* (1) the canonical compose: r=cho ㄱ, k=jung ㅏ, y=cycle key (ㅛ overlays
     *     jongseong ㄱ; medial present -> primary final) => 각 U+AC01. */
    expect(id, "c1_gak_rky",        "rky", "U+AC01");   /* 각 */

    /* (2) plain syllable, no final: r=ㄱ, k=ㅏ => 가 U+AC00. */
    expect(id, "c2_ga_rk",          "rk",  "U+AC00");   /* 가 */

    /* (3) primary final via cycle key, different vowel:
     *     s=ㄴ, k=ㅏ, h=cycle (ㅗ overlays ㄴ) => 난 U+B09C. */
    expect(id, "c3_nan_skh",        "skh", "U+B09C");   /* 난 */

    /* (4) repeat the SAME cycle key -> primary->secondary toggle (도깨비불
     *     stroke increase). t=ㅅ, k=ㅏ, j? no: use u=ㅕ cycle (ㅅ/ㅆ).
     *     d=ㅇ, k=ㅏ, but we need a vowel already; use:
     *     '잇->있': d? no. Use j=ㅓ? cycle keys are y/u/i/h/b/m/n + ';'.
     *     Take w=ㅈ, k=ㅏ? no final cycle on k. Use:
     *     ㅅ-cycle key is 'u'(ㅕ). So 'tu' = ㅅ+ㅕ? that makes 셔. Instead:
     *     primary then secondary on a standalone-built syllable:
     *     d=ㅇ, l=ㅣ, u=cycle(ㅕ->ㅅ primary) => 잇 U+C787, then u again
     *     (ㅅ->ㅆ secondary) => 있 U+C788. */
    expect(id, "c5_it_dlu",         "dlu", "U+C787");   /* 잇  (1차 종성 ㅅ) */
    expect(id, "c6_iss_dluu",       "dluu", "U+C788");  /* 있  (반복 -> 2차 ㅆ) */

    /* (7) standalone jongseong: NO choseong, vowel-only buffer then cycle key.
     *     k=ㅏ (medial alone), h=cycle(ㅗ->ㄴ); choseong==0 so the lone vowel is
     *     dropped and the final ㄴ stands alone. A standalone final renders as the
     *     compatibility jamo => U+3134 (ㄴ). This proves the choseong==0 drop arm
     *     of nabicloud_engine_jongseong_cycle. */
    expect(id, "c7_standalone_kh",  "kh",  "U+3134");   /* ㄴ 단독 종성 (호환자모) */

    /* (8) ';' key cycles ㅈ (primary) / ㅊ (secondary).
     *     d=ㅇ, l=ㅣ, ';' => 잊? ㅇ+ㅣ+ㅈ = 잊 U+C78A. */
    expect(id, "c8_semicolon_j",    "dl;", "U+C78A");   /* 잊  (종성 ㅈ) */
    /* repeat ';' -> ㅈ->ㅊ : 잊 -> 잋 U+C78B. */
    expect(id, "c9_semicolon_ch",   "dl;;","U+C78B");   /* 잋  (종성 ㅊ) */

    /* (10) cluster merge (different cycle keys -> standard 겹받침), and the case
     *     where jungseong AND jongseong are BOTH nonzero in one keystroke:
     *     d=ㅇ, j=ㅓ, n=cycle(ㅜ->ㄹ primary) => 얼 U+C5BC,
     *     then y=cycle(ㅛ->ㄱ): the existing final ㄹ combines with the primary
     *     candidate ㄱ into the cluster ㄺ => 얽 U+C5BD. The stage-2 cluster-merge
     *     arm (combine with primary candidate) is exercised here. */
    expect(id, "c10_eol_djn",       "djn", "U+C5BC");   /* 얼  (종성 ㄹ) */
    expect(id, "c11_eolg_djny",     "djny","U+C5BD");   /* 얽  (ㄹ+ㄱ -> ㄺ) */

    /* (12) Shift cluster key direct: H = 겹받침 ㄶ.
     *     s=ㄴ? no, build ㄴ-base vowel then... H places ㄶ directly only when a
     *     ㄴ final could host it; use simple direct: d=ㅇ, k=ㅏ, then H(ㄶ)
     *     -> 않 U+C54A. */
    expect(id, "c12_anh_dkH",       "dkH", "U+C54A");   /* 않  (겹받침 ㄶ 직접글쇠) */

    /* (13) full two-syllable word exercising carry-over (도깨비불):
     *     r=ㄱ k=ㅏ n(ㄹ) => 갈? n on ㅏ-syllable: ㄱ+ㅏ+ㄹ = 갈 U+AC08,
     *     then r=ㄱ k=ㅏ => 가, ... keep simple: 'rkn' = 갈. */
    expect(id, "c13_gal_rkn",       "rkn", "U+AC08");   /* 갈  (종성 ㄹ) */

    printf("# end fail=%d\n", g_fail);
    return g_fail;
}
