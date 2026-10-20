# Results and verification status

The source tables, complete captions, sample sizes, frozen conditions and regeneration commands are in [FIGURES.md](FIGURES.md). Figures are regenerated from preserved research evidence, not the new smoke runs.

## Online Hot/Cold

![Policy results](../figures/rebuild_policy_comparison.png)

WAF changes: MSR −15.32%, dev4 −34.03%, dev49 −8.99%, dev740 0.00%. Corresponding migration changes: −26.31%, −47.51%, −17.17%, 0 pages to 0 pages. These are deterministic slice comparisons, not confidence estimates over independent samples.

![Workload context](../figures/rebuild_workload_characteristics.png)

Observed repeat-write structure provides context for the workload-dependent result; four selected cases do not isolate causality or prove a universal relationship. The dev740 neutral result is retained. Historical threshold-2 sensitivity worsens modeled max latency despite lower WAF.

Data: [policy comparison](../data/policy_comparison_all.csv), [workload characteristics](../data/workload_characteristics.csv), [threshold sensitivity](../data/threshold_sensitivity.csv), [archived per-policy manifests and input extracts](../results_archive.zip). Script: [plot_results.py](../analysis/plot_results.py).

## Latency diagnostic

Historical dev4 manifests record `latency_nonnegative=false`. Replayed 100k-row request logs contain zero negative queue, service, total-latency or completion values; each policy has two cases where latency is less than service by at most 4.66e−10 microseconds. This is consistent with strict floating-point comparison. It does not indicate observed negative total latency. Source code and original diagnostics are preserved. Timing claims stay out of the headline pending a separately versioned tolerance/model review; see [diagnostics](../results/manifests/rebuild_latency_diagnostics.json).

## Reproduction status

Three archived Alibaba extracts are available for same-condition historical replay. MSR raw input is absent and full streaming profile was not repeated. The checked-in [validation report](../results/manifests/rebuild_validation.json) is authoritative for execution status and comparison tolerances. Historical original results are not replaced.
