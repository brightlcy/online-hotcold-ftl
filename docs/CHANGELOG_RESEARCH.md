# Research changelog

## 2026-10-22 — recovered checkpoint

Read all three user operating prompts. Reconciled local checkpoint with retained worker and Notion. Worker had completed at 2026-10-20T06:21:40Z although summaries still said RUNNING. Finalized existing output on 2026-10-22 without simulation; all three devices pass 24 metrics and both policies’ phase/config checks. Kept legacy missing executed-digest limitation, MSR blocker and original raw output. Nine finalizer safety tests passed. Continuing existing repo on an isolated development branch.

AI assistance: Codex performed state recovery, report reconciliation and will generate C++ interfaces/tests/implementation. No independent human design/review or hardware work is claimed.

## NAND-01 verified

Added checked C++ geometry and NAND state/token model with Catch2/CTest. Recorded expected runtime RED then GREEN 11 cases, ASan/UBSan, deterministic 1500-operation reference-model property case, formatting and Clang static analysis. cpp coverage 97.59% lines / 97.37% branch outcomes; counter-overflow edge disclosed. AI generated design/tests/implementation; self-review is not independent validation. Historical source/archive unchanged. Next: mapping TDD.
