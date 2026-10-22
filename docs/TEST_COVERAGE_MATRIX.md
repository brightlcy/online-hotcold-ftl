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
