"use strict";
/* ════════════════════════════════════════════════════════════════════════════
   JS↔C 직렬화 differential (WU9 보강) — 의존 0(node 내장).

   editor_jsc_dump.exe 가 tests/_jsc/<id>.xml 에 써둔 C 정본 직렬화
   (editor_serialize.c) 출력을, JS 코어(editor-core.js)의 parseXml→serialize 가
   bytes 그대로 재현하는지 검증한다 → JS 직렬화 형식 == 정본 C 형식(전 자판).
   WU7(신세벌 P 단건 byte-parity)을 신세벌 9종으로 확장.

   build_editor_jsc_diff.bat 가 C 덤프 후 이 스크립트를 돌린다.
   ════════════════════════════════════════════════════════════════════════════ */

const fs = require("node:fs");
const path = require("node:path");
const core = require(path.join(__dirname, "..", "..", "..", "windows", "settings", "tests", "editor-core.js"));
const { serialize, parseXml, canonModel } = core;

const DIR = path.join(__dirname, "_jsc");

let files;
try {
  files = fs.readdirSync(DIR).filter((f) => f.endsWith(".xml")).sort();
} catch (e) {
  console.error("JSC_DIFF_FAIL: " + DIR + " 없음 — C 덤프(editor_jsc_dump) 선행 필요.");
  process.exit(1);
}
if (!files.length) { console.error("JSC_DIFF_FAIL: _jsc 비어있음."); process.exit(1); }

let fail = 0;
for (const f of files) {
  const cXml = fs.readFileSync(path.join(DIR, f), "utf8");
  let jsXml;
  try {
    jsXml = serialize(parseXml(cXml));
  } catch (e) {
    console.error("  [FAIL] " + f + " — JS parseXml/serialize 예외: " + e);
    fail++; continue;
  }
  if (jsXml !== cXml) {
    let i = 0;
    while (i < jsXml.length && i < cXml.length && jsXml[i] === cXml[i]) { i++; }
    console.error("  [FAIL] " + f + " — byte 불일치 @" + i +
      "\n    C : " + JSON.stringify(cXml.slice(Math.max(0, i - 18), i + 28)) +
      "\n    JS: " + JSON.stringify(jsXml.slice(Math.max(0, i - 18), i + 28)));
    fail++; continue;
  }
  // 의미 동치(round-trip 안정) 보강
  if (canonModel(parseXml(cXml)) !== canonModel(parseXml(jsXml))) {
    console.error("  [FAIL] " + f + " — canonModel 불일치");
    fail++; continue;
  }
  console.log("  [PASS] " + f + " (JS==C, " + cXml.length + "B)");
}

if (fail) { console.error("JSC_DIFF_FAIL: " + fail + "/" + files.length); process.exit(1); }
console.log("JSC_DIFF_PASS: " + files.length + " 신세벌 (JS serialize == C serialize, byte-동일)");
