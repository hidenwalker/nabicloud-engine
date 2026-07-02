/* F-3 R2-2/R4 gate: shell-owned builtin keymap byte-equivalence.
 *
 * Asserts the golden-frozen windows/tsf/nabicloud_builtin_keymaps.h still equals
 * the engine's hangul_keyboard_map_to_char + type for every pristine builtin x
 * key 0x21..0x7E. This is the safety net for the de-fork's map_to_char cut: the
 * shell stops calling the internal symbol and reads the embedded table instead,
 * so this gate catches engine drift, e.g. the R4 vanilla 41c702f swap.
 *
 * R4: external keyboards are V2-only and are enumerated/previewed through the shell
 * registry + clean V2Backend. This gate deliberately does not call the old
 * engine file parser or any engine XML-loader surface; V2 XML coverage is
 * in build_cleanroom_selftests (sel_r4par/sel_r4xml) and build_jaso_editor_verify.
 *
 * ASCII-only TU. Built by build_gate_shell_keymap_equiv.bat against the engine tree
 * with windows/tsf on the include path (for the generated header + meta parser).
 */
#include <stdio.h>
#include <stdlib.h>

#include "hangul.h"
#include "hangulinternals.h"          /* hangul_keyboard_map_to_char */
#include "nabicloud_builtin_keymaps.h" /* kNabicloudBuiltinKeymaps (windows/tsf, via /I) */
/* internal-meta (R2-3b/R4 registry equiv: shell type == engine). */
int  hangul_keyboard_get_type(const HangulKeyboard* keyboard);

int main(int argc, char** argv)
{
    int i, fail = 0, checked = 0;
    unsigned key;
    (void)argc;
    (void)argv;

    hangul_keyboard_list_init(NULL);

    for (i = 0; i < NABICLOUD_BUILTIN_KEYMAP_COUNT; ++i)
    {
        const struct NabicloudBuiltinKeymap* e = &kNabicloudBuiltinKeymaps[i];
        const HangulKeyboard* kb = hangul_keyboard_list_get_keyboard(e->id);
        int et;
        if (kb == NULL)
        {
            printf("KEYMAP_EQUIV_FAIL: builtin '%s' not registered\n", e->id);
            ++fail;
            continue;
        }
        /* R4: flags are shell-owned metadata; pristine engine exposes type only. */
        et  = hangul_keyboard_get_type(kb);
        if (et  != e->type)        { printf("REG_EQUIV_FAIL: %s type engine=%d header=%d\n", e->id, et, e->type); ++fail; }
        for (key = NABICLOUD_BUILTIN_KEYMAP_LO; key <= NABICLOUD_BUILTIN_KEYMAP_HI; ++key)
        {
            unsigned int expect = (unsigned int)hangul_keyboard_map_to_char(kb, 0, key);
            unsigned int got = e->map[key - NABICLOUD_BUILTIN_KEYMAP_LO];
            ++checked;
            if (expect != got)
            {
                printf("KEYMAP_EQUIV_FAIL: %s key=0x%02x engine=0x%04x header=0x%04x\n",
                       e->id, key, expect, got);
                ++fail;
            }
        }
    }

    if (fail == 0)
        printf("GATE_SHELL_KEYMAP_EQUIV_PASS: %d builtins (%d keys) == engine; engine XML-loader consumers 0\n",
               NABICLOUD_BUILTIN_KEYMAP_COUNT, checked);
    else
        printf("GATE_SHELL_KEYMAP_EQUIV_FAIL: %d mismatch(es)\n", fail);
    return fail == 0 ? 0 : 1;
}
