# Executed validation snapshot

Verified on 2026-10-20. [GitHub CI run](https://github.com/brightlcy/online-hotcold-ftl/actions/runs/39378621293) **PASS**: core and figure jobs. Tested source commit: `2d5d86236bba69f5ecd20ea4e89b9041816574f3`. Subsequent documentation-only changes do not replace that recorded run identity.

## Execution evidence

- 139 local core/reproduction tests PASS, and the same source passes the Ubuntu CI test command.
- Five optimized-mode analysis regression tests PASS; damaged numeric/config/checksum fixtures are rejected.
- Source/archive preservation and plot-source validation PASS.
- The Quick Start verify/tests/smoke commands passed in an independent project copy and in GitHub CI.
- Pinned Python 3.12.14 figure generation passed in CI and uploaded regenerated figures. Rendering bytes can differ across platforms; numeric validation is independent of renderer packages.
- At the initial publication, all 117 tracked files downloaded from GitHub matched local bytes. The final delivery audit records any subsequent documentation-file count.
- Both research repositories remain private. The original project-wide code license was not changed to a permissive license; Alibaba data attribution/CC BY 4.0 are recorded separately.

## Historical reproduction status

All six Alibaba policy replays completed on 2026-10-20 at 06:21:40 UTC. Existing output was finalized on 2026-10-22: dev4, dev49 and dev740 each match all 24 archived comparison metrics, plus both policies’ configuration and phase counters. [Completion report](../results/manifests/rebuild_completed_hotcold_replay.json). No simulation was rerun during finalization. MSR input remains absent. The legacy worker records expected-source hashes and a passing preservation check, without a separate executed-source digest. Historical dev4 strict floating-point latency flags are retained; timing is outside the headline.

See [reproducibility](REPRODUCIBILITY.md), [current compact reports](../results/manifests/), [figure data and captions](FIGURES.md), and [Red Team review](RED_TEAM.md). No algorithm or original research result was altered during the rebuild.

The finalizer explicitly validates recorded executed-source hashes when available and labels legacy source metadata. The underlying research simulator and results are unchanged.

Independent [focused finalizer review](FINALIZER_REVIEW.md) found no unresolved issue after executed-source and legacy-metadata checks were added.
