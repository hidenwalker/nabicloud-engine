/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Requires the old engine
   XML/editor path removed in F-3 R4 option 4 (2026-06-29); no longer meaningful for the
   target architecture. */
/* NabiCloud keyboard editor END-TO-END runtime gate (2026-06-23).
 *
 * Proves the WHOLE editor->IME chain at RUNTIME, with no TSF shell / no DLL
 * registration -- exercising the exact engine entry points the shipping IME uses:
 *
 *   editor_serialize(oracle)  -> write to  <TEMP>/nabicloud_e2e_kbd/<id>.xml
 *   hangul_keyboard_list_load_dir(dir)     <- the SAME loader WU1 calls in
 *                                             NabiCloud.cpp::LoadExternalKeyboardsOnce
 *   hangul_keyboard_list_get_count/_id     <- keyboard becomes selectable
 *   hangul_ic_new(id) + hangul_ic_process  <- the engine COMPOSES Hangul with it
 *
 * Checks:
 *   (1) the editor file, dropped in a directory, raises the keyboard count by 1
 *       and the id becomes resolvable (= WU1 %APPDATA% scan mechanism works);
 *   (2) selecting it and typing a choseong key + a jungseong key (auto-discovered
 *       from the loaded keyboard's own map[0]) commits exactly the expected
 *       precomposed syllable AC00 + (choIdx*21 + jungIdx)*28  (= the loaded-from-
 *       file keyboard actually drives composition, correctly).
 *
 * Uses a shin id ("3shin-p"): NOT a builtin (removed in B2), so no dedup/collision
 * with the builtin list, and the data dir is never auto-loaded here. Writes only
 * under %TEMP% (repo stays clean). ASCII-only TU.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <direct.h>   /* _mkdir (MSVC) */

#include "hangul.h"
#include "hangulinternals.h"
#include "editor_serialize.h"

#ifndef HANGUL_KEYBOARD_TABLE_SIZE
#define HANGUL_KEYBOARD_TABLE_SIZE 0x80
#endif

#define TEST_ID "3shin-p"

/* append a 0-terminated ucschar string to out[*olen] (cap guarded). */
static void append_ucs(ucschar* out, int* olen, int cap, const ucschar* s)
{
    if (s == NULL) return;
    for (int i = 0; s[i] != 0 && *olen < cap - 1; ++i)
        out[(*olen)++] = s[i];
    out[*olen] = 0;
}

int main(int argc, char** argv)
{
    int fails = 0;
    printf("== NabiCloud editor END-TO-END runtime gate ==\n");
    printf("   serialize -> write dir -> hangul_keyboard_list_load_dir -> ic_new -> compose\n\n");

    /* --- serialize a sample keyboard as the 'editor output' --- */
    /*   R5(2026-06-27): shin 빌트인 오라클(hangulkeyboard.c nabicloud_shin_builtin_by_id)이
     *   제거돼, *배포* 3shin-p.xml 을 샘플 자판으로 로드한다(이 게이트는 *에디터*를 검증하므로
     *   subject 자판이 무엇이든 동등 — 자판 무영향). cwd=repo 루트(bat tests\ 호출). */
    const char* kbdir = (argc > 1) ? argv[1] : "shared/data/keyboards";
    char srcpath[640];
    snprintf(srcpath, sizeof(srcpath), "%s/%s.xml", kbdir, TEST_ID);
    HangulKeyboard* oracle = hangul_keyboard_new_from_file(srcpath);
    if (oracle == NULL) { printf("FAIL: deployed %s.xml not found (%s)\n", TEST_ID, srcpath); return 1; }
    size_t xlen = 0;
    char* xml = editor_serialize_keyboard(oracle, &xlen);
    hangul_keyboard_delete(oracle);   /* heap(new_from_file): serialize 후 즉시 해제 */
    if (xml == NULL) { printf("FAIL: serialize NULL\n"); return 1; }

    /* --- write it into a fresh dir under %TEMP% (like %APPDATA%\\NabiCloud\\keyboards) --- */
    const char* tmp = getenv("TEMP");
    if (tmp == NULL) tmp = getenv("TMP");
    if (tmp == NULL) tmp = ".";
    char dir[512];
    snprintf(dir, sizeof(dir), "%s/nabicloud_e2e_kbd", tmp);
    _mkdir(dir);   /* ok if it already exists */
    char path[640];
    snprintf(path, sizeof(path), "%s/%s.xml", dir, TEST_ID);
    FILE* f = fopen(path, "wb");
    if (f == NULL) { printf("FAIL: cannot write %s\n", path); free(xml); return 1; }
    fwrite(xml, 1, xlen, f);
    fclose(f);
    free(xml);
    printf("wrote editor output: %s (%u bytes)\n", path, (unsigned)xlen);

    /* --- (1) load the directory the way the IME does; count must rise by 1 --- */
    unsigned count0 = hangul_keyboard_list_get_count();
    unsigned loaded = hangul_keyboard_list_load_dir(dir);
    unsigned count1 = hangul_keyboard_list_get_count();
    printf("load_dir loaded=%u  count %u -> %u\n", loaded, count0, count1);

    bool id_seen = false;
    for (unsigned i = 0; i < count1; ++i) {
        const char* id = hangul_keyboard_list_get_keyboard_id(i);
        if (id != NULL && strcmp(id, TEST_ID) == 0) { id_seen = true; break; }
    }
    if (count1 != count0 + 1) { printf("  FAIL: count did not rise by exactly 1\n"); ++fails; }
    if (!id_seen)             { printf("  FAIL: id '%s' not in keyboard list after load\n", TEST_ID); ++fails; }
    else                       printf("  PASS: '%s' is now a selectable keyboard\n", TEST_ID);

    /* --- discover a choseong key and a jungseong key from the LOADED keyboard --- */
    const HangulKeyboard* loadedKb = hangul_keyboard_list_get_keyboard(TEST_ID);
    int choKey = -1, jungKey = -1;
    ucschar choVal = 0, jungVal = 0;
    if (loadedKb != NULL && loadedKb->table[0] != NULL) {
        const ucschar* t = loadedKb->table[0];
        for (int k = 0; k < HANGUL_KEYBOARD_TABLE_SIZE; ++k) {
            ucschar v = t[k];
            if (choKey < 0 && v >= 0x1100 && v <= 0x1112) { choKey = k; choVal = v; }
            if (jungKey < 0 && v >= 0x1161 && v <= 0x1175) { jungKey = k; jungVal = v; }
        }
    }
    if (choKey < 0 || jungKey < 0) {
        printf("  FAIL: could not find choseong/jungseong keys in loaded map[0]\n");
        ++fails;
    } else {
        /* --- (2) select it and compose: choKey + jungKey -> expected syllable --- */
        unsigned choIdx  = (unsigned)(choVal  - 0x1100);
        unsigned jungIdx = (unsigned)(jungVal - 0x1161);
        ucschar expect = (ucschar)(0xAC00 + (choIdx * 21 + jungIdx) * 28);

        HangulInputContext* ic = hangul_ic_new(TEST_ID);
        if (ic == NULL) { printf("  FAIL: hangul_ic_new NULL\n"); ++fails; }
        else {
            ucschar out[64]; int olen = 0; out[0] = 0;
            hangul_ic_process(ic, choKey);
            append_ucs(out, &olen, 64, hangul_ic_get_commit_string(ic));
            hangul_ic_process(ic, jungKey);
            append_ucs(out, &olen, 64, hangul_ic_get_commit_string(ic));
            append_ucs(out, &olen, 64, hangul_ic_flush(ic));
            hangul_ic_delete(ic);

            bool found = false;
            for (int i = 0; i < olen; ++i) if (out[i] == expect) { found = true; break; }
            printf("  compose key 0x%02x(%04x cho) + 0x%02x(%04x jung) -> expect U+%04X ; got:",
                   choKey, (unsigned)choVal, jungKey, (unsigned)jungVal, (unsigned)expect);
            for (int i = 0; i < olen; ++i) printf(" U+%04X", (unsigned)out[i]);
            printf("\n");
            if (found) printf("  PASS: loaded keyboard composed the expected syllable\n");
            else       { printf("  FAIL: expected syllable not produced\n"); ++fails; }
        }
    }

    /* cleanup (best-effort) */
    remove(path);
    _rmdir(dir);

    printf("\n----------------------------------------------------------------------\n");
    if (fails == 0) { printf("EDITOR_E2E_PASS (editor XML -> dir load -> selectable -> composes Hangul)\n"); return 0; }
    printf("EDITOR_E2E_FAIL: %d check(s)\n", fails);
    return fails;
}
