#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# runtime_checklist_catalog.py -- T5 (2026-07-23, 59번): thin delegating shim.
#
# The real implementation moved to raindrop_testsys/jaso_catalog.py (absorbed into the shared test
# package, docs/TEST_CHECKLIST_AUTOMATION_DESIGN.md sec 13). This file stays at the SAME path so
# existing diagnostic call sites keep working. The retired check command is no longer delegated.
# Prefer the package entry point for anything new: `python -m raindrop_testsys <cmd> --root <repo>`
# (see shared/engine/tests/README.md). Output/exit-code parity with the pre-absorption version was
# verified byte-for-byte before this file was reduced to a shim.
import os
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.normpath(os.path.join(_HERE, "..", "..", ".."))
sys.path.insert(0, os.path.join(_ROOT, "raindrop-runtime", "sdk", "test", "python"))

from raindrop_testsys import jaso_catalog  # noqa: E402

_CMDS = {
    "catalog": jaso_catalog.cmd_catalog, "select": jaso_catalog.cmd_select,
    "census": jaso_catalog.cmd_census,
    "collect": jaso_catalog.cmd_collect, "verify": jaso_catalog.cmd_verify,
    "pairs": jaso_catalog.cmd_pairs,
}


def main():
    if len(sys.argv) < 2 or sys.argv[1] not in _CMDS:
        sys.stderr.write("사용: python runtime_checklist_catalog.py "
                         "{catalog|select|census|collect|verify|pairs} [args]\n")
        sys.stderr.write("(정본 = raindrop_testsys/jaso_catalog.py -- 이 파일은 위임 shim)\n")
        return 2
    return _CMDS[sys.argv[1]](_ROOT, sys.argv[2:])


if __name__ == "__main__":
    sys.exit(main())
