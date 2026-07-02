#!/usr/bin/env python3
# golden_pristine9_anchor.py — F-3 골든 재베이스 안전 앵커 (Codex 교차검증 F1 반영, 2026-06-26).
#
# 목적: de-fork 가 포크 빌트인 8 을 제거하면 golden_all.txt 가 재생성(rebase)되는데, 그때 *순정 9 빌트인*
#   (id ∈ {2,2y,39,3f,3s,3y,32,ro,ahn}) 출력이 1바이트라도 바뀌면 = de-fork 가 순정 코드를 오염시킨 증거다.
#   ★Codex F1: golden_all.c 는 *배열 인덱스*(idx)로 덤프하므로 빌트인 제거 시 idx 가 재정렬된다 → "idx0-8 대조"는
#   엉뚱한 자판을 통과시킨다. 따라서 앵커는 **id+cfg 키**로 저장하고 idx 는 비교에서 *정규화 제외*한다.
#
# 사용:
#   capture: python golden_pristine9_anchor.py capture golden_all.txt golden_pristine9.anchor
#            (빌트인 제거 *전* 1회 — 순정9 블록을 id+cfg 키·idx-정규화로 보관)
#   verify : python golden_pristine9_anchor.py verify  golden_all.txt golden_pristine9.anchor
#            (rebase *후* — 재생성된 golden 의 순정9 블록이 앵커와 byte-동일이면 exit0, 회귀면 exit1)
#
# exit: 0=순정9 불변(OK) / 1=순정9 회귀(FAIL, 작업 롤백) / 2=순정9 일부 부재(빌트인 set 손상)
import sys, re

PRISTINE9 = {'2', '2y', '39', '3f', '3s', '3y', '32', 'ro', 'ahn'}
HDR = re.compile(r'^=== KBD idx=(\d+) id=(\S+) cfg=(\S+) ===\s*$')


def parse_pristine_blocks(path):
    """golden_all.txt → {(id,cfg): block_text}  (id ∈ PRISTINE9 만, 헤더 idx 는 '*' 로 정규화)."""
    with open(path, 'r', encoding='utf-8', newline='') as f:
        lines = f.read().replace('\r\n', '\n').replace('\r', '\n').split('\n')
    blocks, key, cur = {}, None, []

    def flush():
        if key is not None and key[0] in PRISTINE9:
            # ★트레일러-강건(2026-06-27, F-3 R4): 각 블록은 다음 '=== KBD' 헤더의 분리 빈줄을
            #   흡수하고, *파일 마지막* 블록은 '# end' 트레일러까지 흡수한다(블록 종료자=다음 헤더
            #   or EOF). de-fork 로 순정9 가 파일 끝이 되면 마지막 블록(ahn-nc1)이 빈줄→'# end' 로
            #   바뀌어 *내용 무변경인데도* 오탐 FAIL 한다(앵커가 검증하려던 바로 그 시나리오의 함정).
            #   → 블록 끝의 빈줄·'# end' 를 제거해 자판 suite 본문만 키로 삼는다(앞·뒤 동일 정규화).
            c = list(cur)
            while c and c[-1] in ('', '# end'):
                c.pop()
            blocks[key] = '\n'.join(c)

    for ln in lines:
        m = HDR.match(ln)
        if m:
            flush()
            kid, cfg = m.group(2), m.group(3)
            key = (kid, cfg)
            cur = ['=== KBD idx=* id=%s cfg=%s ===' % (kid, cfg)]  # idx 정규화
        elif key is not None:
            cur.append(ln)
    flush()
    return blocks


def render(blocks):
    return '\n'.join(blocks[k] for k in sorted(blocks)) + '\n'


def main():
    if len(sys.argv) != 4 or sys.argv[1] not in ('capture', 'verify'):
        print('usage: golden_pristine9_anchor.py {capture|verify} <golden_all.txt> <anchor>')
        return 2
    mode, golden, anchor = sys.argv[1], sys.argv[2], sys.argv[3]
    blocks = parse_pristine_blocks(golden)
    ids = {k[0] for k in blocks}
    missing = PRISTINE9 - ids
    if missing:
        print('PRISTINE9_MISSING:', sorted(missing), '— 순정 빌트인 set 손상(de-fork 가 순정 제거?)')
        return 2
    rendered = render(blocks)
    if mode == 'capture':
        with open(anchor, 'w', encoding='utf-8', newline='\n') as f:
            f.write(rendered)
        print('PRISTINE9_ANCHOR_CAPTURED ids=%d blocks=%d' % (len(ids), len(blocks)))
        return 0
    # verify
    with open(anchor, 'r', encoding='utf-8', newline='') as f:
        want = f.read().replace('\r\n', '\n').replace('\r', '\n')
    if rendered == want:
        print('PRISTINE9_ANCHOR_OK blocks=%d (순정9 byte-불변)' % len(blocks))
        return 0
    print('PRISTINE9_ANCHOR_FAIL — 순정9 회귀(de-fork 가 순정 코드 오염?). 작업 롤백 권고.')
    wl, rl = want.split('\n'), rendered.split('\n')
    for i in range(min(len(wl), len(rl))):
        if wl[i] != rl[i]:
            print('  first diff @line %d:\n   anchor: %r\n   now   : %r' % (i, wl[i][:140], rl[i][:140]))
            break
    return 1


if __name__ == '__main__':
    sys.exit(main())
