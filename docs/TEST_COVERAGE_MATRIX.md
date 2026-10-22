# Requirement → invariant → test → implementation → evidence

NAND implementation: src/nand/nand.cpp and include/atlas/nand/nand.hpp. Tests: tests/cpp/unit/nand_test.cpp and tests/cpp/property/nand_property_test.cpp.

| Requirement | Invariant | Test case | Result evidence |
|---|---|---|---|
| Positive/overflow-safe geometry | N6 | geometry rejects zero dimensions and overflow | nand-green.log |
| Address hierarchy/flat identity | N6 | addresses round trip; out of range coordinate | nand-green.log |
| Initialization | N1 | initial pages are free and counters zero | nand-green.log |
| Program/read data integrity | N2 | program preserves payload extremes | nand-green.log |
| Illegal program/invalidation | N2/N3 | double program; invalidate rejects unreadable | nand-green.log |
| Erase and neighbor integrity | N4/N5 | erase resets; live block erase nonmutation | nand-green.log |
| Bounds/nonmutation | N7 | bounds errors never change valid data | nand-green.log |
| Mixed repeated transitions | N8 | seeded independent page model, seed 0xA71A52026, 1500 operations | nand-green.log |

Paths in the last column resolve under results/validation/20261022-. Coverage and source hashes: 20261022-nand-quality.json. RED logs preserved separately. Uint64 erase overflow is defensively checked but cannot be reached through a feasible public-API run; disclose the uncovered edge. No fabricated counter injection is added solely for coverage.

Mapping implementation: src/ftl/page_mapping.cpp. Seven mapping unit cases plus one seeded integration/property case cover M1–M5; tests/cpp/unit/mapping_test.cpp, tests/cpp/integration/mapping_property_test.cpp. Rejected exhaustion and out-of-range writes preserve data. 19 C++ cases + 1 Python differential CTest pass normal and ASan/UBSan. Differential: 19 traces/662 committed events + altered-mapping negative control. Combined core coverage 136/144 lines (94.44%), 68/74 branch outcomes (91.89%). Consistency-audit internal-corruption throws and erase-overflow are uncovered; see quality JSON per-file summaries.

GC: 4 selector/trigger unit cases plus 4 integration/property cases map G1–G6 to tests/cpp/unit/gc_test.cpp and tests/cpp/integration/gc_integration_test.cpp. Source: src/gc/greedy.cpp and PageMapping::collect. GC RED had 8 failed new cases with 20 previous entries passing; final normal and sanitizer run 28/28. Seed 0x6CA71A52026, 1000 writes, twin replay, >100 GC cycles with payload/free-space/erase checks. Total core cpp coverage 214/224 lines (95.54%), 113/122 branch outcomes (92.62%). Defensive mapping corruption and counter-max throws remain unexecuted.
