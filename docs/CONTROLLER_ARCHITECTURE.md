# ATLAS-SSD controller architecture — Gate 1 contract

This extends the existing research on a separate branch. Python simulator/archive bytes remain reference evidence. C++ produces future primary outputs only after correctness and scientific gates pass. This is a single-threaded algorithmic model, not firmware or timing-accurate hardware.

## Structure and responsibilities

Trace → future queue/scheduler → FTL mapping → future placement/GC/wear → NAND → future metrics recorder. Policy and observation modules will depend on interfaces rather than own the mapping or NAND.

- `Geometry`: owns checked positive channel/chip/die/block/page dimensions; maps structured addresses to flat PPNs in channel/chip/die/block/page order, page fastest. Products are checked before allocation. No planes in this version.
- `Nand`: owns all pages and erase counters by value; no mutable state escapes. Payload is an opaque uint64 token for integrity tests, not a byte-accurate NAND sector. Program only FREE→VALID; read only VALID; invalidate only VALID→INVALID. Erase requires no VALID pages in the whole block, then resets FREE and increments once. Rejected operations leave state unchanged.
- `PageMapping`: owns NAND and optional L2P/P2L arrays. Validate inputs and available space before programming. Program destination, update inverse mappings, invalidate previous data in a no-throw commit sequence. Reserve-aware GC is a later explicit contract.
- GC transaction (`PageMapping::collect`) and separate pure greedy selector: deterministic victim, preflight space, migrate valid payload and mappings, erase only after no valid data remains. Never erase live data to make an allocation succeed.

## Memory, flow and ordering

One controller owns all mutable state; no shared ownership, concurrency or background events. API calls finish atomically at the model level. Allocation failure is reported before state mutation. A caller cannot directly change page state/counters. Object allocation failures use standard C++ exceptions. Bounds failures use out_of_range; invalid geometry uses invalid_argument/overflow_error; invalid state transitions use logic_error.

## Failure conditions and invariants

See INVARIANTS.md. Whole-block erase preflight precedes all state changes. Address validation precedes indexing. Flat capacity multiplication is checked; no wrapped address can alias another page. Allocation can still fail with bad_alloc/length_error for unreasonably large but representable geometry.

## Complexity

Geometry encode/decode and page operations O(1); initial allocation O(P+B); erase O(pages/block); storage O(P+B). No timing, ECC, NAND program order, bad blocks, power-loss recovery, DMA, NVMe, endurance limit or real SSD lifespan is modeled. The uint64 erase counter cannot be increased beyond its maximum; overflow must fail before reset. The overflow branch is not practically reachable through public operations and needs separate coverage accounting.

## Test strategy and expansion gate

Catch2/CTest: normal, boundary, illegal transitions, nonmutation on failure, payload extremes, multi-dimensional addresses and deterministic randomized reference-state comparisons. Run ASan+UBSan and compile-time warnings/static analysis. Complete NAND before mapping, mapping before GC. Preserve red/green logs and requirement→invariant→test links. Linux CI is only verified after observing a remote pass.

## Mapping atomic contract (before implementation)

PageMapping owns NAND and fixed-size optional L2P/P2L vectors. Logical capacity must be 1..physical pages. Empty read throws out_of_range, bad LPN/PPN throws out_of_range, physical exhaustion throws NoFreePage (runtime_error). A failed overwrite must leave the old payload and both directions intact. Only successful writes consume one FREE page. Deterministic circular next-free scan; before GC the order equals the Python FIFO. No automatic GC in this atomic unit. Constructor validates logical capacity before NAND allocation. Per-write operations after NAND program do not allocate: optional<size_t> assignment and validated invalidate cannot throw. Allocation scans O(P), mapping/read O(1), invariant audit O(P+L).

## GC atomic contract (before implementation)

Pure `select_greedy_victim(span<BlockSummary>)` chooses reclaimable blocks by most INVALID pages, then fewest VALID pages, then lowest ID. Reject candidates whose live pages exceed FREE pages outside the victim. Partial blocks are eligible only with INVALID pages and positive net reclamation. Free-sum overflow is rejected before subtraction. No wear-aware selection yet.

PageMapping owns the mutation transaction (`collect`), while the pure selector is separate and replaceable. Gather summaries and allocate the complete migration plan before mutation; copy each valid payload to distinct FREE pages outside victim, update inverse maps, invalidate old sources, erase only after all live pages move. Preflight erase-count overflow too. No new allocation or external callback after migration starts. Return victim, migration count and net reclaimed INVALID pages. No eligible/feasible victim throws GcBlocked with no mutation. `needs_gc(reserve_pages)` is free<=reserve, validates reserve<=capacity; caller explicitly invokes collect. Automatic write-triggered GC and background scheduling are not claimed.

GC costs O(P+B) scan plus O(valid victim pages), temporary plan O(valid victim pages), no infinite retry loop. The snapshot/plan allocation can fail before mutation; external fault injection is not yet performed. Logical data and mappings are preserved even when allocation or GC preflight fails.
