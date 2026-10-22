# Test failure log

2026-10-22: no unexpected failure in recovered finalizer suite (9/9). C++ NAND test-first RED is pending. Append actual command, reason and log; do not mark unexecuted tests PASS.

NAND RED: first build failed -Wunused-private-field under -Werror because stubs did not reference interface fields. Kept warnings enabled, made test stubs reference fields and rebuilt. Runtime RED then executed all 11 tests: 0 passed, 11 failed due to unimplemented geometry. Logs: results/validation/20261022-nand-red.log and 20261022-nand-red-runtime.log. This is expected development failure, not accepted final state.

NAND GREEN: all 11 cases pass normal and ASan/UBSan. No warning option or test was removed. Static analyzer and formatting pass; coverage records the remaining unreachable-in-practice overflow edge.

Mapping RED: normal build succeeded; 11 existing NAND cases passed, all 8 new mapping/property cases failed for the unimplemented constructor. Evidence: results/validation/20261022-mapping-red.log. Implementation follows only after that execution.

Mapping GREEN: 19 C++ cases plus Python differential pass normal and ASan/UBSan. No skipped/expected-failure final tests. Coverage/static/format checks pass.
