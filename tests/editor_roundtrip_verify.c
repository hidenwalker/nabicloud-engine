/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Requires the old engine
   XML/editor path removed in F-3 R4 option 4 (2026-06-29); no longer meaningful for the
   target architecture. */
/* NabiCloud keyboard editor round-trip gate (2026-06-23).
 *
 * Proves the EDITOR pipeline is byte-faithful, end to end, through the SAME
 * loader the shipping IME uses -- without touching any committed data file:
 *
 *   oracle struct  --editor_serialize_keyboard-->  XML buffer (buf1)
 *                  --nabicloud_xml_parse_buffer-->  reloaded keyboard (kb2)
 *
 * Two independent checks per keyboard:
 *   (A) FIELD DIFF : kb2 vs oracle, full field set (id/type/name/mode/map[0..3]/
 *       combination[0..3]/flag[6]/addon/multikey/key-context). diff==0 == exact.
 *   (B) IDEMPOTENCE: serialize(kb2) == buf1 byte-for-byte (serializer is a fixed
 *       point -> save/reload/save is stable -> no byte churn on re-save).
 *
 * A-stage scope (DESIGN 5.5): the 9 신세벌 oracles, reachable in-memory via
 * nabicloud_shin_builtin_by_id() -- the same oracle tests/shin_xml_verify.c uses,
 * but exercised through editor_serialize.c + nabicloud_xml_parse_buffer instead of
 * the file generator. Modifies NO builtin source and writes NO data file.
 * ASCII-only TU.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "hangul.h"
#include "hangulinternals.h"
#include "editor_serialize.h"

#ifndef HANGUL_KEYBOARD_TABLE_SIZE
#define HANGUL_KEYBOARD_TABLE_SIZE 0x80
#endif

static const char* kIds[] = {
    "3shin-1995", "3shin-2003", "3shin-2012",
    "3shin-2015", "3shin-m",
    "3shin-p",    "3shin-p-yet",
    "3shin-p2",   "3shin-p2-yet",
};
#define NSHIN (sizeof(kIds) / sizeof(kIds[0]))

/* ---- comparison helpers (adapted from shin_xml_verify.c) ---- */

static int cmp_combi_item(const void* a, const void* b)
{
    const HangulCombinationItem* x = a;
    const HangulCombinationItem* y = b;
    if (x->key != y->key) return (x->key < y->key) ? -1 : 1;
    if (x->code != y->code) return (x->code < y->code) ? -1 : 1;
    return 0;
}

static int combi_equal(const HangulCombination* a, const HangulCombination* b, const char* tag)
{
    if ((a == NULL) != (b == NULL)) {
        printf("    DIFF %s presence: oracle=%s rt=%s\n", tag,
               a ? "present" : "NULL", b ? "present" : "NULL");
        return 0;
    }
    if (a == NULL) return 1;
    if (a->size != b->size) {
        printf("    DIFF %s size: oracle=%u rt=%u\n", tag, (unsigned)a->size, (unsigned)b->size);
        return 0;
    }
    if (a->size == 0) return 1;
    int n = (int)a->size;
    HangulCombinationItem* sa = malloc(sizeof(*sa) * n);
    HangulCombinationItem* sb = malloc(sizeof(*sb) * n);
    memcpy(sa, a->table, sizeof(*sa) * n);
    memcpy(sb, b->table, sizeof(*sb) * n);
    qsort(sa, n, sizeof(*sa), cmp_combi_item);
    qsort(sb, n, sizeof(*sb), cmp_combi_item);
    int ok = 1;
    for (int i = 0; i < n; ++i)
        if (sa[i].key != sb[i].key || sa[i].code != sb[i].code) {
            printf("    DIFF %s[%d]: oracle key=0x%08x code=0x%04x  rt key=0x%08x code=0x%04x\n",
                   tag, i, (unsigned)sa[i].key, (unsigned)sa[i].code,
                   (unsigned)sb[i].key, (unsigned)sb[i].code);
            ok = 0;
        }
    free(sa); free(sb);
    return ok;
}

static int streq0(const char* a, const char* b)
{
    if (a == NULL && b == NULL) return 1;
    if (a == NULL || b == NULL) return 0;
    return strcmp(a, b) == 0;
}

static int ucs_list_equal(const ucschar* a, const ucschar* b)
{
    if ((a == NULL) != (b == NULL)) return 0;
    if (a == NULL) return 1;
    unsigned i = 0;
    for (; a[i] != 0 && b[i] != 0; ++i)
        if (a[i] != b[i]) return 0;
    return a[i] == b[i];
}

static int map_equal(const ucschar* a, const ucschar* b, const char* tag)
{
    if ((a == NULL) != (b == NULL)) {
        printf("    DIFF %s presence: oracle=%s rt=%s\n", tag,
               a ? "present" : "NULL", b ? "present" : "NULL");
        return 0;
    }
    if (a == NULL) return 1;
    int d = 0;
    for (unsigned k = 0; k < HANGUL_KEYBOARD_TABLE_SIZE; ++k)
        if (a[k] != b[k]) {
            if (d < 6)
                printf("    DIFF %s[0x%02x]: oracle=0x%04x rt=0x%04x\n",
                       tag, k, (unsigned)a[k], (unsigned)b[k]);
            ++d;
        }
    if (d) printf("    %s differential entries=%d\n", tag, d);
    return d == 0;
}

/* number of multikey rows (NULL-terminated by firstKey==0), or -1 if NULL. */
static int multikey_count(const KeyMultiKeyTable* m)
{
    if (m == NULL) return -1;
    int n = 0;
    while (m[n].firstKey != 0) ++n;
    return n;
}

static int multikey_equal(const KeyMultiKeyTable* a, const KeyMultiKeyTable* b)
{
    int an = multikey_count(a), bn = multikey_count(b);
    if (an != bn) { printf("    DIFF multikey count: oracle=%d rt=%d\n", an, bn); return 0; }
    if (an <= 0) return 1;
    for (int i = 0; i < an; ++i)
        if (a[i].firstKey != b[i].firstKey || a[i].secondKey != b[i].secondKey ||
            a[i].thirdKey != b[i].thirdKey) {
            printf("    DIFF multikey[%d]\n", i); return 0;
        }
    return 1;
}

static int keyctx_count(const NabiCloudKeyContextItem* c)
{
    if (c == NULL) return -1;
    int n = 0;
    while (c[n].when != NABICLOUD_KEY_CONTEXT_NONE) ++n;
    return n;
}

static int keyctx_equal(const NabiCloudKeyContextItem* a, const NabiCloudKeyContextItem* b)
{
    int an = keyctx_count(a), bn = keyctx_count(b);
    if (an != bn) { printf("    DIFF key-context count: oracle=%d rt=%d\n", an, bn); return 0; }
    /* Order-insensitive compare not needed: emit groups by `when` preserving
     * within-group order; the loader appends in document order, so a present
     * key-context round-trips in the same order. A simple positional compare
     * suffices for the shin family (none carry key-context, so an==bn==-1). */
    if (an <= 0) return 1;
    for (int i = 0; i < an; ++i)
        if (a[i].when != b[i].when || a[i].key != b[i].key || a[i].value != b[i].value) {
            printf("    DIFF key-context[%d]\n", i); return 0;
        }
    return 1;
}

static int verify_one(const HangulKeyboard* o, const HangulKeyboard* r)
{
    int diff = 0;
    if (r == NULL) { printf("    DIFF parse returned NULL\n"); return 1; }

    if (!streq0(o->id, r->id))   { printf("    DIFF id: %s / %s\n", o->id, r->id); ++diff; }
    if (o->type != r->type)      { printf("    DIFF type: %d / %d\n", o->type, r->type); ++diff; }
    if (!streq0(o->name, r->name)) { printf("    DIFF name\n"); ++diff; }
    if (o->mode_key != r->mode_key) { printf("    DIFF mode_key: 0x%04x / 0x%04x\n",
                                             (unsigned)o->mode_key, (unsigned)r->mode_key); ++diff; }

    if (!map_equal(o->table[0], r->table[0], "map[0]")) ++diff;
    if (!map_equal(o->table[1], r->table[1], "map[1]")) ++diff;
    if (!map_equal(o->table[2], r->table[2], "map[2]")) ++diff;
    if (!map_equal(o->table[3], r->table[3], "map[3]")) ++diff;

    if (!combi_equal(o->combination[0], r->combination[0], "combination[0]")) ++diff;
    if (!combi_equal(o->combination[1], r->combination[1], "combination[1]")) ++diff;
    if (!combi_equal(o->combination[2], r->combination[2], "combination[2]")) ++diff;
    if (!combi_equal(o->combination[3], r->combination[3], "combination[3]")) ++diff;

    for (int i = 0; i < 6; ++i)
        if (o->flag[i] != r->flag[i]) {
            printf("    DIFF flag[%d]: %d / %d\n", i, o->flag[i], r->flag[i]); ++diff;
        }

    for (int i = 0; i < 3; ++i)
        if (!streq0(o->addon_key[i], r->addon_key[i])) {
            printf("    DIFF addon_key[%d]\n", i); ++diff;
        }
    for (int i = 0; i < 4; ++i)
        if (!ucs_list_equal(o->addon_value[i], r->addon_value[i])) {
            printf("    DIFF addon_value[%d]\n", i); ++diff;
        }
    if ((o->addon_func[HANGUL_FUNCTION_SYMBOL] != NULL) !=
        (r->addon_func[HANGUL_FUNCTION_SYMBOL] != NULL)) {
        printf("    DIFF addon_func[SYMBOL] presence\n"); ++diff;
    } else if (o->addon_func[HANGUL_FUNCTION_SYMBOL] != NULL &&
               o->addon_func[HANGUL_FUNCTION_SYMBOL] != r->addon_func[HANGUL_FUNCTION_SYMBOL]) {
        printf("    DIFF addon_func[SYMBOL] pointer\n"); ++diff;
    }

    if (!multikey_equal(o->multikey, r->multikey)) ++diff;
    if (!keyctx_equal(o->key_context, r->key_context)) ++diff;

    return diff;
}

int main(int argc, char** argv)
{
    /* R5(2026-06-27): shin 빌트인 오라클(nabicloud_shin_builtin_by_id) 제거 → *배포* 3shin-*.xml 을
     *   샘플 자판으로 로드(이 게이트는 *에디터* serialize/round-trip 을 검증하므로 subject 자판 무관).
     *   cwd=repo 루트(bat tests\ 호출). 디렉터리는 argv[1] 또는 기본값. */
    const char* kbdir = (argc > 1) ? argv[1] : "shared/data/keyboards";
    int total_diff = 0;
    int fail_ids = 0;
    int idempotence_fail = 0;

    printf("== NabiCloud editor round-trip gate (editor_serialize + nabicloud_xml_parse_buffer) ==\n");
    printf("   oracle -> serialize -> parse_buffer -> (A) field diff vs oracle  (B) re-serialize idempotence\n\n");

    for (size_t i = 0; i < NSHIN; ++i) {
        const char* id = kIds[i];
        char srcpath[640];
        snprintf(srcpath, sizeof(srcpath), "%s/%s.xml", kbdir, id);
        const HangulKeyboard* oracle = hangul_keyboard_new_from_file(srcpath);
        printf("%s\n", id);
        if (oracle == NULL) {
            printf("    FAIL: deployed %s.xml not found (%s)\n", id, srcpath);
            ++fail_ids;
            continue;
        }

        size_t len1 = 0;
        char* buf1 = editor_serialize_keyboard(oracle, &len1);
        if (buf1 == NULL) {
            printf("    FAIL: serialize returned NULL\n");
            ++fail_ids;
            hangul_keyboard_delete((HangulKeyboard*)oracle);
            continue;
        }

        HangulKeyboard* kb2 = nabicloud_xml_parse_buffer(buf1, len1);
        int d = verify_one(oracle, kb2);
        total_diff += d;

        /* (B) idempotence: serialize the reparsed keyboard, compare bytes. */
        int idem = 0;
        if (kb2 != NULL) {
            size_t len2 = 0;
            char* buf2 = editor_serialize_keyboard(kb2, &len2);
            if (buf2 == NULL) {
                printf("    DIFF idempotence: re-serialize NULL\n");
                idem = 1;
            } else if (len1 != len2 || memcmp(buf1, buf2, len1) != 0) {
                printf("    DIFF idempotence: buf1(%u) != buf2(%u)\n",
                       (unsigned)len1, (unsigned)len2);
                idem = 1;
            }
            free(buf2);
        }

        if (d == 0 && idem == 0) {
            printf("    PASS (field diff=0, idempotent, %u bytes)\n", (unsigned)len1);
        } else {
            printf("    FAIL (field diff=%d%s)\n", d, idem ? ", idempotence broken" : "");
            ++fail_ids;
            if (idem) ++idempotence_fail;
        }
        free(buf1);
        hangul_keyboard_delete((HangulKeyboard*)oracle);   /* heap(new_from_file) 해제 */
    }

    /* ---- synthetic tests for review fixes (M1 escape, M2 NULL-map) + canonicalize ---- */
    printf("\n[synthetic] review-fix + canonicalize checks\n");
    {
        /* M1: XML entities in <name> must round-trip (loader decodes char data). */
        const char* x1 =
            "<hangul-keyboard id=\"t-esc\" type=\"jaso\">"
            "<name>A &amp; B &lt;C&gt;</name>"
            "<map id=\"0\"><item key=\"0x72\" value=\"0x1100\"/></map>"
            "</hangul-keyboard>";
        HangulKeyboard* k1 = nabicloud_xml_parse_buffer(x1, strlen(x1));
        int ok = (k1 != NULL && k1->name != NULL && strcmp(k1->name, "A & B <C>") == 0);
        if (ok) {
            size_t cl = 0;
            char* cx = editor_serialize_keyboard(k1, &cl);
            HangulKeyboard* k2 = cx ? nabicloud_xml_parse_buffer(cx, cl) : NULL;
            ok = (cx != NULL && k2 != NULL && k2->name != NULL &&
                  strcmp(k2->name, "A & B <C>") == 0);
            if (ok && strstr(cx, "&lt;C&gt;") == NULL) ok = 0;  /* must be escaped, not raw '<' */
            if (k2) hangul_keyboard_delete(k2);
            free(cx);
        }
        if (k1) hangul_keyboard_delete(k1);
        printf("  M1 name-escape round-trip: %s\n", ok ? "PASS" : "FAIL");
        if (!ok) ++fail_ids;
    }
    {
        /* M2: no <map> -> table[0]==NULL must round-trip to NULL (no 128-zero map). */
        const char* x =
            "<hangul-keyboard id=\"t-nomap\" type=\"jaso\"><name>x</name></hangul-keyboard>";
        HangulKeyboard* k1 = nabicloud_xml_parse_buffer(x, strlen(x));
        int ok = (k1 != NULL && k1->table[0] == NULL);
        if (ok) {
            size_t cl = 0;
            char* cx = editor_serialize_keyboard(k1, &cl);
            ok = (cx != NULL && strstr(cx, "<map") == NULL);
            HangulKeyboard* k2 = cx ? nabicloud_xml_parse_buffer(cx, cl) : NULL;
            if (ok) ok = (k2 != NULL && k2->table[0] == NULL);
            if (k2) hangul_keyboard_delete(k2);
            free(cx);
        }
        if (k1) hangul_keyboard_delete(k1);
        printf("  M2 NULL-map round-trip: %s\n", ok ? "PASS" : "FAIL");
        if (!ok) ++fail_ids;
    }
    {
        /* canonicalize: 배포 3shin-p.xml direct-serialize == canonicalize (fixed point);
         * a 'messy' valid input (xml decl + comment + extra whitespace) canonicalizes.
         * R5: shin 오라클 제거 → 배포 XML 로드(에디터 정규화 검증, subject 자판 무관). */
        char cpath[640];
        snprintf(cpath, sizeof(cpath), "%s/3shin-p.xml", kbdir);
        const HangulKeyboard* oracle = hangul_keyboard_new_from_file(cpath);
        int ok = (oracle != NULL);
        if (ok) {
            size_t dl = 0;
            char* direct = editor_serialize_keyboard(oracle, &dl);
            size_t canl = 0;
            char* canon = direct ? editor_canonicalize_xml(direct, dl, &canl) : NULL;
            ok = (direct != NULL && canon != NULL && dl == canl && memcmp(direct, canon, dl) == 0);
            free(canon);
            free(direct);
        }
        if (oracle != NULL) hangul_keyboard_delete((HangulKeyboard*)oracle);
        const char* messy =
            "<?xml version=\"1.0\"?>\n<!-- hi -->\n"
            "<hangul-keyboard   id=\"t-messy\"   type=\"jaso\" >\n"
            "  <name>m</name>\n"
            "  <map id=\"0\"><item key=\"0x72\" value=\"0x1100\"/></map>\n"
            "</hangul-keyboard>\n";
        size_t ml = 0;
        char* mc = editor_canonicalize_xml(messy, strlen(messy), &ml);
        int ok2 = (mc != NULL && ml > 0);
        free(mc);
        printf("  canonicalize fixed-point: %s ; messy-input: %s\n",
               ok ? "PASS" : "FAIL", ok2 ? "PASS" : "FAIL");
        if (!ok) ++fail_ids;
        if (!ok2) ++fail_ids;
    }

    printf("\n----------------------------------------------------------------------\n");
    if (fail_ids == 0 && total_diff == 0) {
        printf("EDITOR_ROUNDTRIP_PASS (%u/%u keyboards, total field diff=0, idempotent)\n",
               (unsigned)NSHIN, (unsigned)NSHIN);
        return 0;
    }
    printf("EDITOR_ROUNDTRIP_FAIL: %d keyboard(s), total field diff=%d, idempotence fails=%d\n",
           fail_ids, total_diff, idempotence_fail);
    return fail_ids ? fail_ids : 1;
}
