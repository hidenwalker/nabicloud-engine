/* editor_jsc_dump_v2.cpp -- V2 JS<->C differential dump (2026-07-02).
 *
 * Successor of the RETIRED editor_jsc_dump.c (old engine XML/editor path, F-3 R4).
 * Dumps every kJasoBuiltin V2 jaso keyboard (14 ids, same list as
 * jaso_editor_verify.cpp / v2backend.cpp kJasoBuiltin single-source table) through
 * the LIVE editor bridge jaso_editor_serialize_by_id() -> canonical jaso_xml_dump
 * bytes, then replicates the shell meta injection (SettingsWebView.cpp editorLoad):
 *
 *     "<hangul-keyboard type=\"nabicloud\">"
 *  -> "<hangul-keyboard id=\"ID\" type=\"nabicloud\">\n  <name>NAME</name>"
 *
 * (name escapes & < > ; id attr additionally escapes ") so each _jsc_v2/<id>.xml
 * holds the EXACT bytes the JS editor (editor-core.js parseKeyboardXml) receives
 * in production. tests/jsc-diff-v2.js then proves JS parse->serialize reproduces
 * these bytes 1:1 (JS V2 serializer == C canonical dump + shell meta).
 *
 * [chord round-trip contract] Production editorLoad additionally re-injects the
 * load-bearing shell meta the dump omits -- real type for chord keyboards
 * (1006 sebeol-chord / 1007 dubeol-chord) and a "<flags loose-order/abbrev>" line
 * after </name> -- from the shell registry (SettingsWebView.cpp EditorCmdLoad).
 * All 14 kJasoBuiltin ids are non-chord and flag-less, so that injection is a
 * byte no-op here and this replica stays exact WITHOUT registry access. The chord
 * side of the contract is anchored by WU9 (windows/settings/tests/
 * editor-jaso.test.js "chord 셸 메타" tests) against the shipping addon XMLs.
 * If a chord/flagged keyboard ever joins kJasoBuiltin, extend this replica with
 * a stand-in meta column mirroring EditorCmdLoad.
 *
 * Probe names deliberately include the encoding traps: XML escapes (& < >),
 * UTF-8 Hangul (U+AC00 = EA B0 80) and PUA (U+E000 = EE 80 80) via hex escapes
 * (ASCII-only TU, mirroring editor_jsc_dump.c / jaso_editor_verify.cpp harness).
 *
 * Output is EPHEMERAL (gate-time only, not committed; dir passed as argv[1]).
 * Also asserts the negative: "4z" (VM register machine, no jaso layout) must
 * serialize to NULL (editor non-target), mirroring jaso_editor_verify edges.
 */
#include "jaso_xml_editor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

/* kJasoBuiltin 14 ids (v2backend.cpp single-source table; keep in sync with
 * jaso_editor_verify.cpp ids[]). name = display-name stand-in; two entries carry
 * escape/UTF-8/PUA probes -- parity only needs both sides to see the same bytes,
 * so the gate name need not equal the production display name. */
static const struct { const char* id; const char* name; } kKb[] = {
    { "3shin-p2",     "3shin-p2" },
    { "3shin-p",      "3shin-p" },
    { "3shin-2012",   "3shin-2012" },
    { "3shin-2015",   "3shin-2015" },
    { "3shin-m",      "3shin-m" },
    { "3shin-p-yet",  "3shin-p-yet \xEE\x80\x80" },          /* PUA U+E000 probe */
    { "3shin-p2-yet", "3shin-p2-yet" },
    { "3gs",          "3gs" },
    { "39",           "39" },
    { "3f",           "3f" },
    { "2noshift",     "2noshift" },
    { "2n9256",       "2n9256" },
    { "2",            "dubeol &<> \xEA\xB0\x80" },           /* esc + U+AC00 probe */
    { "3sun-2014",    "3sun-2014" },
};
#define NKB (sizeof(kKb) / sizeof(kKb[0]))

/* Mirror of SettingsWebView.cpp editorLoad escaping: name = & < > ;
 * attr(id) additionally " . Never emits &apos;/&quot; in char data, so the JS
 * unescText -> escText round-trip reproduces the bytes exactly. */
static void esc_append(std::string& o, const char* s, bool attr)
{
    for (; *s != 0; ++s) {
        char c = *s;
        if (c == '&')              o += "&amp;";
        else if (c == '<')         o += "&lt;";
        else if (c == '>')         o += "&gt;";
        else if (attr && c == '"') o += "&quot;";
        else                       o += c;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("JSC_DUMP_V2_FAIL: usage: editor_jsc_dump_v2 <outdir>\n");
        return 1;
    }
    const char* outdir = argv[1];
    static const char kRootOld[] = "<hangul-keyboard type=\"nabicloud\">";

    int n = 0;
    for (size_t i = 0; i < NKB; ++i) {
        size_t xl = 0;
        char* xml = jaso_editor_serialize_by_id(kKb[i].id, &xl);
        if (xml == NULL) {
            printf("JSC_DUMP_V2_FAIL: serialize %s -> NULL\n", kKb[i].id);
            return 1;
        }
        std::string s(xml, xl);
        free(xml);

        /* Shell meta injection (editorLoad replica) -- root tag replace. */
        size_t p = s.find(kRootOld);
        if (p == std::string::npos) {
            printf("JSC_DUMP_V2_FAIL: %s dump has no canonical root tag\n", kKb[i].id);
            return 1;
        }
        std::string repl = "<hangul-keyboard id=\"";
        esc_append(repl, kKb[i].id, true);
        repl += "\" type=\"nabicloud\">\n  <name>";
        esc_append(repl, kKb[i].name, false);
        repl += "</name>";
        s.replace(p, sizeof(kRootOld) - 1, repl);

        /* Sanity: the injected XML must still pass the canonical save-path
         * validator (jaso_xml_load), like a JS save round would. */
        size_t vl = 0;
        char* v = jaso_editor_validate_xml(s.data(), s.size(), &vl);
        if (v == NULL) {
            printf("JSC_DUMP_V2_FAIL: validate %s (meta-injected) rejected\n", kKb[i].id);
            return 1;
        }
        free(v);

        std::string path = std::string(outdir) + "/" + kKb[i].id + ".xml";
        FILE* f = fopen(path.c_str(), "wb");
        if (f == NULL) {
            printf("JSC_DUMP_V2_FAIL: open %s\n", path.c_str());
            return 1;
        }
        size_t w = fwrite(s.data(), 1, s.size(), f);
        fclose(f);
        if (w != s.size()) {
            printf("JSC_DUMP_V2_FAIL: short write %s\n", path.c_str());
            return 1;
        }
        ++n;
    }

    /* Negative: VM register machine (4z) has no jaso layout -> NULL. */
    {
        size_t zl = 0;
        char* z = jaso_editor_serialize_by_id("4z", &zl);
        if (z != NULL) {
            printf("JSC_DUMP_V2_FAIL: 4z serialized (must be NULL, editor non-target)\n");
            free(z);
            return 1;
        }
    }

    printf("JSC_DUMP_V2_OK: %d V2 builtin canonical XMLs -> %s\n", n, outdir);
    return 0;
}
