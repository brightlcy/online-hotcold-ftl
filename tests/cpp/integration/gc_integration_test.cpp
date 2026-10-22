#include "atlas/ftl/page_mapping.hpp"
#include <array>
#include <catch.hpp>
#include <random>

TEST_CASE("GC migrates only live pages preserves payload and reclaims exact garbage",
          "[gc][integration]") {
  atlas::PageMapping f(atlas::Geometry(1, 1, 1, 4, 3), 3);
  f.write(0, 10);
  f.write(1, 11);
  const auto old = f.write(2, 12);
  f.write(0, 20);
  f.write(1, 21);
  const auto before = f.free_pages();
  const auto gc = f.collect();
  REQUIRE(gc.victim == 0);
  REQUIRE(gc.migrated == 1);
  REQUIRE(gc.reclaimed == 2);
  REQUIRE(f.free_pages() == before + 2);
  REQUIRE(f.nand().erase_count(0) == 1);
  REQUIRE(f.nand().erase_count(1) == 0);
  REQUIRE(f.physical(2) != old);
  REQUIRE_FALSE(f.owner(old));
  REQUIRE(f.read(0) == 20);
  REQUIRE(f.read(1) == 21);
  REQUIRE(f.read(2) == 12);
  for (std::size_t p = 0; p < 3; ++p)
    REQUIRE(f.nand().state(p) == atlas::PageState::Free);
  REQUIRE_NOTHROW(f.assert_consistent());
}

TEST_CASE("GC fully invalid block is erased without any live copy", "[gc][integration]") {
  atlas::PageMapping f(atlas::Geometry(1, 1, 1, 2, 2), 1);
  f.write(0, 1);
  f.write(0, 2);
  f.write(0, 3);
  const auto gc = f.collect();
  REQUIRE(gc.victim == 0);
  REQUIRE(gc.migrated == 0);
  REQUIRE(gc.reclaimed == 2);
  REQUIRE(f.free_pages() == 3);
  REQUIRE(f.read(0) == 3);
  REQUIRE_NOTHROW(f.assert_consistent());
  REQUIRE(f.write(0, 4) == 3);
  REQUIRE(f.write(0, 5) == 0);
  REQUIRE(f.read(0) == 5);
}

TEST_CASE("GC blocked preflight preserves all live data mapping and erase counts",
          "[gc][regression]") {
  atlas::PageMapping f(atlas::Geometry(1, 1, 1, 2, 2), 3);
  REQUIRE_THROWS_AS(f.collect(), atlas::GcBlocked);
  f.write(0, 10);
  f.write(1, 11);
  f.write(2, 12);
  f.write(0, 20);
  const std::array expected_states{atlas::PageState::Invalid, atlas::PageState::Valid,
                                   atlas::PageState::Valid, atlas::PageState::Valid};
  REQUIRE_THROWS_AS(f.collect(), atlas::GcBlocked);
  REQUIRE(f.free_pages() == 0);
  REQUIRE(f.needs_gc(0));
  REQUIRE(f.physical(0) == 3);
  REQUIRE(f.physical(1) == 1);
  REQUIRE(f.physical(2) == 2);
  REQUIRE(f.read(0) == 20);
  REQUIRE(f.read(1) == 11);
  REQUIRE(f.read(2) == 12);
  for (std::size_t p = 0; p < 4; ++p)
    REQUIRE(f.nand().state(p) == expected_states[p]);
  REQUIRE(f.nand().erase_count(0) == 0);
  REQUIRE(f.nand().erase_count(1) == 0);
  REQUIRE_NOTHROW(f.assert_consistent());
}

TEST_CASE("GC seeded repeated pressure replay preserves all tokens and deterministic mappings",
          "[gc][property][integration]") {
  constexpr std::uint64_t seed = 0x6CA71A52026;
  std::mt19937_64 rng(seed);
  atlas::PageMapping f(atlas::Geometry(1, 1, 1, 6, 4), 6);
  atlas::PageMapping twin(atlas::Geometry(1, 1, 1, 6, 4), 6);
  std::array<std::optional<std::uint64_t>, 6> expected{};
  std::size_t gc_count = 0;
  for (std::size_t step = 0; step < 1000; ++step) {
    INFO("seed=" << seed << " step=" << step);
    if (f.needs_gc(4)) {
      const auto before = f.free_pages();
      const auto result = f.collect();
      REQUIRE(result == twin.collect());
      REQUIRE(result.reclaimed > 0);
      REQUIRE(f.free_pages() == before + result.reclaimed);
      ++gc_count;
    }
    const auto lpn = static_cast<std::size_t>(rng() % 6);
    const auto value = rng();
    REQUIRE(f.write(lpn, value) == twin.write(lpn, value));
    expected[lpn] = value;
    REQUIRE_NOTHROW(f.assert_consistent());
    REQUIRE_NOTHROW(twin.assert_consistent());
    for (std::size_t l = 0; l < 6; ++l) {
      REQUIRE(f.physical(l) == twin.physical(l));
      if (expected[l])
        REQUIRE(f.read(l) == *expected[l]);
    }
    std::size_t erases = 0;
    for (std::size_t b = 0; b < 6; ++b) {
      erases += f.nand().erase_count(b);
      REQUIRE(f.nand().erase_count(b) == twin.nand().erase_count(b));
    }
    REQUIRE(erases == gc_count);
  }
  REQUIRE(gc_count > 100);
}
