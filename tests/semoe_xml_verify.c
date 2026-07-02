/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Requires the old engine
   XML/editor path removed in F-3 R4 option 4 (2026-06-29); no longer meaningful for the
   target architecture. */
/* NabiCloud Semo-e external-XML load verifier (PROOF harness).
 *
 * Phase 2a (2026-06-18): the Semo-e keyboards were REMOVED from the engine
 * builtins (hangulkeyboard.c) and from the GPL binary's data
 * (layout-3semoe.c/.h excluded from libhangul.vcxproj). They now ship ONLY as
 * external XML addons at addons/semoe/keyboards/3moa-semoe-*.xml.
 *
 * Byte-fidelity vs the former builtins was already proven at Phase 1.5 (commit
 * afba051) where this harness generated each XML from its builtin and diffed
 * every behavior field -> FULL-field PASS. Those byte-identical XML files
 * are now the committed oracle; the builtins are gone, so this harness can no
 * longer compare against them. Instead it proves the SHIPPING contract:
 *
 *   1. Each committed XML, loaded through the SAME loader the DLL uses
 *      (hangul_keyboard_new_from_file), registers as a valid jaso-sebeol
 *      keyboard with the documented field set (type, table[0], combination[0],
 *      flag[], addon columns).
 *   2. Loading the whole addon DIRECTORY via hangul_keyboard_list_load_dir
 *      (the DLL's LoadExternalKeyboardsOnce path) makes all 5 ids appear in the
 *      global keyboard list as jaso-sebeol keyboards -- i.e. an installed addon
 *      auto-registers the 5 keyboards, and (because the builtins are gone) they
 *      are now genuinely EXTERNAL.
 *
 * A per-id EXPECTED table (below) encodes the documented Semo-e contract so the
 * proof is positive, not just "non-NULL": which keyboards carry the symbol
 * layer (2016~2018, symbol-func=semoe), which carry the galmadeuli combination
 * in slot [1] (2018), and the loose-order flag (2018 only). This mirrors
 * the field expectations the old builtin-diff covered, now expressed directly.
 *
 * Reads internal struct fields, so it includes hangulinternals.h (compiled
 * inside the engine tree, like diff_semoe.c). ASCII-only.
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hangul.h"
#include "hangulinternals.h"

#ifndef HANGUL_KEYBOARD_TABLE_SIZE
#define HANGUL_KEYBOARD_TABLE_SIZE 0x80
#endif

/* Documented Semo-e contract (from the former builtin definitions, now the XML
 * oracle). want_symbol: addon_func[SYMBOL] bound (symbol-func="semoe"). want_galma:
 * combination[1] present (the 2018 galmadeuli table 3beol puts in the ADDON slot).
 * want_loose: flag[LOOSE_ORDER] set — ★세모이 5종 전부(전부 모아치기=loose-order; 오라클 XML 5종이
 *   모두 loose-order="true" 선언, 실측 has_loose=1). 2026-06-26: 옛 "2018 only" 는 *stale 기대*였고
 *   false-green 게이트(exit /b 0)에 가려져 있었음(Codex 지적). 검증된 실제값에 맞춰 5종 true 교정. */
typedef struct {
    const char* id;
    bool        want_symbol;
    bool        want_galma;
    bool        want_loose;
} SemoeExpect;

static const SemoeExpect kExpect[] = {
    { "3moa-semoe-2014",  false, false, true  },
    { "3moa-semoe-2015",  false, false, true  },
    { "3moa-semoe-2016",  true,  false, true  },
    { "3moa-semoe-2017",  true,  false, true  },
    { "3moa-semoe-2018",  true,  true,  true  },
};

/* Verify one loaded keyboard against its expectation. Returns 1 on full PASS. */
static int check_kb(const HangulKeyboard* kb, const SemoeExpect* e)
{
    if (kb == NULL) { printf("  FAIL load returned NULL\n"); return 0; }

    int type_ok   = (kb->type == HANGUL_KEYBOARD_TYPE_JASO_SEBEOL);
    int id_ok     = (kb->id != NULL && strcmp(kb->id, e->id) == 0);
    int t0_ok     = (kb->table[0] != NULL);
    int c0_ok     = (kb->combination[0] != NULL && kb->combination[0]->size > 0);

    bool has_symbol = (kb->addon_func[HANGUL_FUNCTION_SYMBOL] != NULL);
    bool has_galma  = (kb->combination[1] != NULL && kb->combination[1]->size > 0);
    bool has_loose  = kb->flag[HANGUL_KEYBOARD_FLAG_LOOSE_ORDER];
    /* every semoe keyboard carries the moeum key + value */
    int moeum_ok = (kb->addon_key[HANGUL_KEYBOARD_KEY_MOEUM] != NULL &&
                    kb->addon_value[HANGUL_KEYBOARD_VALUE_MOEUM] != NULL);

    int symbol_ok = (has_symbol == e->want_symbol);
    int galma_ok  = (has_galma  == e->want_galma);
    int loose_ok  = (has_loose  == e->want_loose);

    printf("  type=%d(%s) id=%s table0=%s combi0=%s moeum=%s symbol=%d/%d(%s) galma=%d/%d(%s) loose=%d/%d(%s)\n",
           kb->type, type_ok ? "JASO_SEBEOL" : "WRONG",
           id_ok ? "OK" : "DIFF", t0_ok ? "OK" : "MISSING", c0_ok ? "OK" : "MISSING",
           moeum_ok ? "OK" : "MISSING",
           (int)has_symbol, (int)e->want_symbol, symbol_ok ? "OK" : "DIFF",
           (int)has_galma,  (int)e->want_galma,  galma_ok  ? "OK" : "DIFF",
           (int)has_loose,  (int)e->want_loose,  loose_ok  ? "OK" : "DIFF");

    int pass = type_ok && id_ok && t0_ok && c0_ok && moeum_ok &&
               symbol_ok && galma_ok && loose_ok;
    printf("  --> %s\n", pass ? "PASS" : "FAIL");
    return pass;
}

int main(int argc, char** argv)
{
    /* Same default as the build script: addon keyboards dir, relative to the
     * engine root the harness runs from. */
    const char* dir = (argc > 1) ? argv[1] : "../addons/semoe/keyboards";
    printf("# semoe external-XML load verifier (Phase 2a)\n");
    printf("# dir=%s\n", dir);

    int total_file_pass = 0;  /* per-file load + field check */
    int total_dir_pass  = 0;  /* present in global list after dir scan */

    /* ---- (A) per-file load + field check (the loader path the DLL uses). ---- */
    for (unsigned i = 0; i < sizeof(kExpect)/sizeof(kExpect[0]); ++i) {
        const SemoeExpect* e = &kExpect[i];
        char path[512];
        snprintf(path, sizeof(path), "%s/%s.xml", dir, e->id);
        printf("\n=== file %s ===\n", e->id);

        HangulKeyboard* kb = hangul_keyboard_new_from_file(path);
        if (check_kb(kb, e)) ++total_file_pass;
        if (kb != NULL) hangul_keyboard_delete(kb);
    }

    /* ---- (B) directory scan: prove the 5 ids register as EXTERNAL keyboards
     * in the global list, exactly as the DLL's LoadExternalKeyboardsOnce does. */
    printf("\n=== directory scan (global registration) ===\n");
    unsigned before = hangul_keyboard_list_get_count();
    unsigned loaded = hangul_keyboard_list_load_dir(dir);
    printf("  list count before=%u after=%u (loaded=%u)\n",
           before, hangul_keyboard_list_get_count(), loaded);
    for (unsigned i = 0; i < sizeof(kExpect)/sizeof(kExpect[0]); ++i) {
        const SemoeExpect* e = &kExpect[i];
        const HangulKeyboard* kb = hangul_keyboard_list_get_keyboard(e->id);
        int ok = (kb != NULL && kb->type == HANGUL_KEYBOARD_TYPE_JASO_SEBEOL);
        printf("  %-18s registered=%s type=%d %s\n", e->id,
               kb ? "yes" : "no", kb ? kb->type : -1, ok ? "OK" : "FAIL");
        if (ok) ++total_dir_pass;
    }

    printf("\n# summary: per-file PASS=%d/5  dir-registration PASS=%d/5\n",
           total_file_pass, total_dir_pass);
    int all = (total_file_pass == 5 && total_dir_pass == 5);
    printf("# %s\n", all ? "SEMOE_XML_ALL_PASS" : "SEMOE_XML_FAIL");
    printf("# end\n");
    return all ? 0 : 1;
}
