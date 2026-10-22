# Research changelog

## 2026-10-22 — recovered checkpoint

Read all three user operating prompts. Reconciled local checkpoint with retained worker and Notion. Worker had completed at 2026-10-20T06:21:40Z although summaries still said RUNNING. Finalized existing output on 2026-10-22 without simulation; all three devices pass 24 metrics and both policies’ phase/config checks. Kept legacy missing executed-digest limitation, MSR blocker and original raw output. Nine finalizer safety tests passed. Continuing existing repo on an isolated development branch.

AI assistance: Codex performed state recovery, report reconciliation and will generate C++ interfaces/tests/implementation. No independent human design/review or hardware work is claimed.

## NAND-01 verified

Added checked C++ geometry and NAND state/token model with Catch2/CTest. Recorded expected runtime RED then GREEN 11 cases, ASan/UBSan, deterministic 1500-operation reference-model property case, formatting and Clang static analysis. cpp coverage 97.59% lines / 97.37% branch outcomes; counter-overflow edge disclosed. AI generated design/tests/implementation; self-review is not independent validation. Historical source/archive unchanged. Next: mapping TDD.

## Mapping-01 verified

Implemented bounded L2P/P2L mapping after 8-test runtime RED; exhaustion preserves old data. Added payload oracle and event-level Python metadata comparison (19 traces, 662 committed events), negative control, sanitizer and quality evidence. Combined core coverage 94.44%/91.89%. No GC, WAF or timing equivalence is claimed. Next: greedy GC contract, preflight and migration integrity tests.

## GC-01 verified

Added deterministic greedy victim selection, explicit reserve trigger and migration/erase transaction after 8-test runtime RED. Final 28 CTest entries PASS normal and ASan+UBSan, including mapping differential and 1000-write repeated GC payload oracle/twin replay. Exact migrated/reclaimed counts and single erase verified. Core cpp coverage 95.54% lines / 92.62% branch outcomes. Clang analyzer and formatting pass. Python source/archive verification PASS at artifacts/runs/20261022T072230Z-verify-8373f7e5. New Linux GCC/Clang workflow prepared; execution not yet observed at this commit. Next: metrics/WAF and wear distribution tests, then wear-aware selection.

## Private save and observed Linux CI

Initial push was rejected by automatic approval review because explicit destination/branch/payload authorization was missing. Verified brightlcy ownership/private status/admin+push access, requested concrete approval, and user explicitly approved. Same private branch push then succeeded. Source 400ec1e passed Linux GCC and Clang ASan/UBSan CI, plus preserved Python core/figure CI. No main merge, public release, paper submission or paid resource was used. Final handoff update changes docs/evidence only; no extra test replay needed. Workspace-root checkpoint symlink points to the one canonical repository file.
