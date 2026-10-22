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
