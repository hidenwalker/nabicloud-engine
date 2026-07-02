/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Requires the old engine
   XML/editor path removed in F-3 R4 option 4 (2026-06-29); no longer meaningful for the
   target architecture. */
/* JS<->C differential dump (2026-06-23): serialize each Sin builtin oracle to
 * tests/_jsc/<id>.xml via editor_serialize_keyboard (the C canonical serializer).
 * tests/jsc-diff.js then proves the JS serializer (windows/settings editor core)
 * reproduces these bytes exactly: JS parseXml -> serialize == C output, byte for
 * byte, for the richest field set (name/options/flags/addon/map/combination[0,2]).
 * This extends the single-keyboard WU7 byte-parity to all 9 Sin builtins.
 *
 * Output is EPHEMERAL (gate-time only, not committed). ASCII-only TU, mirroring
 * editor_roundtrip_verify.c (same engine tree, same oracle accessor).
 */
#include <stdio.h>
#include <stdlib.h>

#include "hangul.h"
#include "hangulinternals.h"   /* nabicloud_shin_builtin_by_id */
#include "editor_serialize.h"  /* editor_serialize_keyboard */

static const char* kIds[] = {
    "3shin-1995", "3shin-2003", "3shin-2012", "3shin-2015", "3shin-m",
    "3shin-p",    "3shin-p-yet", "3shin-p2",  "3shin-p2-yet",
};
#define NSHIN (sizeof(kIds) / sizeof(kIds[0]))

int main(int argc, char** argv)
{
    /* R5(2026-06-27): shin 빌트인 오라클 제거 → *배포* 3shin-*.xml 로드. JS↔C 차분은 JS-serialize ==
     *   C-serialize 검증이라 양쪽이 같은 배포 XML 을 직렬화 → 무영향(오히려 동일 소스로 더 정확).
     *   cwd=repo 루트(bat tests\ 호출). */
    const char* kbdir = (argc > 1) ? argv[1] : "shared/data/keyboards";
    size_t i;
    int n = 0;
    for (i = 0; i < NSHIN; ++i) {
        char srcpath[640];
        snprintf(srcpath, sizeof(srcpath), "%s/%s.xml", kbdir, kIds[i]);
        HangulKeyboard* kb = hangul_keyboard_new_from_file(srcpath);
        char path[256];
        size_t len = 0;
        char* xml;
        FILE* f;
        if (kb == NULL) { printf("JSC_DUMP_FAIL: deployed %s.xml missing (%s)\n", kIds[i], srcpath); return 1; }
        xml = editor_serialize_keyboard(kb, &len);
        hangul_keyboard_delete(kb);   /* heap(new_from_file) 해제 */
        if (xml == NULL) { printf("JSC_DUMP_FAIL: serialize %s\n", kIds[i]); return 1; }
        sprintf(path, "tests/_jsc/%s.xml", kIds[i]);
        f = fopen(path, "wb");
        if (f == NULL) { printf("JSC_DUMP_FAIL: open %s\n", path); free(xml); return 1; }
        fwrite(xml, 1, len, f);
        fclose(f);
        free(xml);
        ++n;
    }
    printf("JSC_DUMP_OK: %d shin canonical XMLs -> tests/_jsc/\n", n);
    return 0;
}
