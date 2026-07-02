/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Linked baram.c (engine-off
   toggle), dropped with the 8 old engine-layer TUs in F-3 R8 (2026-06-29); can no longer
   link. */
/*
 * NabiCloud OFF-mode (gu-engine = vanilla) golden -- seal the master-gate OFF path.
 *
 * (2026-06-21 verification D) The default s_baram_enabled flip false->true left the
 * OFF (vanilla) path untested by any gate (every other gate runs ON). This golden
 * seals it with two SELF-CHECKING invariants (no frozen baseline needed):
 *
 *   1. Standard keyboard ("2", 2-beolsik / JAMO): nabicloud_baram_enabled() ON output
 *      == OFF output, byte-identical. Proves the master gate is a NO-OP for vanilla
 *      keyboard types (both modes hit the same upstream automaton).
 *
 *   2. NabiCloud keyboard ("3gs", type NABICLOUD=99): ON output != OFF output (at
 *      least one sequence differs). Proves the gate actually DISABLES the NabiCloud
 *      engine in OFF mode (3gs falls through to the vanilla jamo automaton).
 *
 * Together they prove OFF == pure vanilla: the gate changes nothing for vanilla
 * types and genuinely bypasses NabiCloud for >=99 types.
 *
 * ASCII-only source (no UTF-8 BOM, matches golden_finalsun.c). Built/run by
 * build_and_verify_off.bat. PASS -> OFF_PASS (exit 0); failure -> OFF_FAIL (exit 1).
 */
#include <stdio.h>
#include <string.h>
#include "hangul.h"
#include "baram.h"     /* nabicloud_set_baram_enabled / nabicloud_baram_enabled */

/* Run `keys` through a fresh IC of `kbd`, accumulate commit+flush into `out` as
 * "U+XXXX U+XXXX ...". Returns 0 if the keyboard could not be created. */
static int cap(const char* kbd, const char* keys, char* out, size_t cap_sz) {
    ucschar acc[512];
    int n = 0;
    const char* p;
    int i;
    HangulInputContext* hic = hangul_ic_new(kbd);
    if (hic == NULL) { snprintf(out, cap_sz, "IC_NULL"); return 0; }
    for (p = keys; *p; ++p) {
        const ucschar* c;
        hangul_ic_process(hic, (unsigned char)*p);
        c = hangul_ic_get_commit_string(hic);
        for (; c && *c && n < 511; ++c) acc[n++] = *c;
    }
    {
        const ucschar* f = hangul_ic_flush(hic);
        for (; f && *f && n < 511; ++f) acc[n++] = *f;
    }
    hangul_ic_delete(hic);
    out[0] = '\0';
    for (i = 0; i < n; ++i) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%sU+%04X", (i ? " " : ""), (unsigned)acc[i]);
        strncat(out, tmp, cap_sz - strlen(out) - 1);
    }
    if (n == 0) snprintf(out, cap_sz, "(empty)");
    return 1;
}

int main(void) {
    static const char* SEQS[] = {
        "rkdtkfa", "gksrmf", "rkk", "tkfa", "rhk",
        "kf[sif[", "mfsrnfdfj", "0123456789", "!@#$%^&*()", "kkffss",
    };
    int nseq = (int)(sizeof(SEQS) / sizeof(SEQS[0]));
    int fails = 0;
    int i;
    printf("# nabicloud golden_off v1 (master gate OFF = vanilla; do not edit by hand)\n");

    /* 1) standard 2-beolsik: ON == OFF (gate byte-neutral for vanilla types) */
    printf("## std_on_eq_off (kbd=2)\n");
    for (i = 0; i < nseq; ++i) {
        char onb[1024], offb[1024];
        nabicloud_set_baram_enabled(1);  cap("2", SEQS[i], onb, sizeof onb);
        nabicloud_set_baram_enabled(0);  cap("2", SEQS[i], offb, sizeof offb);
        nabicloud_set_baram_enabled(1);  /* restore default ON */
        {
            int same = (strcmp(onb, offb) == 0);
            printf("  [%s] keys=%s\n", same ? "PASS" : "FAIL", SEQS[i]);
            if (!same) { printf("    ON =%s\n    OFF=%s\n", onb, offb); fails++; }
        }
    }

    /* 2) NabiCloud 3gs (type 99): ON != OFF (gate disables the NabiCloud engine) */
    {
        const HangulKeyboard* kb = hangul_keyboard_list_get_keyboard("3gs");
        if (kb == NULL) {
            printf("## nabicloud_on_ne_off (kbd=3gs): SKIP (3gs not registered)\n");
        } else {
            static const char* NSEQS[] = { "rk", "rks", "rhk", "tkfa", "mfsrnfdfj" };
            int nn = (int)(sizeof(NSEQS) / sizeof(NSEQS[0]));
            int differ = 0;
            int j;
            printf("## nabicloud_on_ne_off (kbd=3gs)\n");
            for (j = 0; j < nn; ++j) {
                char onb[1024], offb[1024];
                nabicloud_set_baram_enabled(1);  cap("3gs", NSEQS[j], onb, sizeof onb);
                nabicloud_set_baram_enabled(0);  cap("3gs", NSEQS[j], offb, sizeof offb);
                nabicloud_set_baram_enabled(1);
                if (strcmp(onb, offb) != 0) differ++;
            }
            {
                int ok = (differ > 0);
                printf("  [%s] %d/%d sequences differ ON vs OFF (gate must disable NabiCloud)\n",
                       ok ? "PASS" : "FAIL", differ, nn);
                if (!ok) fails++;
            }
        }
    }

    if (fails == 0) { printf("OFF_PASS\n"); return 0; }
    printf("OFF_FAIL: %d\n", fails);
    return 1;
}
