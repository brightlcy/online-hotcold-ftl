#include "atlas/nand/nand.hpp"
#include <array>
#include <catch.hpp>
#include <random>

TEST_CASE("NAND mixed operations match seeded independent page model", "[nand][property]") {
  constexpr std::uint64_t seed = 0xA71A52026;
  std::mt19937_64 rng(seed);
  atlas::Nand n(atlas::Geometry(1, 1, 1, 3, 4));
  // 0=free, 1=valid, 2=invalid; deliberately separate from production enum.
  std::array<int, 12> states{};
  std::array<std::uint64_t, 12> values{};
  std::array<std::uint64_t, 3> erases{};
  for (int step = 0; step < 1500; ++step) {
    const auto ppn = static_cast<std::size_t>(rng() % 12);
    const auto op = rng() % 3;
    INFO("seed=" << seed << " step=" << step << " ppn=" << ppn << " op=" << op);
    if (op == 0 && states[ppn] == 0) {
      values[ppn] = rng();
      n.program(ppn, values[ppn]);
      states[ppn] = 1;
    } else if (op == 1 && states[ppn] == 1) {
      n.invalidate(ppn);
      states[ppn] = 2;
    } else if (op == 2) {
      const auto block = ppn / 4;
      bool live = false;
      for (std::size_t p = block * 4; p < block * 4 + 4; ++p) {
        live = live || states[p] == 1;
      }
      if (live) {
        REQUIRE_THROWS_AS(n.erase(block), std::logic_error);
      } else {
        n.erase(block);
        ++erases[block];
        for (std::size_t p = block * 4; p < block * 4 + 4; ++p) {
          states[p] = 0;
        }
      }
    }
    for (std::size_t p = 0; p < 12; ++p) {
      const auto expected = states[p] == 0   ? atlas::PageState::Free
                            : states[p] == 1 ? atlas::PageState::Valid
                                             : atlas::PageState::Invalid;
      REQUIRE(n.state(p) == expected);
      if (states[p] == 1) {
        REQUIRE(n.read(p) == values[p]);
      }
    }
    for (std::size_t b = 0; b < 3; ++b) {
      REQUIRE(n.erase_count(b) == erases[b]);
    }
  }
}
