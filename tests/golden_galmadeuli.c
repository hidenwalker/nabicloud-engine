/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Old-engine fork-builtin
   (2noshift/2n9256) golden; the fork builtins were removed by the F-3 R4 pristine libhangul
   0.2.0 rebase (2026-06-27), so the pinned output is unreproducible. */
/*
 * NabiCloud galmadeuli (Dubeolsik Noshift / North-9256) differential harness.
 *
 * Purpose: freeze the EXACT observable behavior of the galmadeuli engine
 * (commit/preedit per key, flush, backspace, reset) for a given keyboard id, so
 * the NabiCloud port can be proven to match the 3beol (yous/libhangul,
 * gureum-1.11.1) reference by byte-for-byte diff.
 *
 * It is parameterized by the keyboard id (argv[1], default "2noshift") so the
 * SAME source compiles against BOTH:
 *   - the 3beol upstream clone  -> produces tests/golden_galmadeuli.txt (reference)
 *   - the NabiCloud engine tree -> compared against that reference
 *
 * The reference is generated from the upstream clone (where the keyboards are
 * type JAMO with the galmadeuli combination at index 2 + the built-in
 * hangul_ic_process_jamo_dubeol handler). Our port reproduces the same output
 * via type GALMADEULI(1001) -> nabicloud_engine_galmadeuli_process.
 *
 * ASCII-only on purpose (no UTF-8 BOM dependency, no CP949/C4819 trap).
 */
#include <stdio.h>
#include <string.h>
#include "hangul.h"

static const char* g_kbd = "2noshift";

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
    HangulInputContext* hic = hangul_ic_new(g_kbd);
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
    HangulInputContext* hic = hangul_ic_new(g_kbd);
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
    HangulInputContext* hic = hangul_ic_new(g_kbd);
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

int main(int argc, char** argv) {
    if (argc > 1) g_kbd = argv[1];
    printf("# libhangul galmadeuli golden v1 kbd=%s (do not edit by hand)\n", g_kbd);

    /* sanity: new must succeed for this keyboard id */
    {
        HangulInputContext* hic = hangul_ic_new(g_kbd);
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

    /* 2) galmadeuli replace rotation: choseong + jungseong + (same)jungseong
     * -> tense choseong. dubeolsik keys: g=r d=e b=q s=t j=w; a=k eo=j */
    printf("## replace_fire\n");
    run_str("kka", "rkk");   /* g + a + a   -> kka (ssangkiyeok)  */
    run_str("tteo", "ejj");  /* d + eo + eo -> tteo (ssangtikeut) */
    run_str("ppa", "qkk");   /* b + a + a   -> ppa (ssangpieup)   */
    run_str("ssa", "tkk");   /* s + a + a   -> ssa (ssangsios)    */
    run_str("jjeo", "wjj");  /* j + eo + eo -> jjeo (ssangcieuc)  */
    run_str("kkae", "roo");  /* g + ae + ae -> ? (ae=o key)       */

    /* 3) replace NON-fire controls */
    printf("## replace_nofire\n");
    run_str("ka_keo", "rkj");   /* g + a + eo: different jungseong -> no replace */
    run_str("aa_nocho", "kk");  /* a + a: no choseong -> no replace path        */
    run_str("kkk", "rkkk");     /* g + a + a + a: third jungseong after replace */

    /* 4) jongseong cluster split / move (the jongseong + jungseong path) */
    printf("## cluster\n");
    run_str("salm", "tkfa");    /* s a l m  -> salm-ish cluster path  */
    run_str("dak_a", "ekrk");   /* d a g a  -> jongseong->choseong move */
    run_str("anj", "kswk");     /* a + n + j cluster path             */

    /* 5) ordinary words (must equal standard jamo for non-replace paths) */
    printf("## words\n");
    run_str("hangul", "gksrmf");/* arbitrary multi-jamo word          */
    run_str("nara", "skfk");    /* n a r a                            */
    run_str("digits", "0123456789");
    run_str("symbols", "!@#$%^&*()");

    /* 6) compound vowels (combination[0] jungseong merges) */
    printf("## vowels\n");
    run_str("gwa", "rhk");      /* g + o + a -> gwa (o+a=wa)          */
    run_str("gwo", "rnj");      /* g + u + eo -> gwo (u+eo=weo)? u=n key */
    run_str("yae", "rml");      /* ya + i -> yae (default_2 only)     */

    /* 7) backspace / reset snapshots */
    printf("## backspace\n");
    run_backspace("bs_kka",  "rkk");
    run_backspace("bs_salm", "tkfa");
    run_backspace("bs_gwa",  "rhk");

    printf("## reset\n");
    run_reset("rs_1", "rkk", "sk");
    run_reset("rs_2", "rhk", "ej");

    printf("# end\n");
    return 0;
}
