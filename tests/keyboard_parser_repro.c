/* Reproduce the four removed K1..K4 protections with the real Expat reader. */
#include <stdio.h>
#include <string.h>
#include "../hangul/hangulkeyboard.c"
#include "../hangul/hangulinputcontext.c"

int main(int argc, char** argv) {
    FILE* f;
    HangulKeyboard* keyboard;
    const char* name;
    int ok;
    if (argc != 4) return 2;
    name = argv[1];
    if (!strcmp(name, "table-range")) {
        HangulInputContext* ic = hangul_ic_new("2");
        if (!ic) return 2;
        hangul_ic_switch_keyboard_table(ic, 3);
        hangul_ic_switch_keyboard_table(ic, -1);
        ok = ic->tableid == 3;
        hangul_ic_switch_keyboard_table(ic, 3);
        hangul_ic_switch_keyboard_table(ic, 4);
        ok = ok && ic->tableid == 3;
        hangul_ic_switch_keyboard_table(ic, 0);
        ok = ok && ic->tableid == 0;
        printf("%s table-range\n", ok ? "PASS" : "FAIL");
        hangul_ic_delete(ic);
        return ok ? 0 : 1;
    }
    f = fopen(argv[2], "wb");
    if (!f) return 2;
    if (!strcmp(name, "missing-type")) fputs("<hangul-keyboard id='test'/>", f);
    else if (!strcmp(name, "missing-id")) fputs("<hangul-keyboard type='jamo'/>", f);
    else if (!strcmp(name, "bare-item")) fputs("<hangul-keyboard id='test' type='jamo'><item key='1' value='2'/></hangul-keyboard>", f);
    else if (!strcmp(name, "include") || !strcmp(name, "include-valid")) fprintf(f, "<hangul-keyboard id='test' type='jamo'><include file='%s'/></hangul-keyboard>", argv[3]);
    else fputs("<hangul-keyboard id='test' type='jamo'><map id='0'><item key='97' value='0x1100'/></map></hangul-keyboard>", f);
    fclose(f);
    keyboard = hangul_keyboard_new_from_file(argv[2]);
    ok = keyboard != NULL;
    if (keyboard && !strcmp(name, "missing-id")) ok = keyboard->id && keyboard->id[0] == 0;
    if (keyboard && !strcmp(name, "include")) ok = !strcmp(keyboard->id, "test");
    if (keyboard && !strcmp(name, "include-valid")) ok = !strcmp(keyboard->id, "escaped");
    if (keyboard && !strcmp(name, "valid")) ok = hangul_keyboard_map_to_char(keyboard, 0, 97) == 0x1100;
    printf("%s %s\n", ok ? "PASS" : "FAIL", name);
    hangul_keyboard_delete(keyboard);
    return ok ? 0 : 1;
}
