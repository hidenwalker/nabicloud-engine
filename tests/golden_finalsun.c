/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Old-engine fork-builtin
   (3gs/3-91-noshift) golden; the finalsun fork builtin was dropped by the F-3 R4 pristine
   rebase (2026-06-27); no V2 replacement. */
/*
 * NabiCloud 3finalsun (Sebeolsik 3-91 Final Noshift) differential harness.
 *
 * Purpose: freeze the EXACT observable behavior of the 3finalsun engine
 * (commit/preedit per key, flush, backspace, reset) for a given keyboard id, so
 * the NabiCloud port can be proven to match the 3beol (yous/libhangul,
 * gureum-1.11.1) reference by byte-for-byte diff.
 *
 * It is parameterized by the keyboard id (argv[1], default "3-91-noshift") so the
 * SAME source compiles against BOTH:
 *   - the 3beol upstream clone  -> produces tests/golden_finalsun.txt (reference)
 *   - the NabiCloud engine tree -> compared against that reference
 *
 * The reference is generated from the upstream clone (where 3-91-noshift is type
 * HANGUL_KEYBOARD_TYPE_3FINALSUN with the built-in hangul_ic_process_3finalsun
 * handler). Our port reproduces the same output via type FINALSUN(1010) ->
 * nabicloud_engine_finalsun_process.
 *
 * The 3finalsun trick is the 종성시프트 (jong-shift) key '['. The sequences below
 * exercise it heavily: forming a cluster jongseong (겹받침) on a syllable with no
 * final, replacing an existing final with a cluster, parking the sentinel and
 * combining it with the next jamo, plus the ordinary choseong-double / jungseong-
 * merge / jongseong-cluster paths shared with the standard jaso automaton.
 *
 * 3-91-noshift key map (relevant keys, ascii -> jamo):
 *   choseong : k(0x6B)=ㄱ h(0x68)=ㄴ u(0x75)=ㄷ y(0x79)=ㄹ i(0x69)=ㅁ
 *              ;(0x3B)=ㅂ n(0x6E)=ㅅ j(0x6A)=ㅇ l(0x6C)=ㅈ o(0x6F)=ㅊ
 *              0(0x30)=ㅋ p(0x70)=ㅍ m(0x6D)=ㅎ '(0x27)=ㅌ
 *   jungseong: f(0x66)=ㅏ r(0x72)=ㅐ t(0x74)=ㅓ c(0x63)=ㅔ v(0x76)=ㅗ
 *              b(0x62)=ㅜ g(0x67)=ㅡ d(0x64)=ㅣ /(0x2F)=ㅚ 9(0x39)=ㅟ e(0x65)=ㅕ
 *   jongseong: a(0x61)=ㅇ x(0x78)=ㄱ s(0x73)=ㄴ w(0x77)=ㄹ z(0x7A)=ㅁ
 *              q(0x71)=ㅅ A(0x41)=ㄷ ...; '[' = 종성시프트 (SHKEY)
 *
 * The test bodies are ASCII-only (key strings are bare ASCII); the header has a
 * few Korean labels in comments for readability. No UTF-8 BOM (matches
 * golden_galmadeuli.c); both the 3beol clone and our tree compile it cleanly
 * (only non-fatal C4819 notes on the comment bytes -> FINALSUN_PASS is the proof).
 */
#include <stdio.h>
#include <string.h>
#include "hangul.h"
#include "baram.h"   /* Stage D-1: nabicloud_set_baram_enabled (old/new mode) */

static const char* g_kbd = "3-91-noshift";

/* FINALSUN functional gate (re-baselined after the Sandeulbaram integration,
 * 2026-06-21).
 *
 * The FINALSUN_PASS gate proves the NabiCloud port
 * (nabicloud_engine_finalsun_process) == the 3beol reference, by diffing our
 * tree's output against the FROZEN baseline tests\golden_finalsun.txt.
 *
 * The former Stage D-1 old/new equivalence axis (baram_process_finalsun vs
 * nabicloud_engine_finalsun_process) was RETIRED by the Sandeulbaram integration
 * (0bfdfd6): the parallel BaramPolicy automata (baram_process_* / baram_dispatch)
 * are gone, and FINALSUN now routes through the single baram ops path
 * (nabicloud_baram_lookup -> baram_finalsun_process ->
 * nabicloud_engine_finalsun_process). The master gate (Sandeulbaram) always runs
 * ON, and this gate proves that output == the frozen golden_finalsun.txt
 * (== the 3beol reference), byte + NFC.
 *
 * All input flows through the production entry hangul_ic_process ->
 * nabicloud_dispatch. g_new is a harmless residual (unused for routing;
 * (void)g_new in new_ic()). Backspace (run_backspace) also flows through the
 * production hangul_ic_backspace -> nabicloud_dispatch_backspace; finalsun has NO
 * dedicated backspace hook (the parked buffer.shift is preserved by the shared
 * hangul_buffer_backspace), so its backspace equivalence is exercised too. */
static int g_new = 0;

static HangulInputContext* new_ic(void) {
    /* 2026-06-21 통합: 산들바람(전 NabiCloud 엔진) 항상 ON. 옛 old/new(ops vs baram)
     * 등가축은 baram 병렬 impl 은퇴와 함께 폐기 — 이제 finalsun 은 ops 단일 경로다.
     * 기능 게이트(ops finalsun 출력 == frozen golden_finalsun.txt == 3beol 레퍼런스)는
     * 그대로 유지된다. g_new 는 무해한 잔존(라우팅에 미사용). */
    (void)g_new;
    nabicloud_set_baram_enabled(true);
    return hangul_ic_new(g_kbd);
}

static void fmt(const ucschar* s, char* out, size_t cap) {
    size_t n = 0;
    if (s == NULL) { snprintf(out, cap, "(null)"); return; }
    if (*s == 0)   { snprintf(out, cap, "(empty)"); return; }
    out[0] = '\0';
    while (*s && n + 8 < cap) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%sU+%04X", (n ? " " : ""), (unsigned)*s);
        strncat(out, tmp, cap - strlen(out) - 1);
        n += strlen(tmp);
        ++s;
    }
}

static void run_str(const char* name, const char* s) {
    char cbuf[512], pbuf[512];
    HangulInputContext* hic = new_ic();
    if (hic == NULL) { printf("T %s\n  FAIL new==NULL\n", name); return; }
    printf("T %s\n", name);
    for (const char* p = s; *p; ++p) {
        int processed = hangul_ic_process(hic, (unsigned char)*p) ? 1 : 0;
        fmt(hangul_ic_get_commit_string(hic), cbuf, sizeof(cbuf));
        fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
        printf("  k=0x%02X p=%d commit=%s pre=%s\n",
               (unsigned)(unsigned char)*p, processed, cbuf, pbuf);
    }
    fmt(hangul_ic_flush(hic), cbuf, sizeof(cbuf));
    printf("  flush=%s\n", cbuf);
    hangul_ic_delete(hic);
}

static void run_backspace(const char* name, const char* s) {
    char pbuf[512];
    HangulInputContext* hic = new_ic();
    if (hic == NULL) { printf("T %s\n  FAIL new==NULL\n", name); return; }
    printf("T %s\n", name);
    for (const char* p = s; *p; ++p) hangul_ic_process(hic, (unsigned char)*p);
    fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
    printf("  built pre=%s\n", pbuf);
    for (int i = 0; i < 12; ++i) {
        int ok = hangul_ic_backspace(hic) ? 1 : 0;
        fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
        printf("  bs r=%d pre=%s\n", ok, pbuf);
        if (hangul_ic_is_empty(hic)) break;
    }
    hangul_ic_delete(hic);
}

static void run_reset(const char* name, const char* a, const char* b) {
    char cbuf[512], pbuf[512];
    HangulInputContext* hic = new_ic();
    if (hic == NULL) { printf("T %s\n  FAIL new==NULL\n", name); return; }
    printf("T %s\n", name);
    for (const char* p = a; *p; ++p) hangul_ic_process(hic, (unsigned char)*p);
    hangul_ic_reset(hic);
    fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
    printf("  afterreset pre=%s empty=%d\n", pbuf, hangul_ic_is_empty(hic) ? 1 : 0);
    for (const char* p = b; *p; ++p) hangul_ic_process(hic, (unsigned char)*p);
    fmt(hangul_ic_flush(hic), cbuf, sizeof(cbuf));
    printf("  flush=%s\n", cbuf);
    hangul_ic_delete(hic);
}

/* --------------------------------------------------------------------------
 * Stage D-1 ADDITIVE token: NFC / accumulated-syllable equivalence (old vs new).
 *
 * Mirrors golden_baram.c's NFC oracle. The byte gate (FINALSUN_PASS old-vs-new)
 * proves old==new at the RAW jamo/accessor level; this layers a higher-level
 * oracle on top: accumulate commit+flush into one jamo run, fold to NFC via the
 * PUBLIC hangul_jamos_to_syllables (NO re-implemented normalizer), and compare
 * old vs new at the user-visible syllable level. Crucial for finalsun: the SHKEY
 * cluster park/unpark commits a final early or late depending on the path, and
 * NFC folds those timing differences into the same syllable -- catching any
 * divergence the raw-jamo gate could mask. Emitted under its own "## nfc"
 * section so the legacy byte dump (golden_finalsun.txt) is untouched (separate
 * invocation, separate files). */
static void nfc_collect(const char* keys, char* out, size_t cap) {
    ucschar acc[256], syl[256];
    int n = 0;
    HangulInputContext* hic = new_ic();
    if (hic == NULL) { snprintf(out, cap, "IC_NULL"); return; }
    for (const char* p = keys; *p; ++p) {
        const ucschar* commit;
        hangul_ic_process(hic, (unsigned char)*p);
        commit = hangul_ic_get_commit_string(hic);
        for (; commit && *commit && n < 255; ++commit) acc[n++] = *commit;
    }
    {
        const ucschar* flush = hangul_ic_flush(hic);
        for (; flush && *flush && n < 255; ++flush) acc[n++] = *flush;
    }
    hangul_ic_delete(hic);

    {
        int m = hangul_jamos_to_syllables(syl, 256, acc, n);
        if (m < 0) m = 0;
        if (m == 0) { snprintf(out, cap, "(empty)"); return; }
        out[0] = '\0';
        for (int i = 0; i < m; ++i) {
            char tmp[16];
            snprintf(tmp, sizeof(tmp), "%sU+%04X", (i ? " " : ""), (unsigned)syl[i]);
            strncat(out, tmp, cap - strlen(out) - 1);
        }
    }
}

static void run_nfc_case(const char* name, const char* keys) {
    char got[512];
    nfc_collect(keys, got, sizeof(got));
    printf("N %-14s keys=%-10s nfc=%s\n", name, keys, got);
}

/* NFC suite: the SHKEY park/unpark + cluster shapes that the raw dump also
 * covers, so the section meaningfully exercises every buffer.shift transition. */
static void run_nfc_suite(void) {
    printf("## nfc\n");
    run_nfc_case("ga",        "kf");
    run_nfc_case("gan",       "kfs");
    run_nfc_case("nun",       "hbs");
    run_nfc_case("dal",       "ufw");
    run_nfc_case("dak_sh_i",  "uf[i");   /* SHKEY park on final-less syllable    */
    run_nfc_case("ga_sh_d",   "kf[d");   /* park then jungseong                  */
    run_nfc_case("ga_sh_f",   "kf[f");
    run_nfc_case("ga_sh_v",   "kf[v");
    run_nfc_case("gak_sh",    "kfx[");   /* SHKEY over existing final            */
    run_nfc_case("gan_sh",    "kfs[");
    run_nfc_case("gal_sh",    "kfw[");
    run_nfc_case("ga_sh_x",   "kf[x");   /* park then jongseong key              */
    run_nfc_case("ga_sh_q",   "kf[q");
    run_nfc_case("ga_sh_a",   "kf[a");
    run_nfc_case("empty_sh",  "[");      /* '[' alone -> '(' literal             */
    run_nfc_case("sh_first",  "[kf");
    run_nfc_case("kkf",       "kkf");
    run_nfc_case("goe",       "k/");
    run_nfc_case("gwi",       "k9");
    run_nfc_case("gaks",      "kfxq");
    run_nfc_case("ganj",      "kfsl");
    run_nfc_case("hangul",    "kfshbw");
}

int main(int argc, char** argv) {
    /* argv[1] = keyboard id (default "3-91-noshift"),
     * argv[2] = mode ("old"|"new", default old),
     * argv[3] = "nfc" to emit the additive NFC section instead of the raw dump. */
    int nfc_mode = 0;
    if (argc > 1) g_kbd = argv[1];
    if (argc > 2 && strcmp(argv[2], "new") == 0) g_new = 1;
    if (argc > 3 && strcmp(argv[3], "nfc") == 0) nfc_mode = 1;
    /* NOTE: the banner omits the mode so old/new dumps are byte-identical when
     * the engines agree -- only the per-key routing flag differs. The legacy
     * "old" dump still reproduces the frozen golden_finalsun.txt byte-for-byte. */
    printf("# libhangul 3finalsun golden v1 kbd=%s (do not edit by hand)\n", g_kbd);

    if (nfc_mode) {
        {
            HangulInputContext* hic = new_ic();
            printf("## new_ok=%d\n", hic != NULL ? 1 : 0);
            if (hic) hangul_ic_delete(hic);
        }
        run_nfc_suite();
        printf("# end\n");
        return 0;
    }

    /* sanity: new must succeed for this keyboard id */
    {
        HangulInputContext* hic = new_ic();
        printf("## new_ok=%d\n", hic != NULL ? 1 : 0);
        if (hic) hangul_ic_delete(hic);
    }

    /* 1) all printable singles (keymap sanity across the whole table) */
    printf("## singles\n");
    for (int c = 0x21; c <= 0x7e; ++c) {
        char nm[16], s[2]; s[0] = (char)c; s[1] = 0;
        snprintf(nm, sizeof(nm), "s_%02X", c);
        run_str(nm, s);
    }

    /* 2) plain syllables (choseong + jungseong [+ jongseong]) */
    printf("## plain\n");
    run_str("ga",   "kf");    /* ㄱ + ㅏ          = 가              */
    run_str("gan",  "kfs");   /* ㄱ + ㅏ + ㄴ      = 간              */
    run_str("nun",  "hbs");   /* ㄴ + ㅜ + ㄴ      = 눈              */
    run_str("dal",  "ufw");   /* ㄷ + ㅏ + ㄹ      = 달              */

    /* 3) 종성시프트 '[' : form a CLUSTER jongseong on a final-less syllable
     *    SHKEY parks, then next jamo forms the 겹받침.                       */
    printf("## shift_park\n");
    run_str("dak_shift_g", "uf[i");  /* 다 + '[' + ㅁ(i=choseong ㅁ)         */
    run_str("ga_shift_i",  "kf[d");  /* 가 + '[' + ㅣ(d=jungseong) -> ㄻ?    */
    run_str("ga_shift_f",  "kf[f");  /* 가 + '[' + ㅏ                        */
    run_str("ga_shift_v",  "kf[v");  /* 가 + '[' + ㅗ                        */

    /* 4) 종성시프트 over an EXISTING final: replace final with cluster        */
    printf("## shift_replace\n");
    run_str("gak_shift", "kfx[");    /* 각(final ㄱ) + '[' -> cluster?        */
    run_str("gan_shift", "kfs[");    /* 간(final ㄴ) + '[' -> cluster?        */
    run_str("gal_shift", "kfw[");    /* 갈(final ㄹ) + '[' -> cluster?        */

    /* 5) 종성시프트 then a jongseong key (shift held, final becomes cluster)  */
    printf("## shift_then_jong\n");
    run_str("ga_sh_x",  "kf[x");     /* 가 + '[' + ㄱ(x=jong)                 */
    run_str("ga_sh_q",  "kf[q");     /* 가 + '[' + ㅅ(q=jong)                 */
    run_str("ga_sh_a",  "kf[a");     /* 가 + '[' + ㅇ(a=jong)                 */

    /* 6) shift on EMPTY buffer (no-op: '[' maps to '(' literal)              */
    printf("## shift_empty\n");
    run_str("empty_shift", "[");     /* '[' alone -> '(' symbol               */
    run_str("shift_first", "[kf");   /* '[' then 가                            */

    /* 7) choseong doubling (combination[0]) */
    printf("## cho_double\n");
    run_str("kkf", "kkf");           /* ㄱ + ㄱ -> ㄲ + ㅏ = 까               */
    run_str("llf", "llf");           /* ㅈ + ㅈ -> ㅉ + ㅏ                    */

    /* 8) jungseong merge (compound vowels via combination[0]) */
    printf("## vowels\n");
    run_str("gwa",  "kv/f");         /* not standard; exercises oe-merge      */
    run_str("goe",  "k/");           /* ㄱ + ㅚ                               */
    run_str("gwi",  "k9");           /* ㄱ + ㅟ                               */

    /* 9) jongseong cluster directly (autogen clusters, no shift) */
    printf("## jong_cluster\n");
    run_str("gaks", "kfxq");         /* 각 + ㅅ -> ㄳ cluster?                 */
    run_str("ganj", "kfsl");         /* 간 + ㅈ jong path                     */

    /* 10) ordinary words */
    printf("## words\n");
    run_str("hangul", "kfshbw");     /* arbitrary multi-jamo                  */
    run_str("digits", "0123456789");
    run_str("symbols", "!@#$%^&*()");

    /* 11) backspace / reset snapshots (incl. shift state) */
    printf("## backspace\n");
    run_backspace("bs_gan",      "kfs");
    run_backspace("bs_shiftgg",  "uf[i");
    run_backspace("bs_gakshift", "kfx[");

    printf("## reset\n");
    run_reset("rs_1", "kfs", "hf");
    run_reset("rs_2", "kf[i", "hf");

    printf("# end\n");
    return 0;
}
