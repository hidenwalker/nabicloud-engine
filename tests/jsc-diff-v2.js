"use strict";
/* ════════════════════════════════════════════════════════════════════════════
   JS↔C 직렬화 differential V2판 (2026-07-02 재신설) — 의존 0(node 내장).

   은퇴한 jsc-diff.js(구 스키마 parseXml/serialize, RETIRED build_editor_jsc_diff)
   의 V2 후계. editor_jsc_dump_v2.exe 가 tests/_jsc_v2/<id>.xml 에 써둔
   "C 정본 V2 직렬화(jaso_editor_serialize_by_id→jaso_xml_dump) + 셸 메타 주입
   (SettingsWebView editorLoad 복제)" 바이트를, JS 코어(editor-core.js)의
   parseKeyboardXml→serializeKeyboard 가 bytes 그대로 재현하는지 검증한다
   → JS V2 직렬화 형식 == 정본 C 형식(빌트인 kJasoBuiltin 14종 전체).

   build_editor_jsc_diff_v2.bat 가 C 덤프 후 이 스크립트를 돌린다.
   ════════════════════════════════════════════════════════════════════════════ */

const fs = require("node:fs");
const path = require("node:path");
const core = require(path.join(__dirname, "..", "..", "..", "windows", "settings", "tests", "editor-core.js"));
const { parseKeyboardXml, serializeKeyboard, canonJasoModel, isV2Xml, isVmXml } = core;

const DIR = path.join(__dirname, "_jsc_v2");
const MIN = 14;   // kJasoBuiltin 14종(덤프가 조용히 줄면 드리프트) — editor_jsc_dump_v2.cpp kKb[]

let files;
try {
  files = fs.readdirSync(DIR).filter((f) => f.endsWith(".xml")).sort();
} catch (e) {
  console.error("JSC_DIFF_V2_FAIL: " + DIR + " 없음 — C 덤프(editor_jsc_dump_v2) 선행 필요.");
  process.exit(1);
}
if (files.length < MIN) {
  console.error("JSC_DIFF_V2_FAIL: _jsc_v2 에 " + files.length + "개 (< " + MIN + ") — 덤프 축소 드리프트.");
  process.exit(1);
}

let fail = 0;
for (const f of files) {
  const cXml = fs.readFileSync(path.join(DIR, f), "utf8");
  if (!isV2Xml(cXml) || isVmXml(cXml)) {
    console.error("  [FAIL] " + f + " — V2 jaso 로 감지되지 않음(isV2Xml=" + isV2Xml(cXml) + ", isVmXml=" + isVmXml(cXml) + ")");
    fail++; continue;
  }
  let model, jsXml;
  try {
    model = parseKeyboardXml(cXml);          // 디스패처: V2 → parseJasoXml
    if (model._engine !== "v2") { throw new Error("dispatcher routed _engine=" + model._engine); }
    jsXml = serializeKeyboard(model);        // 디스패처: V2 → serializeJasoXml
  } catch (e) {
    console.error("  [FAIL] " + f + " — JS parse/serialize 예외: " + e);
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
  // 의미 동치(round-trip 안정) 보강 — 재파싱 정규형까지 동일해야.
  if (canonJasoModel(parseKeyboardXml(jsXml)) !== canonJasoModel(model)) {
    console.error("  [FAIL] " + f + " — canonJasoModel 불일치(재파싱 드리프트)");
    fail++; continue;
  }
  console.log("  [PASS] " + f + " (JS==C, " + cXml.length + "B)");
}

if (fail) { console.error("JSC_DIFF_V2_FAIL: " + fail + "/" + files.length); process.exit(1); }
console.log("JSC_DIFF_V2_PASS: " + files.length + " V2 builtin (JS serialize == C serialize+meta, byte-동일)");
