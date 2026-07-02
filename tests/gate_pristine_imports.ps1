# R0 Pristine Import Gate (parser) -- classify NabiCloud.dll's libhangul.dll imports.
#   Audit mode (default): report forbidden non-pristine imports, exit 0.
#   Enforce mode (-Enforce, used at R4): exit 1 if any forbidden import remains.
# Forbidden = the non-pristine ABI being cut in R1~R3:
#   load_dir / process_with_capslock / has_convertible_syllable / table_load_wide /
#   map_to_char / get_type / combine / get_flag / nabicloud_* / editor_*
param(
  [Parameter(Mandatory=$true)][string]$ImportsFile,
  [switch]$Enforce
)

if (-not (Test-Path -LiteralPath $ImportsFile)) {
  Write-Output "GATE_PRISTINE_IMPORTS_FAIL: imports file not found: $ImportsFile"
  exit 1
}

$lines = Get-Content -LiteralPath $ImportsFile
$inBlock = $false
$syms = New-Object System.Collections.Generic.List[string]
foreach ($ln in $lines) {
  if ($ln -match '^\s*libhangul\.dll\s*$') { $inBlock = $true; continue }
  if ($inBlock) {
    if ($ln -match '^\s*\S+\.dll\s*$') { $inBlock = $false; continue }
    if ($ln -match '^\s+[0-9A-Fa-f]+\s+([A-Za-z_][A-Za-z0-9_]*)\s*$') { $syms.Add($Matches[1]) }
  }
}

if ($syms.Count -eq 0) {
  Write-Output "GATE_PRISTINE_IMPORTS_WARN: no libhangul.dll imports parsed (check dumpbin output / dll path)"
}

$forbidden = @(
  '^hangul_keyboard_list_load_dir$',
  '^hangul_ic_process_with_capslock$',
  '^hangul_ic_has_convertible_syllable$',
  '^hanja_table_load_wide$',
  '^hangul_keyboard_map_to_char$',
  '^hangul_keyboard_get_type$',
  '^hangul_keyboard_combine$',
  '^hangul_keyboard_get_flag$',
  '^nabicloud_',
  '^editor_'
)

$hits = New-Object System.Collections.Generic.List[string]
foreach ($s in $syms) {
  foreach ($f in $forbidden) { if ($s -match $f) { $hits.Add($s); break } }
}
$hits = @($hits | Sort-Object -Unique)

Write-Output ("R0 PRISTINE IMPORT GATE -- libhangul.dll imports: {0} symbols, forbidden(non-pristine): {1}" -f $syms.Count, $hits.Count)
if ($hits.Count -gt 0) {
  Write-Output "  forbidden imports remaining (cut targets R1~R3):"
  foreach ($h in $hits) { Write-Output ("    - {0}" -f $h) }
}

if ($hits.Count -eq 0) {
  Write-Output "GATE_PRISTINE_IMPORTS_PASS (0 forbidden)"
  exit 0
}
if ($Enforce) {
  Write-Output ("GATE_PRISTINE_IMPORTS_FAIL (enforce): {0} forbidden import(s)" -f $hits.Count)
  exit 1
}
Write-Output ("GATE_PRISTINE_IMPORTS_AUDIT: {0} forbidden import(s) (audit mode -> exit 0; enforce at R4)" -f $hits.Count)
exit 0
