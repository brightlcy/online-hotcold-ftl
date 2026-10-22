# ATLAS-SSD test status

2026-10-22: finalizer/reproduction-safety suite 9 PASS, 0 FAIL, 0 SKIP. Existing completed Alibaba output finalization PASS for 3 devices × 24 metrics and six phase/config checks; no simulation rerun. Test framework/C++ build/sanitizer/coverage: NOT RUN.

Prior Python 139/215 tests and final 220-run byte comparison are historical verified records; not rerun in this session. MSR: BLOCKED (missing extract). MQSim: NOT RUN.

NAND-01: 11/11 CTest PASS normal and ASan/UBSan (0 skipped); 1500 deterministic mixed operations in property case. clang-format 20.1.8 PASS. Clang static analyzer: 0 diagnostics (clang-tidy unavailable). LLVM core cpp coverage: 81/83 lines, 37/38 branch outcomes. Uncovered: impractical uint64 erase-counter-overflow throw. RED 11/11 failed before implementation; stubs initially failed unused-field -Werror and were adjusted without disabling warnings. Logs and source hashes preserved. Next mapping unit not yet started.
