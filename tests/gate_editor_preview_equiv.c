/* F-3 R4 gate: editor preview path no longer has an engine XML fallback.
 *
 * R2's gate compared the shell file-parser preview path with the
 * old editor_preview_compose buffer parser. R4 removes that entire engine XML
 * loader path. The two legacy standard keyboards that used the fallback
 * (3-89 / 3sun-1990) must therefore be V2 XML, so SettingsWebView routes them to
 * jaso_editor_preview_compose. The actual V2 preview composition is covered by
 * build_jaso_editor_verify; this small gate locks the routing precondition and
 * contains no old engine file-parser / IC-keyboard-setter consumer.
 *
 * ASCII-only TU. cwd = repo root (bat runs from tests\).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* slurp(const char* path)
{
    FILE* f = fopen(path, "rb");
    long n;
    char* buf;
    size_t rd;
    if (f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0) { fclose(f); return NULL; }
    buf = (char*)malloc((size_t)n + 1);
    if (buf == NULL) { fclose(f); return NULL; }
    rd = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[rd] = 0;
    return buf;
}

static int check_one(const char* kbdir, const char* file, const char* id)
{
    char path[640];
    char* xml;
    int fail = 0;
    snprintf(path, sizeof(path), "%s/%s", kbdir, file);
    xml = slurp(path);
    if (xml == NULL) {
        printf("PREVIEW_EQUIV_FAIL: missing %s\n", path);
        return 1;
    }
    if (strstr(xml, "engine=\"v2\"") == NULL) {
        printf("PREVIEW_EQUIV_FAIL: %s is not V2\n", id);
        fail = 1;
    }
    if (strstr(xml, "<jaso-map>") == NULL) {
        printf("PREVIEW_EQUIV_FAIL: %s has no jaso-map\n", id);
        fail = 1;
    }
    if (strstr(xml, "<map ") != NULL || strstr(xml, "<include ") != NULL) {
        printf("PREVIEW_EQUIV_FAIL: %s still has standard map/include fallback\n", id);
        fail = 1;
    }
    free(xml);
    if (!fail) printf("  [ok] %s routes to V2 editor preview\n", id);
    return fail;
}

int main(int argc, char** argv)
{
    const char* kbdir = (argc > 1) ? argv[1] : "shared/data/keyboards";
    int fail = 0;
    fail |= check_one(kbdir, "hangul-keyboard-3-89.xml", "3-89");
    fail |= check_one(kbdir, "hangul-keyboard-3sun-1990.xml", "3sun-1990");
    if (fail == 0)
        printf("GATE_EDITOR_PREVIEW_EQUIV_PASS: legacy standard XML preview moved to V2; engine XML-loader consumers 0\n");
    else
        printf("GATE_EDITOR_PREVIEW_EQUIV_FAIL\n");
    return fail ? 1 : 0;
}
