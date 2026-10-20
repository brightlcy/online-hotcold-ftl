# Reproducibility

The simulator and reproduction runner use the Python standard library. The rebuild was checked with **Python 3.14.6** on macOS arm64. Original archived manifests also record the original runtime; matching a result does not mean the runtimes were identical.

Run these commands from this project directory. The directory can be copied out of the portfolio monorepo without installing its sibling projects:

```sh
python3 reproduce.py tests
python3 reproduce.py verify
python3 reproduce.py smoke
python3 reproduce.py replay
```

`tests` runs the original test suite plus archive-safety tests, then checks the original Baseline and policy-comparison CLI help. `verify` checks source hashes, ZIP CRC, every archived member hash, and each preserved Alibaba input hash against its historical experiment manifest. It performs no experiment execution. `smoke` executes both policies on the small included sample; this is **NEW_SMOKE** evidence and cannot support the historical performance claims.

`replay` runs the three preserved Alibaba 100,000-request extracts with the original comparison CLI and explicit values from [configs/alibaba_t8.json](../configs/alibaba_t8.json): one warm-up pass and one measurement pass over the same trace, 4 KiB pages, 4 chips, 64 pages/block, requested OP 20%, GC threshold 5%, reserve 2 blocks, threshold 8, and window 4,096 writes. Capacity is resolved from each write working set; geometry rounding means actual OP may differ from requested OP. This is deterministic replay; no random seed is used. A single device can be selected:

```sh
python3 reproduce.py replay --device 4
```

The runner selectively extracts only the requested input into a temporary directory. It never expands the archive into source/results. Output goes to a unique `artifacts/runs/<UTC>-<action>-<id>/` directory; existing files are never replaced. Each run records UTC timestamps, Python/platform, current Git commit when available, expected and actual source SHA-256, config/runner hashes, commands, exit status, and logs. Trace hashes, original and replay manifest hashes, metric differences, and exact settings are recorded for each replay. Git provenance can be unavailable in a standalone directory; source hashes remain available.

## Evidence and validation levels

The compact checked-in report is [rebuild_validation.json](../results/manifests/rebuild_validation.json). [rebuild_standalone_validation.json](../results/manifests/rebuild_standalone_validation.json) records an additional test run in an independent temporary copy without the parent repository. Large regenerated request-level logs stay under ignored `artifacts/`; the original `results_archive.zip` remains byte-preserved.

| Evidence | Verification level |
|---|---|
| Original tests | 130 passed before the runner was added |
| Tests with safety checks | 137 passed; original simulator files unchanged |
| Alibaba dev4/dev49/dev740 | See checked-in report for completed historical replay and per-metric comparison |
| MSR `prxy_1` 100k | **BLOCKED** for replay: the input extract is absent from the preserved archive; summaries and manifests are preserved |
| Included sample | Fresh smoke run, separate from research results |

The reference hashes in [configs/evidence_snapshot.json](../configs/evidence_snapshot.json) were checked against the preservation copy records in the source portfolio. All 103 archived member hashes also match the original upload copy records; [rebuild_source_preservation.json](../results/manifests/rebuild_source_preservation.json) records that audit. These checks identify copied bytes; they are not a cryptographic signature or independent proof of trace origin.

## What is compared

Each replay compares all 12 metrics for each policy and the classifier settings against the archived `comparison_manifest.json`. It also checks SSD geometry, actual parameters, phase plan, all phase counters, trace statistics, and latency-model settings against each archived experiment manifest; [rebuild_phase_configuration_validation.json](../results/manifests/rebuild_phase_configuration_validation.json) records the rebuild checks. Missing/extra fields fail validation; numeric tolerance is `rel_tol=abs_tol=1e-12`. The archived result is never rewritten. A match establishes deterministic agreement with the exported implementation and preserved inputs, not independent validation of the model.

WAF is **measurement-phase NAND page programs / measurement-phase host page writes**. Valid-page migrations count GC copies; average migrations per GC is a separate proxy. Warm-up and measurement use repeated passes through the same extract, not disjoint chronological samples. The latency model is a serialized analytical cost model, not measured SSD latency. The original dev4 Baseline and Hot/Cold manifests contain `latency_nonnegative=false`. Fresh replay logs have zero negative queue/service/latency values or completion-before-arrival cases across 100,000 requests per policy. Each has two entries where latency is below service by at most `4.656612873077393e-10` microseconds, consistent with floating-point subtraction and a strict comparison without tolerance. See [rebuild_latency_diagnostics.json](../results/manifests/rebuild_latency_diagnostics.json); subsequent replays also compute these diagnostics automatically. The original flag and algorithm remain unchanged.

## Baseline and direct execution

The portable runner calls the original entry point without algorithm changes. To inspect or customize a **new**, separately labeled run:

```sh
python3 -m simulator.run_compare_hot_cold --help
python3 -m simulator.run_baseline --help
```

Do not silently edit archived result CSVs or the reference snapshot to make a failing replay pass. Record a new configuration, evidence class, source revision, and output directory instead. Figure regeneration is documented in the project README and figure provenance files; it uses preserved result data rather than smoke outputs.
