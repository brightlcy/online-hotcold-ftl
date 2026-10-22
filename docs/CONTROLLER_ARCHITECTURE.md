# ATLAS-SSD controller architecture — Gate 1 contract

This extends the existing research on a separate branch. Python simulator/archive bytes remain reference evidence. C++ produces future primary outputs only after correctness and scientific gates pass. This is a single-threaded algorithmic model, not firmware or timing-accurate hardware.

## Structure and responsibilities

Trace → future queue/scheduler → FTL mapping → future placement/GC/wear → NAND → future metrics recorder. Policy and observation modules will depend on interfaces rather than own the mapping or NAND.

- `Geometry`: owns checked positive channel/chip/die/block/page dimensions; maps structured addresses to flat PPNs in channel/chip/die/block/page order, page fastest. Products are checked before allocation. No planes in this version.
- `Nand`: owns all pages and erase counters by value; no mutable state escapes. Payload is an opaque uint64 token for integrity tests, not a byte-accurate NAND sector. Program only FREE→VALID; read only VALID; invalidate only VALID→INVALID. Erase requires no VALID pages in the whole block, then resets FREE and increments once. Rejected operations leave state unchanged.
- Next `PageMapping`: will own NAND and optional L2P/P2L arrays. Validate inputs and available space before programming. Program destination, update inverse mappings, invalidate previous data in a no-throw commit sequence. Reserve-aware GC is a later explicit contract.
- Future `GarbageCollector`: deterministic greedy victim, preflight space, migrate valid payload and mappings, erase only after no valid data remains. Never erase live data to make an allocation succeed.

## Memory, flow and ordering

One controller owns all mutable state; no shared ownership, concurrency or background events. API calls finish atomically at the model level. Allocation failure is reported before state mutation. A caller cannot directly change page state/counters. Object allocation failures use standard C++ exceptions. Bounds failures use out_of_range; invalid geometry uses invalid_argument/overflow_error; invalid state transitions use logic_error.

## Failure conditions and invariants

See INVARIANTS.md. Whole-block erase preflight precedes all state changes. Address validation precedes indexing. Flat capacity multiplication is checked; no wrapped address can alias another page. Allocation can still fail with bad_alloc/length_error for unreasonably large but representable geometry.

## Complexity

Geometry encode/decode and page operations O(1); initial allocation O(P+B); erase O(pages/block); storage O(P+B). No timing, ECC, NAND program order, bad blocks, power-loss recovery, DMA, NVMe, endurance limit or real SSD lifespan is modeled. The uint64 erase counter cannot be increased beyond its maximum; overflow must fail before reset. The overflow branch is not practically reachable through public operations and needs separate coverage accounting.

## Test strategy and expansion gate

Catch2/CTest: normal, boundary, illegal transitions, nonmutation on failure, payload extremes, multi-dimensional addresses and deterministic randomized reference-state comparisons. Run ASan+UBSan and compile-time warnings/static analysis. Complete NAND before mapping, mapping before GC. Preserve red/green logs and requirement→invariant→test links. Linux CI is only verified after observing a remote pass.
