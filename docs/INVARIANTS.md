# Controller invariants

| ID | Requirement / invariant | Planned test |
|---|---|---|
| N1 | Fresh/erased pages FREE; free pages cannot be read | Initial pages and unreadable states |
| N2 | Program only FREE→VALID; rejected calls preserve state and payload | Program transitions; double program |
| N3 | Invalidate only VALID→INVALID; invalid data never returned | Invalidation/read errors |
| N4 | Erase refuses every block with live data before mutation | Live-block erase rejection |
| N5 | Successful erase resets entire block; count increases exactly once; neighbors unchanged | Erase/reset/reprogram |
| N6 | Geometry positive; products do not overflow; every coordinate validated | Geometry bounds and address roundtrip |
| N7 | Page/block out-of-range calls never mutate state | Address boundaries |
| N8 | Reproducible mixed operations equal independent state/payload model | Seeded NAND property test |

Mapping and GC invariants will be added with their interfaces before implementation. No unimplemented invariant is marked verified.

| M1 | Each mapped LPN has exactly one VALID PPN and inverse owner | Mapping inverse and repeated overwrites |
| M2 | Every VALID page has one owner; FREE/INVALID pages have no current owner | Consistency scan after each generated operation |
| M3 | Read returns last successful write token | Overwrite/integration/property cases |
| M4 | Exhausted/invalid writes leave all mapping, payload and state unchanged | Failed overwrite and bad LPN tests |
| M5 | No GC or hidden erase during basic mapping writes | Exhaustion counter checks |

| G1 | Victim selection deterministic; live copies fit outside victim | Greedy tie/space tests |
| G2 | Only VALID pages migrate; INVALID pages reclaimed and not copied | Mixed and fully-invalid GC cases |
| G3 | Relocation preserves last logical payload and inverse ownership | GC integration and seeded pressure oracle |
| G4 | Failed preflight leaves mappings/data/states/erase counts intact | Blocked GC integration |
| G5 | Successful GC increments one erase, reclaims exactly invalid count | Per-call free/erase accounting |
| G6 | Repeated GC terminates and is deterministic | Two-controller pressure replay |
