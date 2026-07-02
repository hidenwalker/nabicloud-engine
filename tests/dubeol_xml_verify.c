/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Requires the old engine
   XML/editor path removed in F-3 R4 option 4 (2026-06-29); no longer meaningful for the
   target architecture. */
/* NabiCloud Dubeolsik chord (두겹이) external-XML load verifier (PROOF harness).
 *
 * chord refactor (c): the 두겹이 (Dugyeob-e) moachigi keyboard ships as an
 * external CC BY-SA addon at addons/dubeol/keyboards/dugyeobe.xml with the NEW
 * type="dubeol-chord" (enum 1007). This harness proves the SHIPPING contract:
 *
 *   1. dugyeobe.xml, loaded through the SAME loader the DLL uses
 *      (hangul_keyboard_new_from_file), registers as a DUBEOL_CHORD keyboard
 *      with table[0], combination[0], and the loose-order FLAG set
 *      (HANGUL_KEYBOARD_FLAG_LOOSE_ORDER -- the shell chord gate (b)-axis read
 *      by ActiveKeyboardLooseOrder).
 *   2. A directory scan (hangul_keyboard_list_load_dir -- the DLL's
 *      LoadExternalKeyboardsOnce path) registers id "dugyeobe" in the global
 *      list as a DUBEOL_CHORD keyboard, so an installed addon auto-registers.
 *
 * Without the loader's "dubeol-chord"->1007 branch the type would fall through
 * to JAMO(0) and the chord routing (baram kBaram_dubeol_chord) would never
 * be reached -- this harness is the regression guard for that mapping.
 * Reads internal struct fields (hangulinternals.h). ASCII-only. Exit 0 = PASS.
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hangul.h"
#include "hangulinternals.h"

static int check_kb(const HangulKeyboard* kb, const char* want_id)
{
    if (kb == NULL) { printf("  FAIL load returned NULL\n"); return 0; }

    int type_ok  = (kb->type == HANGUL_KEYBOARD_TYPE_DUBEOL_CHORD);
    int id_ok    = (kb->id != NULL && strcmp(kb->id, want_id) == 0);
    int loose_ok = kb->flag[HANGUL_KEYBOARD_FLAG_LOOSE_ORDER] ? 1 : 0;
    /* ★2026-06-25 두겹이 V2 이관: dugyeobe.xml 은 이제 V2 확장 XML(<jaso engine="v2"> + <jaso-map>/
     *   <combine>)이라 libhangul <map>/<combination>(table[0]/combination[0])이 *없다* — 설계상 정상.
     *   조합은 V2 엔진(jaso_chord_compose_dubeol)이 한다. 이 게이트의 핵심 가드 = type=dubeol-chord
     *   (→enum 1007) + loose-order 플래그 + 등록(셸 chord 게이트 진입 조건). table0/combi0 는 정보표시만. */
    int t0_present = (kb->table[0] != NULL);
    int c0_present = (kb->combination[0] != NULL && kb->combination[0]->size > 0);

    printf("  type=%d(%s) id=%s loose=%d(%s) [table0=%s combi0=%s — V2 라 정상 부재 가능]\n",
           kb->type, type_ok ? "DUBEOL_CHORD" : "WRONG",
           id_ok ? "OK" : "DIFF",
           loose_ok, loose_ok ? "OK" : "MISSING",
           t0_present ? "yes" : "no(V2)", c0_present ? "yes" : "no(V2)");

    int pass = type_ok && id_ok && loose_ok;
    printf("  --> %s\n", pass ? "PASS" : "FAIL");
    return pass;
}

int main(int argc, char** argv)
{
    const char* dir = (argc > 1) ? argv[1] : "../../addons/dubeol/keyboards";
    printf("# dubeol(두겹이) external-XML load verifier (chord refactor c)\n");
    printf("# dir=%s\n", dir);

    int file_pass = 0, dir_pass = 0;
    char path[512];

    /* (A) per-file load + field check (the loader path the DLL uses). 두겹이 + 두줄이 둘 다. */
    snprintf(path, sizeof(path), "%s/dugyeobe.xml", dir);
    printf("\n=== file dugyeobe ===\n");
    HangulKeyboard* kb = hangul_keyboard_new_from_file(path);
    if (check_kb(kb, "dugyeobe")) ++file_pass;
    if (kb != NULL) hangul_keyboard_delete(kb);

    snprintf(path, sizeof(path), "%s/dujul-e.xml", dir);
    printf("\n=== file dujul-e ===\n");
    HangulKeyboard* kb2 = hangul_keyboard_new_from_file(path);
    if (check_kb(kb2, "dujul-e")) ++file_pass;
    if (kb2 != NULL) hangul_keyboard_delete(kb2);

    /* (B) directory scan: prove ids "dugyeobe"·"dujul-e" register as EXTERNAL DUBEOL_CHORD. */
    printf("\n=== directory scan (global registration) ===\n");
    unsigned before = hangul_keyboard_list_get_count();
    unsigned loaded = hangul_keyboard_list_load_dir(dir);
    printf("  list count before=%u after=%u (loaded=%u)\n",
           before, hangul_keyboard_list_get_count(), loaded);
    const HangulKeyboard* gkb = hangul_keyboard_list_get_keyboard("dugyeobe");
    int reg_ok = (gkb != NULL && gkb->type == HANGUL_KEYBOARD_TYPE_DUBEOL_CHORD);
    printf("  dugyeobe registered=%s type=%d %s\n",
           gkb ? "yes" : "no", gkb ? gkb->type : -1, reg_ok ? "OK" : "FAIL");
    if (reg_ok) ++dir_pass;
    const HangulKeyboard* gkb2 = hangul_keyboard_list_get_keyboard("dujul-e");
    int reg_ok2 = (gkb2 != NULL && gkb2->type == HANGUL_KEYBOARD_TYPE_DUBEOL_CHORD);
    printf("  dujul-e  registered=%s type=%d %s\n",
           gkb2 ? "yes" : "no", gkb2 ? gkb2->type : -1, reg_ok2 ? "OK" : "FAIL");
    if (reg_ok2) ++dir_pass;

    printf("\n# summary: per-file PASS=%d/2  dir-registration PASS=%d/2\n", file_pass, dir_pass);
    int all = (file_pass == 2 && dir_pass == 2);
    printf("# %s\n", all ? "DUBEOL_XML_ALL_PASS" : "DUBEOL_XML_FAIL");
    return all ? 0 : 1;
}
