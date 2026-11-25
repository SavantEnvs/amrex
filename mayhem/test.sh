#!/usr/bin/env bash
# mayhem/test.sh — run AMReX's own Tests/{Parser,Parser2,ParmParse,TOML,Enum} programs (built by
# build.sh). Each test asserts known answers internally (AMREX_ALWAYS_ASSERT / Abort) and prints a
# success line only after every check passed; a test passes iff it exits 0 AND prints that line.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SRC:=/mayhem}"
cd "$SRC"
emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}
T="$SRC/mayhem/build/tests/Tests"
passed=0; failed=0
# run <name> <dir> <expected regex> <cmd...>
run() {
  local name="$1" dir="$2" expect="$3"; shift 3
  if [ ! -x "$dir/$1" ]; then echo "MISSING test binary $dir/$1 (build.sh bug)"; failed=$((failed+1)); return; fi
  local out rc
  out=$(cd "$dir" && timeout 300 "$@" 2>&1); rc=$?
  if [ $rc -eq 0 ] && grep -Eq "$expect" <<<"$out"; then echo "PASS $name"; passed=$((passed+1))
  else echo "FAIL $name (rc=$rc)"; tail -20 <<<"$out"; failed=$((failed+1)); fi
}
run Parser    "$T/Parser/3d"    "All IParser tests passed" ./Test_Parser_3d
run Parser2   "$T/Parser2/3d"   "All [0-9]+ tests passed"  ./Test_Parser2_3d
run ParmParse "$T/ParmParse/3d" "^SUCCESS"                 ./Test_ParmParse_3d inputs unset_cmdline_test=1 UNSET=unset_from_cmdline
run TOML      "$T/TOML/3d"      "."                        ./Test_TOML_3d config.toml
run Enum      "$T/Enum/3d"      "^SUCCESS"                 ./Test_Enum_3d inputs
emit_ctrf amrex-tests "$passed" "$failed" 0
