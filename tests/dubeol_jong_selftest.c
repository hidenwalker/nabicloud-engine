/* RETIRED (dead archive, not built) -- see tests/RETIRED.md. Requires the old engine
   XML/editor path removed in F-3 R4 option 4 (2026-06-29); no longer meaningful for the
   target architecture. */
/*
 * NabiCloud 두겹이 겹받침 자동전환 (Dugyeob-e auto cluster-jong) self-test.
 * (DUBEOL_CHORD_DESIGN.md §4 / chord refactor step (d) gate.)
 *
 * 본가 Sinseiki/Dugyeob-e README 정본: "초성과 중성을 입력하면 I, J, K, L 자판이
 * 겹받침 ㅆ, ㄶ, ㅄ, ㄺ 로 일시적으로 자동 변경된다." 이 테스트는 실제 두겹이 자판
 * (type dubeol-chord/1007)을 로드하고 ASCII 키를 hangul_ic_process 로 흘려(= 셸의
 * 순차 키 경로: hangul_ic_process -> nabicloud_dispatch_process -> ops_dubeol_chord_
 * process) commit++preedit 결과를 기대 음절과 대조한다.
 *
 * 전환 조건 검증: (1) cho+jung 찬 음절에서 I/J/K/L → 겹받침(값·닭·많·있), (2) ★이중모음
 * 우선 — 현 중성과 그 모음이 이중모음을 이루면 변환 안 함(과=ㅗ+ㅏ, 의=ㅡ+ㅣ), (3) cho/jung
 * 미완성이면 변환 안 함(lone ㅏ). 두겹이 자판이 없으면(로더 미배선) IC type!=1007 라
 * ops 미경유 → 겹받침 안 나와 FAIL = (d) 회귀 가드. ASCII-only. Exit 0 = PASS.
 */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "hangul.h"
#include "hangulinternals.h"

static int g_fail = 0;
static const char* g_kbId = "dugyeobe";   /* test_keys 가 쓰는 자판 id (두겹이/두줄이 공용) */

static int uccmp(const ucschar* a, const ucschar* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (int)(*a) - (int)(*b);
}

static void fmt(const ucschar* s, char* out, size_t cap) {
    size_t k = 0;
    if (s == NULL) { snprintf(out, cap, "(null)"); return; }
    if (*s == 0)   { snprintf(out, cap, "(empty)"); return; }
    out[0] = '\0';
    while (*s && k + 8 < cap) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "%sU+%04X", (k ? " " : ""), (unsigned)*s);
        strncat(out, tmp, cap - strlen(out) - 1);
        k += strlen(tmp); ++s;
    }
}

/* Process each ASCII key in `keys` through a fresh 두겹이 IC, then compare the
 * produced text (commit ++ preedit) to `expect`. */
static void test_keys(const char* name, const char* keys, const ucschar* expect) {
    HangulInputContext* hic = hangul_ic_new(g_kbId);
    ucschar full[64]; char got[64], exp[64];
    const ucschar* commit; const ucschar* pre; int w = 0; size_t i;
    if (hic == NULL) { printf("  [FAIL] %s: hic==NULL\n", name); g_fail++; return; }
    for (i = 0; i < strlen(keys); ++i)
        hangul_ic_process(hic, (int)(unsigned char)keys[i]);
    commit = hangul_ic_get_commit_string(hic);
    pre    = hangul_ic_get_preedit_string(hic);
    if (commit) for (i = 0; commit[i] && w + 1 < (int)(sizeof(full)/sizeof(full[0])); i++) full[w++] = commit[i];
    if (pre)    for (i = 0; pre[i]    && w + 1 < (int)(sizeof(full)/sizeof(full[0])); i++) full[w++] = pre[i];
    full[w] = 0;
    fmt(full, got, sizeof(got)); fmt(expect, exp, sizeof(exp));
    if (uccmp(full, expect) != 0) {
        printf("  [FAIL] %s (keys=%s): got %s, expect %s\n", name, keys, got, exp); g_fail++;
    } else {
        printf("  [ok]   %s (keys=%s) -> %s\n", name, keys, got);
    }
    hangul_ic_delete(hic);
}

int main(int argc, char** argv) {
    const char* dir = (argc > 1) ? argv[1] : "../../addons/dubeol/keyboards";
    unsigned loaded;
    const HangulKeyboard* kb;

    printf("== 두겹이 겹받침 자동전환 self-test (DUBEOL_CHORD_DESIGN s4) ==\n");
    printf("# dir=%s\n", dir);
    loaded = hangul_keyboard_list_load_dir(dir);
    kb = hangul_keyboard_list_get_keyboard("dugyeobe");
    printf("# loaded=%u  dugyeobe type=%d (want %d)\n",
           loaded, kb ? kb->type : -1, HANGUL_KEYBOARD_TYPE_DUBEOL_CHORD);

    /* ★2026-06-25 두겹이 V2 이관: dugyeobe.xml 이 V2 확장 XML(<jaso engine="v2"> + <jaso-map>)이면
     *   libhangul keymap(table[0])이 없어 이 libhangul 경로 테스트는 무의미하다 — V2 fill-jong/chord 는
     *   cleanroom selftest_jaso_filljong(14/14)·selftest_jaso_dubeol_chord(13/13)가 커버. dugyeobe 가
     *   libhangul-body 일 때만 아래 블록 실행. 두줄이(dujul-e, 아직 libhangul)는 그대로 검증. */
    int dg_v2 = (kb != NULL && kb->table[0] == NULL);
    if (dg_v2)
        printf("# dugyeobe = V2 확장 XML (libhangul table 없음) → libhangul fill-jong 테스트 skip "
               "(V2=selftest_jaso_filljong/dubeol_chord 커버)\n");

    /* 두벌식 키: r=ㄱ e=ㄷ a=ㅁ d=ㅇ h=ㅗ m=ㅡ k=ㅏ l=ㅣ i=ㅑ j=ㅓ.
     * 겹받침 트리거(cho+jung 후): i=ㅆ j=ㄶ k=ㅄ l=ㄺ. */
    if (!dg_v2) {
        /* (1) 값 = ㄱ+ㅏ+[ㅄ] : r,k 로 '가' 만든 뒤 k → ㅏ+ㅏ 이중모음 불가 → ㅄ. */
        {   ucschar exp[] = { 0xAC12, 0 }; test_keys("gap", "rkk", exp); }      /* 값 */
        /* (2) 닭 = ㄷ+ㅏ+[ㄺ] : e,k 로 '다' 뒤 l → ㅏ+ㅣ 이중모음 불가 → ㄺ. */
        {   ucschar exp[] = { 0xB2ED, 0 }; test_keys("dak", "ekl", exp); }      /* 닭 */
        /* (3) 많 = ㅁ+ㅏ+[ㄶ] : a,k 로 '마' 뒤 j → ㅏ+ㅓ 이중모음 불가 → ㄶ. */
        {   ucschar exp[] = { 0xB9CE, 0 }; test_keys("manh", "akj", exp); }     /* 많 */
        /* (4) 있 = ㅇ+ㅣ+[ㅆ] : d,l 로 '이' 뒤 i → ㅣ+ㅑ 이중모음 불가 → ㅆ. */
        {   ucschar exp[] = { 0xC788, 0 }; test_keys("iss", "dli", exp); }      /* 있 */
        /* (5) ★이중모음 우선: 과 = ㄱ+ㅗ+ㅏ(k) : ㅗ+ㅏ=ㅘ 결합 → 변환 안 함 → 과. */
        {   ucschar exp[] = { 0xACFC, 0 }; test_keys("gwa(diphthong)", "rhk", exp); }  /* 과 */
        /* (6) ★이중모음 우선: 의 = ㅇ+ㅡ+ㅣ(l) : ㅡ+ㅣ=ㅢ 결합 → 변환 안 함 → 의. */
        {   ucschar exp[] = { 0xC758, 0 }; test_keys("ui(diphthong)", "dml", exp); }   /* 의 */
        /* (7) cho/jung 미완성 → 변환 안 함: k 단독 → lone ㅏ(호환자모 U+314F). */
        {   ucschar exp[] = { 0x314F, 0 }; test_keys("lone-a (no cho+jung)", "k", exp); }
    }

    /* ── 두줄이 (Dujul-e) 줄맞춤 배열 ── 본가 Sinseiki/Dujul-e. 같은 엔진(코드 무변경)·
     *    다른 데이터(dujul-e.xml 의 키맵 + key-context). fill-jong i=ㅄ j=ㄶ k=ㄺ l=ㄲ
     *    (두겹이 ㅆ/ㄶ/ㅄ/ㄺ 와 다름). 두줄이 키: f=ㄱ e=ㄷ a=ㅁ d=ㅇ q=ㅂ h=ㅗ j=ㅡ k=ㅏ l=ㅣ. */
    int dj_v2 = 0;
    {
        const HangulKeyboard* k2 = hangul_keyboard_list_get_keyboard("dujul-e");
        printf("# dujul-e type=%d (want %d)\n",
               k2 ? k2->type : -1, HANGUL_KEYBOARD_TYPE_DUBEOL_CHORD);
        if (k2 == NULL || k2->type != HANGUL_KEYBOARD_TYPE_DUBEOL_CHORD) {
            printf("  [FAIL] dujul-e 미로드 또는 type!=dubeol-chord\n"); g_fail++;
        }
        /* ★2026-06-25 두줄이 V2 이관: dujul-e.xml 도 V2 확장 XML(<jaso engine="v2"> + <jaso-map>)이라
         *   libhangul keymap(table[0])이 없다 — 두겹이와 동형. 아래 libhangul 경로 fill-jong 테스트는
         *   무의미하므로 skip하고, V2 fill-jong 값(ㅄ/ㄶ/ㄺ/ㄲ)은 cleanroom selftest_jaso_filljong 이 커버. */
        dj_v2 = (k2 != NULL && k2->table[0] == NULL);
        if (dj_v2)
            printf("# dujul-e = V2 확장 XML (libhangul table 없음) → libhangul fill-jong 테스트 skip "
                   "(V2=selftest_jaso_filljong 두줄이 값 커버)\n");
    }
    if (!dj_v2) {
        g_kbId = "dujul-e";
        {   ucschar exp[] = { 0xAC12, 0 }; test_keys("dujul gap",  "fki", exp); }  /* 값 = ㄱ+ㅏ+ㅄ(i) */
        {   ucschar exp[] = { 0xB2ED, 0 }; test_keys("dujul dak",  "ekk", exp); }  /* 닭 = ㄷ+ㅏ+ㄺ(k) */
        {   ucschar exp[] = { 0xB9CE, 0 }; test_keys("dujul manh", "akj", exp); }  /* 많 = ㅁ+ㅏ+ㄶ(j) */
        {   ucschar exp[] = { 0xBC16, 0 }; test_keys("dujul bakk", "qkl", exp); }  /* 밖 = ㅂ+ㅏ+ㄲ(l) */
        {   ucschar exp[] = { 0xACFC, 0 }; test_keys("dujul gwa(diphthong)", "fhk", exp); }  /* 과: ㅗ+ㅏ=ㅘ 우선 */
        {   ucschar exp[] = { 0xC758, 0 }; test_keys("dujul ui(diphthong)",  "djl", exp); }  /* 의: ㅡ+ㅣ=ㅢ 우선 */
        {   ucschar exp[] = { 0x314F, 0 }; test_keys("dujul lone-a", "k", exp); }  /* k 단독 → lone ㅏ */
    }

    if (g_fail == 0) { printf("DUBEOL_JONG_SELFTEST_PASS\n"); return 0; }
    printf("DUBEOL_JONG_SELFTEST_FAIL (%d)\n", g_fail); return 1;
}
