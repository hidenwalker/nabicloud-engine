/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. libhangul-path shin XML oracle;
   after the shin XMLs moved to V2 (2026-06-26) the chord output is empty, so the oracle is
   meaningless. */
/*
 * ★RETIRED (2026-06-26 전수정리) — 이 libhangul-경로 오라클은 *역사적 클린룸 절차증거*다.
 *   신세벌 XML 이 V2 확장 XML(engine="v2")로 이관(2026-06-25(3))된 뒤로 이 하니스는 libhangul IC
 *   로 구동돼 chord(예 kf->가)가 빈 출력이 된다(V2 chord 미구동). ★현재 V2 신세벌-p2 거동의 정본
 *   오라클 = build_shin_xml_verify(게이트) + cleanroom selftest_jaso_shinp2(B2). 현 검증엔 쓰지 말 것.
 *   clean-room provenance(CLEANROOM-baram.md) 보존 위해 *이동/V2재작성 없이 상태만 표기*한다 —
 *   V2-재작성은 위 정본 오라클과 중복이고, libhangul 포트지문 증거를 바꿀 소지.
 *
 * NabiCloud 신세벌 P2 behavioral oracle via the EXTERNAL-XML load path.
 *
 * WHY (2026-06-22): the 9 신세벌 keyboards were moved out of the engine builtin
 * list (hangul_builtin_keyboards[] in hangulkeyboard.c) and now ship ONLY as
 * external body keyboards at shared/data/keyboards/3shin-*.xml. The frozen
 * builtin behavioral fingerprint (tests/shin-p2-port-fingerprint.txt, generated
 * by diff_shin.c) is still valid because data+engine are frozen, but diff_shin.c
 * calls hangul_ic_new("3shin-p2") which only resolves builtins -> NULL now. This
 * harness restores a live behavioral oracle by calling hangul_keyboard_list_load_dir()
 * FIRST so the external XML keyboards register, after which hangul_ic_new() resolves
 * "3shin-p2" through hangul_keyboard_list_get_keyboard()'s external fallback.
 *
 * Three blocks, all P2-plain (port-aware verification, NOT clean-room spec):
 *   SUITE   : the exact diff_shin.c suite for 3shin-p2 -- must reproduce
 *             shin-p2-port-fingerprint.txt byte-for-byte (XML oracle == builtin oracle).
 *   ANCHORS : the union of both port-naive specs' verification anchors
 *             (cleanroom/shin-p2-spec.{claude,codex}.md §7) -- types the full
 *             key string and compares the WHOLE output (every commit + final flush)
 *             to the expected NFC codepoints. PASS/FAIL/OBSERVE per anchor.
 *   DIV2    : isolated lowercase context-vowel probes (i/o/p// after a lone
 *             choseong) to settle CLEANROOM-baram §8.6 divergence #2 -- whether the
 *             2018-04-10 revision (which removed the Shift-direct uppercase ㅡ/ㅜ/ㆍ)
 *             left the lowercase right_oua context path active.
 *
 * ASCII-only (the keys are physical English keys; the keymap is in the XML). No
 * UTF-8 BOM, no CP949 trap.
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hangul.h"
#include "hangulinternals.h"   /* HangulKeyboard struct fields (->type), like shin_xml_verify.c */

static void fmt(const ucschar* s, char* out, size_t cap) {
    size_t n = 0;
    if (s == NULL) { snprintf(out, cap, "(null)"); return; }
    if (*s == 0)   { snprintf(out, cap, "(empty)"); return; }
    out[0] = '\0';
    while (*s && n + 8 < cap) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%sU+%04X", (n ? " " : ""), (unsigned)*s);
        strncat(out, tmp, cap - strlen(out) - 1);
        n += strlen(tmp);
        ++s;
    }
}

/* ---- SUITE (verbatim diff_shin.c, so the byte comparison vs the frozen
 *      fingerprint is valid) ---- */
static void run_str(const char* kbd, const char* name, const char* s) {
    char cbuf[512], pbuf[512];
    HangulInputContext* hic = hangul_ic_new(kbd);
    if (hic == NULL) { printf("T %s\n  FAIL new==NULL\n", name); return; }
    printf("T %s\n", name);
    for (const char* p = s; *p; ++p) {
        int processed;
        if (*p == '\b') {
            processed = hangul_ic_backspace(hic) ? 1 : 0;
        } else {
            processed = hangul_ic_process(hic, (unsigned char)*p) ? 1 : 0;
        }
        fmt(hangul_ic_get_commit_string(hic), cbuf, sizeof(cbuf));
        fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
        printf("  k=0x%02X p=%d commit=%s pre=%s\n",
               (unsigned)(unsigned char)*p, processed, cbuf, pbuf);
    }
    fmt(hangul_ic_flush(hic), cbuf, sizeof(cbuf));
    printf("  flush=%s\n", cbuf);
    hangul_ic_delete(hic);
}

static void suite(const char* kbd) {
    run_str(kbd, "word_gana",  "rk");
    run_str(kbd, "word_han",   "msf");
    run_str(kbd, "word_gksrmf","gksrmf");
    run_str(kbd, "rep_f",      "fff");
    run_str(kbd, "rkk",        "rkk");
    run_str(kbd, "tkfa",       "tkfa");
    run_str(kbd, "gwa_rhk",    "rhk");
    run_str(kbd, "oua_left",   "kv");
    run_str(kbd, "oua_P",      "kP");
    run_str(kbd, "oua_O",      "kO");
    run_str(kbd, "jong_dbl",   "rkqq");
    run_str(kbd, "shift_caps", "kRf");
    run_str(kbd, "araea_brk",  "k[");
    run_str(kbd, "araea_brk2", "k[f");
    run_str(kbd, "ext_H",      "jH");
    run_str(kbd, "ext_U",      "jU");
    run_str(kbd, "ext_Y",      "jY");
    run_str(kbd, "ext_Z",      "jZ");
    run_str(kbd, "ext_seq",    "jHk");
    run_str(kbd, "mark_U",     "kfU");
    run_str(kbd, "mark_Y",     "kfY");
    run_str(kbd, "digits",     "0123456789");
    run_str(kbd, "syms",       "!@#$%^&*()");
    run_str(kbd, "long",       "mfsrnfdfj");
    run_str(kbd, "bracket",    "kf[sif[");
    run_str(kbd, "uppers",     "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    run_str(kbd, "lowers",     "abcdefghijklmnopqrstuvwxyz");
    run_str(kbd, "bs_plain",   "rk\b");
    run_str(kbd, "bs_cvc",     "rkq\b\b");
    run_str(kbd, "bs_ext_H",   "jH\b");
    run_str(kbd, "bs_ext_seq", "jHk\b\b");
    run_str(kbd, "bs_araea",   "k[\b");
    run_str(kbd, "bs_mark",    "kfU\b\b");
    run_str(kbd, "bs_overrun", "rk\b\b\b");
    run_str(kbd, "bs_mixed",   "jHk\brk\b");
}

/* ---- ANCHORS / DIV2 ---- */

static int ucs_eq(const ucschar* a, const ucschar* b) {
    unsigned i = 0;
    for (; a[i] != 0 && b[i] != 0; ++i)
        if (a[i] != b[i]) return 0;
    return a[i] == b[i];
}

/* Type the whole key string and accumulate the FULL output (every per-step
 * commit, then the final flush) as one ordered codepoint list. expect==NULL =>
 * OBSERVE (record only, e.g. opt-hangul jamo whose plain rendering is OPEN). */
static int anchor(const char* kbd, const char* name, const char* keys,
                  const ucschar* expect, const char* note) {
    HangulInputContext* hic = hangul_ic_new(kbd);
    if (hic == NULL) { printf("A %-9s FAIL new==NULL\n", name); return 0; }
    ucschar acc[256]; size_t an = 0;
    for (const char* p = keys; *p; ++p) {
        if (*p == '\b') hangul_ic_backspace(hic);
        else hangul_ic_process(hic, (unsigned char)*p);
        const ucschar* c = hangul_ic_get_commit_string(hic);
        if (c) for (; *c && an < 255; ++c) acc[an++] = *c;
    }
    const ucschar* f = hangul_ic_flush(hic);
    if (f) for (; *f && an < 255; ++f) acc[an++] = *f;
    acc[an] = 0;
    hangul_ic_delete(hic);

    char gotbuf[512];
    fmt(acc, gotbuf, sizeof(gotbuf));
    if (expect == NULL) {
        printf("A %-9s keys=\"%-7s\" got=%-28s [OBSERVE] %s\n", name, keys, gotbuf, note ? note : "");
        return 1; /* observe never fails the gate */
    }
    char expbuf[512];
    fmt(expect, expbuf, sizeof(expbuf));
    int ok = ucs_eq(acc, expect);
    printf("A %-9s keys=\"%-7s\" got=%-28s exp=%-12s %s %s\n",
           name, keys, gotbuf, expbuf, ok ? "PASS" : "FAIL", note ? note : "");
    return ok;
}

static int anchors_p2(const char* kbd) {
    int pass = 0, total = 0;
    printf("\n=== ANCHORS (union of shin-p2-spec.{claude,codex} section 7) ===\n");

    /* claude-spec §7 (corrected to merged keymap: c=jong ㄱ, s=jong ㄴ). */
    static const ucschar e_ga[]   = {0xAC00, 0};           /* 가 */
    static const ucschar e_at[]   = {0xC558, 0};           /* 았 */
    static const ucschar e_ilg[]  = {0xC77D, 0};           /* 읽 */
    static const ucschar e_huin[] = {0xD770, 0};           /* 흰 */
    static const ucschar e_hwal[] = {0xD65C, 0};           /* 활 */
    static const ucschar e_hwol[] = {0xD6E8, 0};           /* 훨 */
    static const ucschar e_kka[]  = {0xAE4C, 0};           /* 까 */
    static const ucschar e_jjalb[]= {0xC9E7, 0};           /* 짧 */
    /* codex-spec §7. */
    static const ucschar e_yenil[]= {0xC608, 0xB2D0, 0xACF1, 0}; /* 예닐곱 */
    static const ucschar e_han[]  = {0xD55C, 0};           /* 한 */
    static const ucschar e_geul[] = {0xAE00, 0};           /* 글 */
    static const ucschar e_gwa[]  = {0xACFC, 0};           /* 과 */
    static const ucschar e_wo[]   = {0xC6CC, 0};           /* 워 */
    static const ucschar e_ui[]   = {0xC758, 0};           /* 의 */
    static const ucschar e_gak[]  = {0xAC01, 0};           /* 각 */
    static const ucschar e_gaks[] = {0xAC03, 0};           /* 갃 */
    /* symbol anchors (both specs). */
    static const ucschar e_mdot[] = {0x00B7, 0};           /* \xC2\xB7 middle dot */
    static const ucschar e_wdot[] = {0x25E6, 0};           /* white bullet */
    static const ucschar e_bull[] = {0x2022, 0};           /* bullet */
    static const ucschar e_hell[] = {0x2026, 0};           /* horizontal ellipsis */
    static const ucschar e_mult[] = {0x00D7, 0};           /* multiplication x */
    static const ucschar e_refm[] = {0x203B, 0};           /* reference mark */

    struct { const char* id; const char* keys; const ucschar* exp; const char* note; } A[] = {
        /* claude */
        { "C-A01", "kf",     e_ga,    "claude ga" },
        { "C-A02", "jfx",    e_at,    "claude at" },
        { "C-A03", "jdwc",   e_ilg,   "claude ilg (c=jong g)" },
        { "C-A04", "mids",   e_huin,  "claude huin (i=right eu)" },
        { "C-A05", "m/fw",   e_hwal,  "claude hwal (/=right o)" },
        { "C-A06", "morw",   e_hwol,  "claude hwol (o=right u)" },
        { "C-A07", "kkf",    e_kka,   "claude kka" },
        { "C-A08", "llfwe",  e_jjalb, "claude jjalb (corrected: jong b=e; ; is cho b)" },
        { "C-A09", "jps",    NULL,    "claude opt-hangul on" },
        { "C-A10", "jpds",   NULL,    "claude opt-hangul oin" },
        /* codex */
        { "X-A01", "jshdwkve", e_yenil, "codex yenilgop" },
        { "X-A02", "mfs",    e_han,   "codex han" },
        { "X-A03", "kgw",    e_geul,  "codex geul" },
        { "X-A04", "k/f",    e_gwa,   "codex gwa (/=right o)" },
        { "X-A05", "jor",    e_wo,    "codex wo (o=right u)" },
        { "X-A06", "jid",    e_ui,    "codex ui (i=right eu)" },
        { "X-A07", "kfc",    e_gak,   "codex gak (c=jong g)" },
        { "X-A08", "kkf",    e_kka,   "codex kka" },
        { "X-A09", "kfcq",   e_gaks,  "codex gaks (jong g+s->gs)" },
        /* symbols */
        { "X-A10", "jkf",    e_mdot,  "codex sym 1" },
        { "X-A11", "jlf",    e_wdot,  "codex sym 2" },
        { "X-A12", "j;f",    e_bull,  "codex sym 3" },
        { "C-S1",  "jkg",    e_hell,  "claude sym ellipsis" },
        { "C-S2",  "jkv",    e_mult,  "claude sym mult" },
        { "C-S3",  "jl/",    e_refm,  "claude sym refmark" },
    };
    for (unsigned i = 0; i < sizeof(A)/sizeof(A[0]); ++i) {
        int ok = anchor(kbd, A[i].id, A[i].keys, A[i].exp, A[i].note);
        if (A[i].exp != NULL) { ++total; if (ok) ++pass; }
    }
    printf("  anchors(strict)=%d/%d PASS\n", pass, total);
    return pass == total;
}

/* Divergence #2: after a lone choseong (ㄱ via 'k'), does the lowercase right_oua
 * context vowel still fire post-2018-04-10? If active: 그/구/<ㄱ+araea>/고. If the
 * context path were removed, lowercase i/o/p would instead boundary-commit ㄱ and
 * start the key's choseong (ㅁ/ㅊ/ㅍ). 'v' is the LEFT ㅗ control (always active). */
static void divergence2(const char* kbd) {
    printf("\n=== DIV2 (lowercase right_oua context vowel after lone choseong) ===\n");
    static const ucschar e_geu[] = {0xADF8, 0}; /* 그 = right eu active */
    static const ucschar e_gu[]  = {0xAD6C, 0}; /* 구 = right u active */
    static const ucschar e_go[]  = {0xACE0, 0}; /* 고 = right o (kept in 2018) */
    anchor(kbd, "ki",  "ki",  e_geu, "i -> right EU ?");
    anchor(kbd, "ko",  "ko",  e_gu,  "o -> right U ?");
    anchor(kbd, "kp",  "kp",  NULL,  "p -> right ARAEA (observe)");
    anchor(kbd, "k/",  "k/",  e_go,  "/ -> right O (2018 kept)");
    anchor(kbd, "kv",  "kv",  e_go,  "v -> left O (control, always)");
    /* Uppercase Shift-direct: 2018 removed I/O/(P->;) -> should NOT vowel. */
    anchor(kbd, "kI",  "kI",  NULL,  "Shift+I removed 2018 (observe)");
    anchor(kbd, "kO",  "kO",  NULL,  "Shift+O removed 2018 (observe)");
    anchor(kbd, "kP",  "kP",  NULL,  "Shift+P -> ; in 2018 (observe)");
}

int main(int argc, char** argv) {
    const char* dir = (argc > 1) ? argv[1] : "../data/keyboards";
    const char* kbd = (argc > 2) ? argv[2] : "3shin-p2";

    printf("# nabicloud shin XML-load behavioral oracle\n");
    printf("# dir=%s kbd=%s\n", dir, kbd);

    unsigned loaded = hangul_keyboard_list_load_dir(dir);
    printf("# load_dir registered %u external keyboard(s)\n", loaded);

    const HangulKeyboard* kb = hangul_keyboard_list_get_keyboard(kbd);
    if (kb == NULL) {
        printf("# FATAL: keyboard id=%s not resolvable after load_dir\n", kbd);
        printf("# SHIN_XML_ORACLE_FAIL\n");
        return 2;
    }
    printf("# resolved id=%s type=%d via external XML\n", kbd, kb->type);

    printf("\n=== KBD id=%s ===\n", kbd);
    suite(kbd);

    int anchors_ok = anchors_p2(kbd);
    divergence2(kbd);

    printf("\n# %s\n", anchors_ok ? "SHIN_XML_ORACLE_ANCHORS_PASS" : "SHIN_XML_ORACLE_ANCHORS_PARTIAL");
    printf("# end\n");
    return 0;
}
