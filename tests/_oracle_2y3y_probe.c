/* _oracle_2y3y_probe.c -- libhangul 2y(두벌식 옛글)·3y(세벌식 3-93 옛한글) 블랙박스 오라클 덤프.
 *
 * 클린룸 ground truth: libhangul 소스 테이블(hangulkeyboard.h)을 *읽지 않고*, 오라클의 *출력*만
 *   관찰한다 — (1) 단일키 sweep(키→자모 = 키맵 재구성용), (2) 조합 corpus(결합표 재구성용).
 *   이 덤프를 보고 V2 jaso_layout_2y/3y 를 작성하고, 같은 corpus 로 byte-diff 검증한다.
 *   (출력 관찰 기반 = 프로젝트의 "거동 재구현" 클린룸 방법. diff_sebeol.c 포맷 재사용.)
 * ASCII-only. libhangul 링크.
 */
#include <stdio.h>
#include <string.h>
#include "hangul.h"

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

/* 단일키 sweep: 각 ASCII 를 새 IC 에 1회만 눌러 preedit/commit/flush 를 본다 = 그 키의 자모. */
static void sweep_single(const char* kbd) {
    printf("--- single-key sweep id=%s ---\n", kbd);
    for (unsigned ch = 0x21; ch <= 0x7E; ++ch) {
        char cbuf[128], pbuf[128], fbuf[128];
        HangulInputContext* hic = hangul_ic_new(kbd);
        if (hic == NULL) { printf("FAIL new==NULL id=%s\n", kbd); return; }
        int processed = hangul_ic_process(hic, (int)ch) ? 1 : 0;
        fmt(hangul_ic_get_commit_string(hic), cbuf, sizeof(cbuf));
        fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
        fmt(hangul_ic_flush(hic), fbuf, sizeof(fbuf));
        printf("  '%c' 0x%02X p=%d commit=%s pre=%s flush=%s\n",
               (ch >= 0x20 && ch < 0x7F) ? (char)ch : '?', ch, processed, cbuf, pbuf, fbuf);
        hangul_ic_delete(hic);
    }
}

/* 조합 corpus: 시퀀스 입력 후 키별 commit/preedit + 최종 flush. = 결합표/조합 거동. */
static void run_str(const char* kbd, const char* name, const char* s) {
    char cbuf[256], pbuf[256];
    HangulInputContext* hic = hangul_ic_new(kbd);
    if (hic == NULL) { printf("T %s FAIL new==NULL\n", name); return; }
    printf("T %s\n", name);
    for (const char* p = s; *p; ++p) {
        int processed = (*p == '\b') ? (hangul_ic_backspace(hic) ? 1 : 0)
                                     : (hangul_ic_process(hic, (unsigned char)*p) ? 1 : 0);
        fmt(hangul_ic_get_commit_string(hic), cbuf, sizeof(cbuf));
        fmt(hangul_ic_get_preedit_string(hic), pbuf, sizeof(pbuf));
        printf("  k=0x%02X p=%d commit=%s pre=%s\n", (unsigned)(unsigned char)*p, processed, cbuf, pbuf);
    }
    fmt(hangul_ic_flush(hic), cbuf, sizeof(cbuf));
    printf("  flush=%s\n", cbuf);
    hangul_ic_delete(hic);
}

static void suite(const char* kbd) {
    /* 기본 CV/CVC (소문자) */
    run_str(kbd, "kf", "kf"); run_str(kbd, "rk", "rk"); run_str(kbd, "rkt", "rkt");
    run_str(kbd, "gksrmf", "gksrmf"); run_str(kbd, "dkssud", "dkssud");
    /* 된소리/겹: doubles */
    run_str(kbd, "rr", "rr"); run_str(kbd, "tt", "tt"); run_str(kbd, "rkqq", "rkqq");
    run_str(kbd, "rkrk", "rkrk"); run_str(kbd, "fff", "fff");
    /* 겹홀: vowel after vowel */
    run_str(kbd, "rhk", "rhk"); run_str(kbd, "rnj", "rnj");
    /* Shift 행(옛한글 자모 추정) — 대문자/기호 전수는 sweep 가 덮지만, 조합 거동 확인 */
    run_str(kbd, "RKF", "RKF"); run_str(kbd, "kRf", "kRf"); run_str(kbd, "Ee", "Ee");
    /* 기호/숫자 */
    run_str(kbd, "punct", ";',.[]/"); run_str(kbd, "digits", "0123456789");
    /* backspace */
    run_str(kbd, "bs_cv", "rk\b"); run_str(kbd, "bs_cvc", "rkq\b\b"); run_str(kbd, "bs_over", "rk\b\b\b");
}

int main(void) {
    static const char* kbds[] = { "2y", "3y" };
    printf("# nabicloud 2y/3y libhangul oracle dump\n");
    for (unsigned i = 0; i < sizeof(kbds)/sizeof(kbds[0]); ++i) {
        printf("\n=== KBD id=%s ===\n", kbds[i]);
        sweep_single(kbds[i]);
        suite(kbds[i]);
    }
    printf("# end\n");
    return 0;
}
