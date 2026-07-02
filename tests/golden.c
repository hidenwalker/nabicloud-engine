/*
 * libhangul-gureum 3gs golden / characterization harness.
 *
 * Purpose: freeze the EXACT observable behavior of the gureum "3gs" engine
 * (commit/preedit per key, flush, backspace, reset, mode-key combinations)
 * so that the nabicloud/libhangul restructure can be proven behavior-preserving
 * by byte-for-byte diff of this program's stdout against tests/golden.txt.
 *
 * This is a CHARACTERIZATION test: it does not assert "correct" Hangul, only
 * INVARIANCE. Any behavior change in any covered path shows up as a diff.
 *
 * Build (engine 3 files only; hanja.c not needed):
 *   cl /nologo /DENABLE_EXTERNAL_KEYBOARDS=1 /D_CRT_SECURE_NO_WARNINGS
 *      /D_CRT_NONSTDC_NO_DEPRECATE /I"hangul" /Fe:tests\golden.exe /Fo:tests\
 *      tests\golden.c hangul\hangulinputcontext.c hangul\hangulctype.c
 *      hangul\hangulkeyboard.c
 *
 * ASCII-only on purpose (no UTF-8 BOM dependency, no CP949/C4819 trap).
 */
#include <stdio.h>
#include <string.h>
#include "hangul.h"

/* Render a ucschar string as "U+XXXX U+YYYY" / "(empty)" / "(null)". */
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

/* Run one key sequence start-to-flush, dumping commit+preedit per key. */
static void run_seq(const char* name, const unsigned char* keys, size_t len) {
    char cbuf[512], pbuf[512];
    HangulInputContext* hic = hangul_ic_new("3gs");
    if (hic == NULL) { printf("T %s\n  FAIL new==NULL\n", name); return; }
    printf("T %s\n", name);
    for (size_t i = 0; i < len; ++i) {
        int processed = hangul_ic_process(hic, (int)keys[i]) ? 1 : 0;
        fmt(hangul_ic_get_commit_string(hic), cbuf, sizeof(cbuf));
        fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
        printf("  k=0x%02X p=%d commit=%s pre=%s\n",
               (unsigned)keys[i], processed, cbuf, pbuf);
    }
    fmt(hangul_ic_flush(hic), cbuf, sizeof(cbuf));
    printf("  flush=%s\n", cbuf);
    hangul_ic_delete(hic);
}

static void run_str(const char* name, const char* s) {
    run_seq(name, (const unsigned char*)s, strlen(s));
}

/* Build a sequence, then backspace to empty, dumping preedit each step. */
static void run_backspace(const char* name, const char* s) {
    char pbuf[512];
    HangulInputContext* hic = hangul_ic_new("3gs");
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

/* Build partial, flush mid-composition, continue, flush again. */
static void run_flush_mid(const char* name, const char* a, const char* b) {
    char cbuf[512], pbuf[512];
    HangulInputContext* hic = hangul_ic_new("3gs");
    if (hic == NULL) { printf("T %s\n  FAIL new==NULL\n", name); return; }
    printf("T %s\n", name);
    for (const char* p = a; *p; ++p) hangul_ic_process(hic, (unsigned char)*p);
    fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
    printf("  partA pre=%s\n", pbuf);
    fmt(hangul_ic_flush(hic), cbuf, sizeof(cbuf));
    printf("  flush1=%s\n", cbuf);
    for (const char* p = b; *p; ++p) hangul_ic_process(hic, (unsigned char)*p);
    fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
    printf("  partB pre=%s\n", pbuf);
    fmt(hangul_ic_flush(hic), cbuf, sizeof(cbuf));
    printf("  flush2=%s\n", cbuf);
    hangul_ic_delete(hic);
}

/* Build, reset, verify empty, continue. */
static void run_reset(const char* name, const char* a, const char* b) {
    char cbuf[512], pbuf[512];
    HangulInputContext* hic = hangul_ic_new("3gs");
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

int main(void) {
    printf("# libhangul-gureum 3gs golden v1 (characterization, do not edit by hand)\n");

    /* 1) all printable singles */
    printf("## singles\n");
    for (int c = 0x21; c <= 0x7e; ++c) {
        char nm[16]; unsigned char k = (unsigned char)c;
        snprintf(nm, sizeof(nm), "s_%02X", c);
        run_seq(nm, &k, 1);
    }

    /* 2) all printable pairs */
    printf("## pairs\n");
    for (int a = 0x21; a <= 0x7e; ++a) {
        for (int b = 0x21; b <= 0x7e; ++b) {
            char nm[16]; unsigned char k[2];
            k[0] = (unsigned char)a; k[1] = (unsigned char)b;
            snprintf(nm, sizeof(nm), "p_%02X_%02X", a, b);
            run_seq(nm, k, 2);
        }
    }

    /* 3) curated multi-key sequences.
       3gs key map (lowercase): k=g j=ng-cho(0x110b) i=m h=n y=r u=d l=j m=h n=s o=ch p=p
       vowels: f=A(0x1161) t=eo r=ae(0x1162) c=e e=yeo d=i g=eu v=o b=u
       jong: x=g s=n w=r z=m a=ng q=s ;... combine key '[' (0x5b), compound-vowel key '/' (0xe06c) */
    printf("## curated\n");
    run_str("many",   "ifs[");      /* m a n + combine -> nh jong (manh) */
    run_str("eonj",   "jts[");      /* documented: eonj cluster */
    run_str("salm",   "nfzm");      /* s a m ... lm cluster path */
    run_str("salm2",  "nfwz");      /* s a r m -> rm jong */
    run_str("gwa",    "kf/");       /* g a + compound -> gwa */
    run_str("gwae",   "kr/");       /* g ae + compound -> gwae */
    run_str("repeatA","ff");        /* single-vowel repeat (display-bug path) */
    run_str("repeatA3","fff");
    run_str("repeatO","vv");
    run_str("modekey_only","[");
    run_str("modekey_mid","kf[");
    run_str("compound_vowels","f/t/r/");
    run_str("word_hangul","mfsrnfdfj"); /* arbitrary multi-syllable */
    run_str("word2","kfsifnfjlj");
    run_str("digits","0123456789");
    run_str("symbols","!@#$%^&*()");
    run_str("mixed","kf3if9");
    run_str("doublecons","kkffss");
    run_str("nh_jong","jfs[");
    run_str("lm_jong","jfwz");
    run_str("ks_jong","jfx[");
    run_str("all_vowels","ftrcedgvb");

    /* 4) backspace snapshot restore */
    printf("## backspace\n");
    run_backspace("bs_many",   "ifs[");
    run_backspace("bs_gwae",   "kr/");
    run_backspace("bs_word",   "kfsifnf");
    run_backspace("bs_repeat", "fff");
    run_backspace("bs_compound","kf/");
    run_backspace("bs_cluster","jfwz");

    /* 5) flush mid-composition (internal pointer behavior) */
    printf("## flushmid\n");
    run_flush_mid("fm_1", "kf", "sif");
    run_flush_mid("fm_2", "if", "s[");
    run_flush_mid("fm_3", "f",  "f");

    /* 6) reset */
    printf("## reset\n");
    run_reset("rs_1", "kfs", "if");
    run_reset("rs_2", "kr/", "nf");

    printf("# end\n");
    return 0;
}
