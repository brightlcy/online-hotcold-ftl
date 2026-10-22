# Test strategy

Use C++20 + pinned vendored Catch2 v2.13.10 + CTest, no configure-time network. Each atomic unit writes interface/invariants and failing tests first, preserves RED evidence, implements and executes GREEN, checks sanitizers/static analysis, then commits with checkpoint. Source warnings are errors. Python original is frozen and is not assumed infallible. Deterministic property tests record seeds in source and failure context. No tests are skipped to get green.

Fast: scripts/run_fast_tests.sh. Extended: scripts/run_full_validation.sh (sanitizer; relevant Python differential/property tests as introduced). Existing expensive final historical runs are not repeated without new evidence. Negative test failures during RED are expected development evidence, not a final green run. Coverage target 90% lines/80% branches for core state; report missing edges. CTest number counts test cases, Catch2 assertion count counts checks, neither proves scientific validity.
