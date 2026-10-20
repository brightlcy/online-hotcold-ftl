# Architecture and data flow

The simulator remains in its original package to preserve algorithm bytes. No unnecessary source split or new shared dependency was introduced. `reproduce.py` orchestrates a project-local interpreter, immutable archive reads and output isolation. Analysis scripts consume frozen evidence, not simulator outputs silently substituted into old results.

`raw trace → schema adapter → timestamp,flow_id,op,lba,size → phased experiment runner → BaselineFTL / HotColdFTL → mapping invalidation → greedy GC → page counters → comparison CSV → figures`.

`metadata.py` owns page/block state, `address.py` page ranges, `config.py` geometry, `baseline_ftl.py` page-mapped writes/reads and GC, `gc_policy.py` victim selection, `hot_cold_ftl.py` classifier/frontiers. `experiment_runner.py` and `hot_cold_experiment.py` handle equal phase boundaries. `run_compare_hot_cold.py` is the primary policy entrypoint. Alibaba parser/profiler/extractor entrypoints prepare independent per-device traces; streaming is optional and needs external source/tools. Analytical latency and attribution wrap policy execution and have a known dev4 numerical diagnostic issue; no timing correction was retroactively made.

[File-by-file symbols/imports](FILE_MAP.md) · [Machine-readable graph](DEPENDENCIES.json)
