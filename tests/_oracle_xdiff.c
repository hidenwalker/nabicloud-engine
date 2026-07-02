/* _oracle_xdiff.c  -- scratch: drive the FAITHFUL PORT (3-91-noshift) over the
 * cross-differential probe cases and print the accumulated commit+flush stream
 * (the same quantity the cleanroom xdiff.c accumulates), so the port can be used
 * as the O oracle to arbitrate the orphan-syllable gap-decisions (a)(b)(c).
 * Build like build_and_verify_finalsun.bat (hangul/*.c except hanja.c +
 * nabicloud/ **.c). Not a committed gate -- scratch verifier artifact. */
#include <stdio.h>
#include <string.h>
#include "hangul.h"
#include "baram.h"

static const char *g_kbd = "3-91-noshift";

static void run(const char *keys)
{
    ucschar acc[256];
    int n = 0, i;
    const char *p;
    HangulInputContext *hic;
    nabicloud_set_baram_enabled(true);
    hic = hangul_ic_new(g_kbd);
    if (hic == NULL) { printf("%-8s IC_NULL\n", keys); return; }
    for (p = keys; *p; ++p) {
        const ucschar *c;
        hangul_ic_process(hic, (unsigned char)*p);
        c = hangul_ic_get_commit_string(hic);
        for (; c && *c && n < 255; ++c) acc[n++] = *c;
    }
    {
        const ucschar *f = hangul_ic_flush(hic);
        for (; f && *f && n < 255; ++f) acc[n++] = *f;
    }
    hangul_ic_delete(hic);
    printf("%-8s ->", keys);
    if (n == 0) printf(" (empty)");
    for (i = 0; i < n; ++i) printf(" U+%04X", (unsigned)acc[i]);
    printf("\n");
}

int main(void)
{
    static const char *cases[] = {
        /* 24 anchors */
        "kf","kfs","hbs","ufw","uf[i","kf[d","kf[f","kf[v","kfx[","kfs[",
        "kfw[","kf[x","kf[q","kf[a","[","[kf","kkf","llf","k/","k9",
        "kv/f","kfxq","kfsl","kfshbw",
        /* the 4 divergence-pattern examples */
        "f","fk","fa","ak",
        /* (a) orphan-vowel variants */
        "v","/","9","kff","kvf","k/f",
        /* (b) vowel-then-initial */
        "fl","vk","fh",
        /* (c) orphan-final variants */
        "fx","xf","xk","aq","af","fq",
        /* combos */
        "kfa","kf/f","kf[",
        0
    };
    int i;
    printf("# oracle (faithful port 3-91-noshift) : accumulated commit+flush\n");
    for (i = 0; cases[i]; ++i) run(cases[i]);
    printf("# end\n");
    return 0;
}
