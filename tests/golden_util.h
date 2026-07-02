/*
 * NabiCloud golden-gate shared formatter (Stage-B, 2026-06-20).
 *
 * Single home for the ucschar -> "U+XXXX U+YYYY" / "(empty)" / "(null)" renderer
 * that golden_all.c, golden_baram.c and golden_3gs_dual.c each carried as a
 * verbatim copy. (golden_318na.c uses a different collect_text renderer and
 * intentionally stays independent.) Extracting JUST this one helper removes the
 * copy-paste drift risk without forcing the (deliberately per-file) suite
 * structure, expectation tables, or main() to be shared -- those stay local to
 * each gate on purpose (see tests/README.md, over-engineering guard).
 *
 * ASCII-only on purpose (no UTF-8 BOM dependency, no CP949/C4819 trap), matching
 * every golden_*.c that includes it. Header-only static so each translation unit
 * gets its own copy with no extra .c in the auto-globbed source set.
 */
#ifndef NABICLOUD_GOLDEN_UTIL_H
#define NABICLOUD_GOLDEN_UTIL_H

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

#endif /* NABICLOUD_GOLDEN_UTIL_H */
