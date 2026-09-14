/* NabiCloud parser regression reproduction. Invoke with CASE and output file. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int fail_at, allocations;
static void* test_malloc(size_t n) {
    return fail_at && ++allocations == fail_at ? NULL : malloc(n);
}
#define malloc test_malloc
#include "../hangul/hanja.c"
#undef malloc

int main(int argc, char** argv) {
    FILE* f;
    HanjaTable* table;
    HanjaList* rows = NULL;
    int ok = 0, i;
    const char* name;
    if (argc != 3) return 2;
    name = argv[1];
    f = fopen(argv[2], "wb");
    if (!f) return 2;
    if (!strcmp(name, "empty")) fputs("# comment\n\n", f);
    else if (!strcmp(name, "missing-value")) fputs("aa:GOOD:ok\naa:\nzz:LAST:ok\n", f);
    else if (!strcmp(name, "empty-field")) fputs("aa::injected\nzz:LAST:ok\n", f);
    else if (!strcmp(name, "colon-only")) fputs("00:FIRST:ok\naa:GOOD:ok\n::::", f);
    else if (!strcmp(name, "single-key")) fputs("aa:GOOD:ok\n", f);
    else if (!strcmp(name, "crlf")) fputs("aa:GOOD:ok\r\nzz:LAST:ok\r\n", f);
    else if (!strcmp(name, "nul-line")) {
        fputs("aa:INJECTED", f); fputc(0, f); fputs("tail\nzz:LAST:ok\n", f);
    }
    else if (!strcmp(name, "cap")) {
        for (i = 0; i < 1100; ++i) fputs("aa:GOOD:ok\n", f);
        fputs("zz:LAST:ok\n", f);
    } else if (!strcmp(name, "long-line")) {
        for (i = 0; i < 511; ++i) fputc('x', f);
        fputs("aa:INJECTED\nzz:LAST:ok\n", f);
    } else fputs("# before\naa:GOOD:ok\n# between\nzz:LAST:ok\n", f);
    fclose(f);
    if (!strcmp(name, "index-alloc")) { allocations = 0; fail_at = 1; }
    table = hanja_table_load(argv[2]);
    fail_at = 0;
    if (!strcmp(name, "empty") || !strcmp(name, "index-alloc")) ok = table == NULL;
    else if (!strcmp(name, "empty-table")) {
        HanjaTable empty = {0};
        hanja_table_match(&empty, "aa", &rows);
        ok = rows == NULL;
    } else if (table) {
        if (!strcmp(name, "row-alloc")) { allocations = 0; fail_at = 3; }
        rows = hanja_table_match_exact(table, "aa");
        fail_at = 0;
        if (!strcmp(name, "empty-field") || !strcmp(name, "long-line") || !strcmp(name, "nul-line")) ok = hanja_list_get_size(rows) == 0;
        else if (!strcmp(name, "row-alloc")) ok = hanja_list_get_size(rows) == 0;
        else if (!strcmp(name, "cap")) ok = hanja_list_get_size(rows) == 1024;
        else ok = hanja_list_get_size(rows) == 1 && hanja_list_get_nth_value(rows, 0) &&
            !strcmp(hanja_list_get_nth_value(rows, 0), "GOOD");
    }
    printf("%s %s rows=%d\n", ok ? "PASS" : "FAIL", name, hanja_list_get_size(rows));
    hanja_list_delete(rows);
    hanja_table_delete(table);
    return ok ? 0 : 1;
}
