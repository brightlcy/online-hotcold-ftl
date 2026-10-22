"""Compare committed C++ mapping transitions against existing Python metadata.

No GC/latency equivalence is asserted. The adapter allocates before invalidating
to express the shared committed-operation contract and preserve failed writes.
"""
import json
from pathlib import Path
import random
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from simulator.config import SSDConfig
from simulator.metadata import FTLMetadata, NoFreePageError, PageState


def reference(trace):
    metadata = FTLMetadata(SSDConfig(page_size=4096, chips=1, blocks_per_chip=2,
                                    pages_per_block=4, reserved_gc_blocks=1))
    rows = []
    for lpn, _ in trace:
        try:
            dest = metadata.allocate_free_page()
            if lpn in metadata.mapping_table:
                metadata.invalidate_lpn(lpn)
            metadata.register_valid_page(lpn, dest)
            status = "OK"
        except NoFreePageError:
            status = "NO_SPACE"
        metadata.assert_consistent()
        codes = {PageState.FREE: 0, PageState.VALID: 1, PageState.INVALID: 2}
        rows.append({"status": status,
                     "l2p": [metadata.mapping_table.get(l) for l in range(3)],
                     "p2l": [metadata.page_owner.get(p) for p in range(8)],
                     "states": [codes[metadata.get_page_state(p)] for p in range(8)],
                     "free": metadata.free_page_count,
                     "erases": [b.erase_count for b in metadata.block_info]})
    return rows


def compare(expected, observed):
    if len(expected) != len(observed):
        raise ValueError(f"event count mismatch {len(expected)} != {len(observed)}")
    for event, (left, right) in enumerate(zip(expected, observed)):
        if left != right:
            raise ValueError(f"first mismatch event {event}: {left} != {right}")


def main(binary):
    # Validator negative control: a deliberately altered transition must fail.
    try:
        compare([{"l2p": [0]}], [{"l2p": [1]}])
    except ValueError:
        pass
    else:
        raise RuntimeError("negative control accepted altered mapping")
    cases = [("single", [(0, 0)]), ("overwrite-exhaustion", [(0, i) for i in range(12)]),
             ("mixed", [(l, i) for i, l in enumerate([0, 1, 2, 1, 0, 2, 2, 1, 0])])]
    for seed in range(1701, 1717):
        rng = random.Random(seed)
        cases.append((f"seed-{seed}", [(rng.randrange(3), rng.getrandbits(64)) for _ in range(40)]))
    events = 0
    for name, trace in cases:
        try:
            output = subprocess.run([binary], input="".join(f"{l} {v}\n" for l, v in trace),
                                    text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                    check=True, timeout=30)
            compare(reference(trace), [json.loads(line) for line in output.stdout.splitlines()])
        except Exception as error:
            folder = ROOT / "artifacts" / ("mapping-differential-failure-" + str(time.time_ns()))
            folder.mkdir(parents=True, exist_ok=False)
            (folder / "counterexample.json").write_text(json.dumps(
                {"case": name, "trace": trace, "error": str(error)}, indent=2))
            raise
        events += len(trace)
    print(f"PASS: {len(cases)} traces, {events} committed events; negative control rejected")


if __name__ == "__main__":
    main(sys.argv[1])
