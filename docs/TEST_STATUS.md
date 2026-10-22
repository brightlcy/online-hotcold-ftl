# ATLAS-SSD test status — 2026-10-22

Tested source: 400ec1e88fc9c5f30f354b59257c3310d73ba95b. Subsequent checkpoint-only changes preserve source digests. No tests were deleted/skipped to get green.

| Check | Actual result | Evidence |
|---|---|---|
| macOS ARM64 C++20 debug build / warnings-as-errors | PASS | 20261022-gc-green.log |
| CTest normal | 28 PASS, 0 FAIL, 0 SKIP | 27 C++ cases + Python differential |
| CTest ASan + UBSan | 28 PASS, 0 FAIL, 0 SKIP | 20261022-gc-green.log |
| Linux GCC 13.3 normal | 28/28 PASS | CI 39740543341 |
| Linux Clang 18.1.3 ASan + UBSan | 28/28 PASS | CI 39740543341 |
| Python committed mapping differential | 19 traces / 662 events PASS; altered mapping rejected | mapping-differential-green.log, normal/sanitizer CTest |
| Property/integration | 1500 NAND operations; 300 mapping attempts including exhaustion; 1000 writes/twin GC replay | seeds in tests, >100 GC cycles checked |
| clang-format 20.1.8 | PASS locally and Linux | gc-quality.log, CI |
| Clang static analyzer | 0 local diagnostics; Linux step PASS | gc-quality.json/log |
| clang-tidy | NOT RUN, unavailable | explicit limit |
| LLVM core cpp coverage | lines 214/224=95.54%; branch outcomes 113/122=92.62% | gc-quality.json |
| Finalizer safety locally | 9/9 PASS | initial session tool output, existing test suite |
| Preserved source/archive | PASS, no original changes | preservation-and-scan.json + CI |
| Existing Python core / smoke / analysis / figures | CI PASS; five analysis regression tests PASS | CI 39740543186 |
| Alibaba completed historical output finalization | 6 policies / 3 devices PASS, 24 metrics each + phase/config | rebuild_completed_hotcold_replay.json; simulation_executed=false |

Evidence filenames in this table are under results/validation/20261022- unless the path says otherwise. Full CI metadata and observed log excerpts are committed; full downloaded CI logs remain under ignored artifacts/ci/20261022. User's original operating prompts are in local attachments and not copied into new public artifacts.

Expected test-first failures retained: initial unused-private-field stub build failure, then NAND runtime RED 11 failed; mapping RED 8 new failed/11 previous passed; GC RED 8 new failed/20 previous CTest entries passed. These are development evidence; final runs have no failures. See TEST_FAILURE_LOG.md.

Not executed: clang-tidy, fuzzing, allocation-fault injection, GC/WAF differential equivalence, MQSim integration, new performance/holdout experiments. Uncovered branches include internally corrupted inverse-map audit throws and NAND/GC uint64 counter-overflow defense. Existing Host 215 tests/220-run replay are historical records, not rerun in this session.

Next implementation: define cumulative counters/WAF and wear-stat invariants, add failing unit tests, then implement and run sh scripts/run_fast_tests.sh followed by sanitizer/quality checks. Do not rerun historical studies.
