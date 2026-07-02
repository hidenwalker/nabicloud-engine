/*
 * NabiCloud Semo-e (세모이) cross-engine differential harness.
 *
 * Drives the 5 Semo-e keyboards (3moa-semoe 2014/2015/2016/2017/2018)
 * with a broad ASCII key sweep and dumps the per-key commit/preedit plus the
 * final flush as "U+XXXX" tokens. Compiled twice -- once against the NabiCloud
 * engine tree (TYPE_JASO_SEBEOL -> nabicloud_engine_jaso_sebeol_process), once
 * against the 3beol reference tree (TYPE_JASO -> hangul_ic_process_jaso_sebeol,
 * libhangul_3beol defined). The two stdout streams are byte-diffed. mismatch==0
 * means the faithful port matches the 3beol handler for these keyboards.
 *
 * Coverage targets the Semo-e-specific paths:
 *   - plain CV / CVC moeum / jongseong combos (default combination table[0]);
 *   - symbol_semoe EXTENDED expansion: prep keys J/K/L/':' then 1..5 step keys,
 *     covering addon_func[0]=hangul_ascii_to_symbol_semoe + ext_step value table;
 *   - 2016 has addon_value[SYMBOL]=NULL (the others non-NULL) -> exercises both;
 *   - right_ou moeum keys: 2018='.'/'b', 2017='.'/'p', 2016='['/'p',
 *     2014/2015 deprecated '\''/'p';
 *   - galmadeuli 2018 (combination[1]) moachigi vs ieochigi (LOOSE_ORDER on/off);
 *   - backspace incl. extended-mode step restore and overrun.
 *
 * ASCII-only on purpose. No UTF-8 BOM, no CP949 trap.
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hangul.h"

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

static void run_str(const char* kbd, const char* name, const char* s) {
    char cbuf[512], pbuf[512];
    HangulInputContext* hic = hangul_ic_new(kbd);
    if (hic == NULL) { printf("T %s\n  FAIL new==NULL\n", name); return; }
    printf("T %s\n", name);
    for (const char* p = s; *p; ++p) {
        int processed;
        if (*p == '\b') {
            processed = hangul_ic_backspace(hic) ? 1 : 0;
        } else {
            processed = hangul_ic_process(hic, (unsigned char)*p) ? 1 : 0;
        }
        fmt(hangul_ic_get_commit_string(hic), cbuf, sizeof(cbuf));
        fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
        printf("  k=0x%02X p=%d commit=%s pre=%s\n",
               (unsigned)(unsigned char)*p, processed, cbuf, pbuf);
    }
    fmt(hangul_ic_flush(hic), cbuf, sizeof(cbuf));
    printf("  flush=%s\n", cbuf);
    hangul_ic_delete(hic);
}

/* Same ASCII for every keyboard so keymap + handler differences (not the input)
 * drive the output. */
static void suite(const char* kbd) {
    /* basic syllables across hand positions */
    run_str(kbd, "kf",     "kf");
    run_str(kbd, "msf",    "msf");
    run_str(kbd, "rk",     "rk");
    run_str(kbd, "rkt",    "rkt");
    run_str(kbd, "gksrmf", "gksrmf");
    run_str(kbd, "dkssud", "dkssud");
    run_str(kbd, "qhgk",   "qhgk");
    run_str(kbd, "jf",     "jf");
    run_str(kbd, "jfa",    "jfa");
    /* right_ou moeum keys -- semoe per-layout: '.','b','p','[','\'' */
    run_str(kbd, "k_dot",   "k.");
    run_str(kbd, "k_b",     "kb");
    run_str(kbd, "k_p",     "kp");
    run_str(kbd, "k_brk",   "k[");
    run_str(kbd, "k_apos",  "k'");
    run_str(kbd, "k_dotb",  "k.b");
    run_str(kbd, "k_dotp",  "k.p");
    run_str(kbd, "r_dot_f", "r.f");
    run_str(kbd, "r_p_f",   "rpf");
    /* galmadeuli / vowel-after-vowel, jong doubling */
    run_str(kbd, "rhk",    "rhk");
    run_str(kbd, "rkk",    "rkk");
    run_str(kbd, "fff",    "fff");
    run_str(kbd, "rkqq",   "rkqq");
    run_str(kbd, "tkfa",   "tkfa");
    run_str(kbd, "rkrk",   "rkrk");
    run_str(kbd, "vowels", "frtnbg");
    run_str(kbd, "moach",  "rhkrhk");
    /* symbol_semoe EXTENDED: prep keys J / K / L / ':' then step keys 1..5 */
    run_str(kbd, "extJ1",  "J1");
    run_str(kbd, "extJ2",  "J2");
    run_str(kbd, "extJ3",  "J3");
    run_str(kbd, "extJ4",  "J4");
    run_str(kbd, "extJ5",  "J5");
    run_str(kbd, "extK1",  "K1");
    run_str(kbd, "extL2",  "L2");
    run_str(kbd, "extC3",  ":3");
    run_str(kbd, "ext2J1", "2J1");
    run_str(kbd, "extJq",  "Jq");
    run_str(kbd, "extJk",  "Jk");
    run_str(kbd, "extseq", "kfJ2");
    run_str(kbd, "extJ0",  "J0");
    run_str(kbd, "extJ6",  "J6");
    run_str(kbd, "extJJ",  "JJ");
    /* shift / caps mixed */
    run_str(kbd, "kRf",    "kRf");
    run_str(kbd, "RKF",    "RKF");
    /* digits and symbols */
    run_str(kbd, "digits", "0123456789");
    run_str(kbd, "syms",   "!@#$%^&*()");
    run_str(kbd, "punct",  ";',.[]");
    /* full alpha rows */
    run_str(kbd, "lowers", "abcdefghijklmnopqrstuvwxyz");
    run_str(kbd, "uppers", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    /* long mixed words */
    run_str(kbd, "long1",  "mfsrnfdfj");
    run_str(kbd, "long2",  "gksrnfeofgksalsrnr");
    run_str(kbd, "long3",  "rk.f.rh/kqq");
    run_str(kbd, "long4",  "jfJ2kfK1msf");
    /* backspace coverage */
    run_str(kbd, "bs_cv",    "rk\b");
    run_str(kbd, "bs_cvc",   "rkq\b\b");
    run_str(kbd, "bs_ou",    "k.\b");
    run_str(kbd, "bs_ext",   "J2\b");
    run_str(kbd, "bs_extseq","kfJ2\b\b");
    run_str(kbd, "bs_over",  "rk\b\b\b");
    run_str(kbd, "bs_mix",   "rkq\brk\b");
    run_str(kbd, "bs_extmix","J2\brk\b");
}

int main(void) {
    static const char* kbds[] = {
        "3moa-semoe-2014", "3moa-semoe-2015", "3moa-semoe-2016",
        "3moa-semoe-2017", "3moa-semoe-2018"
    };
    printf("# nabicloud semoe differential\n");
    for (unsigned i = 0; i < sizeof(kbds)/sizeof(kbds[0]); ++i) {
        printf("\n=== KBD id=%s ===\n", kbds[i]);
        suite(kbds[i]);
    }
    printf("# end\n");
    return 0;
}
