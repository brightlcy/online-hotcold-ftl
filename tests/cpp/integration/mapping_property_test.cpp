#include "atlas/ftl/page_mapping.hpp"
#include <array>
#include <catch.hpp>
#include <random>

TEST_CASE("FTL seeded writes and failed writes preserve logical payload oracle",
          "[mapping][property][integration]") {
  constexpr std::uint64_t seed = 0xF71A52026;
  std::mt19937_64 rng(seed);
  atlas::PageMapping f(atlas::Geometry(2, 1, 2, 4, 8), 17);
  std::array<std::optional<std::uint64_t>, 17> expected{};
  for (std::size_t step = 0; step < 300; ++step) {
    const auto lpn = static_cast<std::size_t>(rng() % expected.size());
    const auto value = rng();
    INFO("seed=" << seed << " step=" << step << " lpn=" << lpn);
    if (step < 128) {
      f.write(lpn, value);
      expected[lpn] = value;
    } else {
      REQUIRE_THROWS_AS(f.write(lpn, value), atlas::NoFreePage);
    }
    REQUIRE_NOTHROW(f.assert_consistent());
    for (std::size_t l = 0; l < expected.size(); ++l) {
      if (expected[l]) {
        REQUIRE(f.read(l) == *expected[l]);
      } else {
        REQUIRE_THROWS_AS(f.read(l), std::out_of_range);
      }
    }
  }
}
