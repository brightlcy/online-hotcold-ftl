# Existing research audit — resumed 2026-10-22

## Scope and source authority

Actual local files/retained outputs take precedence over summaries. Read all three pasted operating prompts. Searched accessible home directories excluding caches, package stores and Library for the dedicated checkpoint/atlas directory. No ATLAS checkpoint or prior C++ core was found. `/Users/changyeop/atlas` is not a Git repository and has no recovered controller. Authenticated GitHub CLI returned five account repositories: the three existing research repos and two receipt projects; research branches contain main and historical portfolio rebuild branches. Connector search returned empty and direct private fetch returned 404, so these were not treated as proof of absence. No inaccessible prior Work environment is claimed read.

Verified remote refs: Online main 2bc94ffb070892d2265ec9bd013a2d16e8fecb6f; Host main ebec951dd86d4bd4a80cae0b2c92d713afaae43c; portfolio main a4d67e1cd18bf1dc3456b7cfdb50bee062ea5e19. Local Online/Host were clean on those main refs before recovery. New work continues on Online branch atlas-ssd-resume-20261022. No repository was replaced.

Notion fetched (historical workspace evidence, not current code execution):
- [Evidence bank](https://app.notion.com/p/3cd132c5e443819a89b7db9b60f83496)
- [Experiment specification](https://app.notion.com/p/3c2132c5e443810a9f12fe61fc10e905)
- [Related work matrix](https://app.notion.com/p/3c2132c5e4438172a374c0eb14d34df4)
- [Final results dashboard](https://app.notion.com/p/3c2132c5e44381eca8edf8e1e5c8cb65)
- [Portfolio resume](https://app.notion.com/p/3ff132c5e443819a9d8cfe029cffe782)

ATLAS-specific keyword search returned related historical pages, not a dedicated resume page. This bounded retrieval cannot exclude inaccessible material.

## Verified Research Assets

- Existing Online Python simulator, tests, configs, results_archive.zip and manifests; sibling Host simulator, detector code and final results archive. Original simulator/archive hashes remain the reference.
- Existing source-history bundle and preservation ledger are documented in ../preservation and prior DELIVERY_REPORT.md; bundle was not rebuilt or uploaded in this session.
- Actual retained Alibaba run 20261020T053138Z-replay-12d7c9db has PASS, three exit-zero commands and finished_at_utc=2026-10-20T06:21:40.036915Z.
- The summaries that still said RUNNING were stale. Existing-output finalization now verifies six policies: 24 metrics per device for dev4/dev49/dev740 plus trace/geometry/parameters/phase counters. Report: results/manifests/rebuild_completed_hotcold_replay.json. No simulation was launched by finalization. Legacy expected-source plus preservation evidence is weaker than a separately recorded executed digest; the missing field is explicitly retained.

## Reproducible Results

Existing Alibaba three 100k-request extracts + frozen implementation + completed replay outputs verified by finalizer. WAF, migrations and phase controls match archived outputs. Nine reproduction-safety tests passed this session. Existing Host 220-run replay compact report records byte equality of five scientific CSV/JSON files; no second replay performed here. Original Python 139/215-test and CI records are historical and not new session test results.

## Historical Results Only

The following are old research observations, not new C++ results: B=512/K=2 selected on dev seeds 201–205; final seeds 301–305, 220 cached policy runs, 70 paired conditions clustered in five seeds; harmful 21/21 timely detection and lower WAF; seed-equal false fallback 34.6667%→8% (pooled 17/49→4/49 differs); harmful wear-CV delta median +0.02871. v1 early transient switching and boundary v2 false fallbacks remain negative evidence. No new untouched holdout exists. Full 525,559,379-request profile was not repeated. MQSim chip-isolation findings remain historical.

## Missing Raw Data

MSR prxy_1 original 100k extract absent from archive; replay BLOCKED. No new ATLAS workload or raw C++ experiment exists yet. Completed Alibaba large raw request logs stay in the original staging path, not copied into Git.

## Missing Source Code

Previous dedicated ATLAS C++ artifact not located. Modified MQSim engine source was not recovered from available exports; dynamic lending config→execution path is unverified. The ordinary upstream engine would not substitute for the missing modified source.

## Known Scientific Limitations

Python analytical timing is not measured SSD latency. Dev4 has strict floating point latency-versus-service differences ≤4.66e-10 us, with no negative latency/queue/service or completion-before-arrival values in finalized logs; keep original flags. Binary full-inversion symmetry is restricted to symmetric placement resources. WAF/wear trade-off is not device lifespan. Limited seeds/workloads and historical holdout reuse limit generalization. Related-work notes are not a completed 12-paper primary-source deep audit; novelty gate remains OPEN. C++ correctness does not settle novelty.

## Reusable Algorithms

Python mapping/invalidation, deterministic free-page reservation queue, greedy GC, online classifier, completed-lifetime observations and persistence detector. Preserve source/selection/hashes. New C++ interfaces can differ only with documented compatibility rules.

## Reusable Test Fixtures

Tiny config/trace, mapping consistency tests, resource pressure scenarios, original archived manifests and finalizer negative fixtures. Test-only historical provenance fixtures are not production provenance. New NAND uses payload tokens and no RESERVED state; do not silently claim whole-model equivalence.

## What Must Be Reimplemented

Test-first C++ NAND, mapping, GC, wear-aware decisions, host-hint observer, placement interface, adaptive controller and queue/timing. Analysis functions require their own tests before new primary figures. MQSim baseline build and instrumented policy-path validation remain separate gates.

## What Must Not Be Repeated

Do not repeat source archaeology, archive extraction, Alibaba completed replay, Host final 220-run replay, historical selection or plots without a changed-source/reproducibility reason. Never tune on old final seeds or report a repeated deterministic run as another independent sample. Preserve original Python and datasets while extending the isolated C++ branch.

## Red Team disposition

Prior P1 optimized-validation bypass, P2 declared-vs-applied config and P3 seed overlap are resolved in existing tooling per retained independent review. Newly executed finalizer safety tests pass. New C++ review must explicitly cover bounds/overflow, invalid transitions, data preservation and failure atomicity. No independent reviewer was run in this session; role-based self-review must be labeled as such.
