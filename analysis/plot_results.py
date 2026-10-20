#!/usr/bin/env python3
"""Validate archived measurements and rebuild figures without rerunning a simulator.

Run from any directory: python analysis/plot_results.py [--verify-only].
Source archives are read in place. Nothing is extracted or changed.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import io
import json
import math
import platform
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORKLOADS = [
    ("MSR prxy_1", "prxy_1_hot_cold_threshold_8", "prxy_1_100k_workload_profile", None),
    ("Alibaba dev4", "alibaba_compare_device_4_t8", "alibaba_profile_device_4", "device_4.standard.csv"),
    ("Alibaba dev49", "alibaba_compare_device_49_t8", "alibaba_profile_device_49", "device_49.standard.csv"),
    ("Alibaba dev740", "alibaba_compare_device_740_t8", "alibaba_profile_device_740", "device_740.standard.csv"),
]
BLUE, ORANGE, INK = "#0072B2", "#D55E00", "#243347"


def require(condition, message):
    """Reject invalid research evidence even when Python optimization is enabled."""
    if not condition:
        raise ValueError(message)


def close(a, b):
    require(math.isclose(float(a), float(b), rel_tol=1e-12, abs_tol=1e-12), f"Numeric mismatch: {a!r} != {b!r}")


def csv_rows(data):
    return list(csv.DictReader(io.StringIO(data.decode("utf-8"))))


def validate():
    sources = {}
    warnings = []
    archive = ROOT / "results_archive.zip"
    sources["results_archive.zip"] = hashlib.sha256(archive.read_bytes()).hexdigest()

    def local(name):
        data = (ROOT / name).read_bytes()
        sources[name] = hashlib.sha256(data).hexdigest()
        return csv_rows(data)

    wide = {r["workload"]: r for r in local("data/policy_summary_wide.csv")}
    characteristics = {r["workload"]: r for r in local("data/workload_characteristics.csv")}
    verified = []
    with zipfile.ZipFile(archive) as z:
        def member(name):
            data = z.read(name)
            sources["results_archive.zip::" + name] = hashlib.sha256(data).hexdigest()
            return data

        for name, run, profile, trace_file in WORKLOADS:
            summaries, manifests = {}, {}
            for policy, filename in (("baseline", "baseline_summary.csv"), ("hot_cold", "hot_cold_summary.csv")):
                prefix = f"results/{run}/{policy}/"
                summary = csv_rows(member(prefix + filename))[0]
                manifest = json.loads(member(prefix + "experiment_manifest.json"))
                summaries[policy], manifests[policy] = summary, manifest
                host, nand, migrated = (int(summary[k]) for k in ("host_write_pages", "nand_write_pages", "valid_page_migrations"))
                require(host > 0 and nand == host + migrated, 'Validation failed: host > 0 and nand == host + migrated')
                close(nand / host, summary["write_amplification_factor"])
                for metric in ("write_amplification_factor", "valid_page_migrations", "gc_count"):
                    close(summary[metric], wide[name][metric + "_" + policy])
                    close(summary[metric], manifest["phase_results"]["measurement"][metric])
                require(manifest["phase_plan"]["warmup_rounds"] == 1, 'Validation failed: manifest["phase_plan"]["warmup_rounds"] == 1')
                require(manifest["phase_plan"]["measurement_rounds"] == 1, 'Validation failed: manifest["phase_plan"]["measurement_rounds"] == 1')
                require(manifest["phase_plan"]["state_reset_between_phases"] is False, 'Validation failed: manifest["phase_plan"]["state_reset_between_phases"] is False')
                require(manifest["sanity_checks"]["measurement"]["nand_write_identity"], 'Validation failed: manifest["sanity_checks"]["measurement"]["nand_write_identity"]')
                require(manifest["sanity_checks"]["measurement"]["gc_erase_identity"], 'Validation failed: manifest["sanity_checks"]["measurement"]["gc_erase_identity"]')
                if manifest["sanity_checks"]["latency"]["latency_nonnegative"] is False:
                    warnings.append(f"{name}/{policy}: historical latency_nonnegative sanity flag is false; latency is excluded from these figures.")
            require(manifests["baseline"]["ssd_config"] == manifests["hot_cold"]["ssd_config"], 'Validation failed: manifests["baseline"]["ssd_config"] == manifests["hot_cold"]["ssd_config"]')
            require(manifests["baseline"]["input_trace"]["sha256"] == manifests["hot_cold"]["input_trace"]["sha256"], 'Validation failed: manifests["baseline"]["input_trace"]["sha256"] == manifests["hot_cold"]["input_trace"]["sha256"]')
            require(summaries["baseline"]["host_write_pages"] == summaries["hot_cold"]["host_write_pages"], 'Validation failed: summaries["baseline"]["host_write_pages"] == summaries["hot_cold"]["host_write_pages"]')
            require(int(summaries["hot_cold"]["placement_fallback_pages"]) == 0, 'Validation failed: int(summaries["hot_cold"]["placement_fallback_pages"]) == 0')
            require(int(summaries["hot_cold"]["mixed_labeled_blocks"]) == 0, 'Validation failed: int(summaries["hot_cold"]["mixed_labeled_blocks"]) == 0')
            require(int(summaries["hot_cold"]["hot_threshold"]) == 8, 'Validation failed: int(summaries["hot_cold"]["hot_threshold"]) == 8')
            require(int(summaries["hot_cold"]["hot_window_writes"]) == 4096, 'Validation failed: int(summaries["hot_cold"]["hot_window_writes"]) == 4096')
            comparison = json.loads(member(f"results/{run}/comparison_manifest.json"))
            require(comparison["hot_cold_policy"]["future_information_used"] is False, 'Validation failed: comparison["hot_cold_policy"]["future_information_used"] is False')
            for metric in ("write_amplification_factor", "valid_page_migrations"):
                base, hot = (float(summaries[p][metric]) for p in ("baseline", "hot_cold"))
                delta = 100 * (hot - base) / base if base else 0.0
                close(delta, wide[name][metric + "_percent_change"])
            profile_data = json.loads(member(f"results/{profile}/workload_summary.json"))
            require(profile_data["total_requests"] == 100000, 'Validation failed: profile_data["total_requests"] == 100000')
            close(profile_data["write_pages"] / profile_data["total_pages"], profile_data["write_page_ratio"])
            close(profile_data["repeated_write_pages"] / profile_data["write_pages"], profile_data["overwrite_page_ratio"])
            for metric in ("write_page_ratio", "overwrite_page_ratio"):
                close(characteristics[name][metric], profile_data[metric])
            trace = manifests["baseline"]["input_trace"]
            if trace_file:
                raw = member("results/alibaba_candidate_100k/" + trace_file)
                require(hashlib.sha256(raw).hexdigest() == trace["sha256"], 'Validation failed: hashlib.sha256(raw).hexdigest() == trace["sha256"]')
                require(len(raw) == trace["size_bytes"], 'Validation failed: len(raw) == trace["size_bytes"]')
            else:
                warnings.append("MSR prxy_1: original 100k trace is absent from this results archive; its recorded checksum is preserved, but the trace was not reverified here.")
            verified.append({
                "workload": name,
                "measurement": {p: {k: float(v) for k, v in s.items()} for p, s in summaries.items()},
                "profile": {k: profile_data[k] for k in ("total_requests", "write_page_ratio", "overwrite_page_ratio")},
                "ssd_config": manifests["baseline"]["ssd_config"],
                "input_trace_sha256": trace["sha256"],
                "trace_bytes_verified": trace_file is not None,
                "historical_simulator_version": manifests["baseline"]["simulator"],
            })
    return verified, sources, warnings


def setup_plot():
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    plt.rcParams.update({
        "font.family": "DejaVu Sans", "font.size": 11,
        "axes.titlesize": 13, "axes.labelsize": 11,
        "axes.spines.top": False, "axes.spines.right": False,
        "axes.labelcolor": INK, "text.color": INK,
        "xtick.color": INK, "ytick.color": INK,
        "svg.fonttype": "none", "svg.hashsalt": "online-hotcold-20261020",
        "savefig.facecolor": "white",
    })
    return plt, matplotlib.__version__


def save(plt, fig, name):
    out = ROOT / "figures"
    out.mkdir(exist_ok=True)
    for suffix in ("png", "svg"):
        metadata = {"Software": "Matplotlib"} if suffix == "png" else {"Date": None, "Creator": "Matplotlib"}
        fig.savefig(out / f"{name}.{suffix}", dpi=180, bbox_inches="tight", metadata=metadata)
    plt.close(fig)


def plot_comparison(plt, rows):
    fig, axes = plt.subplots(1, 2, figsize=(12.6, 5.3))
    fig.suptitle("Online Hot/Cold placement helps three traces; dev740 stays unchanged", fontsize=16, fontweight="bold", y=1.00)
    labels = ["MSR\nprxy_1", "Alibaba\ndev4", "Alibaba\ndev49", "Alibaba\ndev740"]
    for ax, metric, scale, ylabel, title in [
        (axes[0], "write_amplification_factor", 1, "NAND page writes / host page writes", "(a) Write amplification (lower is better)"),
        (axes[1], "valid_page_migrations", 1000, "Valid pages migrated (thousands)", "(b) GC migration volume (lower is better)"),
    ]:
        ax.set_axisbelow(True)
        ax.grid(axis="y", color="#DDE3E8", linewidth=0.7)
        base = [r["measurement"]["baseline"][metric] / scale for r in rows]
        hot = [r["measurement"]["hot_cold"][metric] / scale for r in rows]
        ax.bar([i - .19 for i in range(4)], base, width=.36, color="#6E7F8D", edgecolor=INK, linewidth=.5, label="Baseline (single frontier)")
        ax.bar([i + .19 for i in range(4)], hot, width=.36, color=BLUE, hatch="///", edgecolor=INK, linewidth=.5, label="Online Hot/Cold")
        ymax = max(base) * 1.24
        ax.set_ylim(0, ymax)
        ax.set_xticks(range(4), labels)
        ax.set_ylabel(ylabel)
        ax.set_title(title, loc="left", pad=14)
        for i, (b, h) in enumerate(zip(base, hot)):
            change = (h / b - 1) * 100 if b else 0
            ax.text(i, max(b, h) + ymax * .035, f"{change:+.1f}%" if change else "unchanged", ha="center", fontsize=11, fontweight="bold")
            if metric == "write_amplification_factor":
                for x, value in ((i - .19, b), (i + .19, h)):
                    ax.text(x, value - .15, f"{value:.2f}", ha="center", va="top", color="white", fontsize=9)
        ax.margins(x=.06)
    axes[0].legend(frameon=False, loc="upper right", fontsize=9)
    fig.text(.5, -.02, "Historical measurements: one 100k-request slice per workload; same slice replayed once for warm-up, once for measurement.\nThreshold=8; window=4,096 host page writes; paired capacity/OP/GC settings. No repeated-seed uncertainty is available.", ha="center", fontsize=9.5, color="#4B5E70")
    fig.tight_layout(rect=(0, .04, 1, .93), w_pad=2)
    save(plt, fig, "rebuild_policy_comparison")


def plot_workloads(plt, rows):
    fig, ax = plt.subplots(figsize=(10.8, 4.6))
    fig.suptitle("Write intensity alone does not explain the placement result", fontsize=16, fontweight="bold", y=1.00)
    values = [[r["profile"][metric] * 100 for r in rows] for metric in ("write_page_ratio", "overwrite_page_ratio")]
    for j, (vals, color, hatch, label) in enumerate(zip(values, ("#6E7F8D", BLUE), ("", "///"), ("Write pages / all I/O pages", "Repeated write pages / write pages"))):
        offset = -.19 if j == 0 else .19
        ax.bar([i + offset for i in range(4)], vals, .36, color=color, hatch=hatch, edgecolor=INK, linewidth=.5, label=label)
        for i, v in enumerate(vals):
            ax.text(i + offset, v + 1.6, f"{v:.1f}", ha="center", fontsize=10)
    ax.set_xticks(range(4), ["MSR prxy_1", "Alibaba dev4", "Alibaba dev49", "Alibaba dev740"])
    ax.set_ylim(0, 118)
    ax.set_ylabel("Page ratio (%) — denominators differ")
    ax.set_axisbelow(True)
    ax.grid(axis="y", color="#DDE3E8", linewidth=.7)
    ax.legend(frameon=False, loc="upper center", bbox_to_anchor=(.5, 1.01), ncol=2, fontsize=10)
    fig.text(.5, -.01, "Source: one-pass profiles of four 100k-request trace slices. dev740 is 100% writes, but only 14.4% are repeat writes.\nThese descriptive observations do not isolate overwrite ratio or hot-set persistence as a causal variable.", ha="center", fontsize=9.5, color="#4B5E70")
    fig.tight_layout(rect=(0, .07, 1, .94))
    save(plt, fig, "rebuild_workload_characteristics")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--verify-only", action="store_true", help="Validate source data with the Python standard library; do not plot or write files.")
    args = parser.parse_args()
    rows, sources, warnings = validate()
    print("PASS: 8 archived policy runs, WAF denominators, migration identities, paired configs, 4 profiles, and 3 archived trace checksums.")
    for warning in warnings:
        print("NOTE:", warning)
    if args.verify_only:
        return
    plt, version = setup_plot()
    plot_comparison(plt, rows)
    plot_workloads(plt, rows)
    manifest = {
        "status": "historical-results-validated-and-replotted; simulator-not-rerun",
        "source_sha256": sources,
        "script_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "generator": {"python": platform.python_version(), "matplotlib": version},
        "metric_definition": "WAF = measurement NAND page writes / measurement host page writes = 1 + valid page migrations / host page writes in this model",
        "uncertainty": "one deterministic trace slice per workload; no error bars or population inference",
        "verified_workloads": rows,
        "warnings": warnings,
    }
    (ROOT / "analysis" / "figure_manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    print("Wrote 2 PNG/SVG figures and analysis/figure_manifest.json.")


if __name__ == "__main__":
    main()
