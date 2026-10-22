#include "atlas/ftl/page_mapping.hpp"
#include <catch.hpp>
#include <limits>

using atlas::Geometry;
using atlas::PageMapping;
using atlas::PageState;

TEST_CASE("FTL rejects invalid logical capacity", "[mapping][boundary]") {
  REQUIRE_THROWS_AS(PageMapping(Geometry(1, 1, 1, 2, 2), 0), std::invalid_argument);
  REQUIRE_THROWS_AS(PageMapping(Geometry(1, 1, 1, 2, 2), 5), std::invalid_argument);
}

TEST_CASE("FTL starts unmapped and rejects missing reads", "[mapping]") {
  PageMapping f(Geometry(1, 1, 1, 2, 2), 3);
  REQUIRE(f.logical_pages() == 3);
  REQUIRE(f.free_pages() == 4);
  for (std::size_t l = 0; l < 3; ++l) {
    REQUIRE_FALSE(f.physical(l).has_value());
    REQUIRE_THROWS_AS(f.read(l), std::out_of_range);
  }
  for (std::size_t p = 0; p < 4; ++p) {
    REQUIRE_FALSE(f.owner(p).has_value());
  }
  REQUIRE_NOTHROW(f.assert_consistent());
}

TEST_CASE("FTL first last logical pages keep distinct inverse owners", "[mapping]") {
  PageMapping f(Geometry(1, 1, 1, 2, 2), 3);
  REQUIRE(f.write(0, 11) == 0);
  REQUIRE(f.write(2, std::numeric_limits<std::uint64_t>::max()) == 1);
  REQUIRE(f.physical(0) == 0);
  REQUIRE(f.owner(0) == 0);
  REQUIRE(f.physical(2) == 1);
  REQUIRE(f.owner(1) == 2);
  REQUIRE(f.read(0) == 11);
  REQUIRE(f.read(2) == std::numeric_limits<std::uint64_t>::max());
  REQUIRE(f.free_pages() == 2);
  REQUIRE_NOTHROW(f.assert_consistent());
}

TEST_CASE("FTL overwrite invalidates only former owner and preserves other data", "[mapping]") {
  PageMapping f(Geometry(1, 1, 1, 2, 3), 3);
  const auto old = f.write(1, 17);
  const auto other = f.write(2, 19);
  const auto now = f.write(1, 23);
  REQUIRE(now != old);
  REQUIRE(f.nand().state(old) == PageState::Invalid);
  REQUIRE_FALSE(f.owner(old).has_value());
  REQUIRE(f.owner(now) == 1);
  REQUIRE(f.physical(1) == now);
  REQUIRE(f.read(1) == 23);
  REQUIRE(f.physical(2) == other);
  REQUIRE(f.read(2) == 19);
  REQUIRE_NOTHROW(f.assert_consistent());
}

TEST_CASE("FTL repeated overwrite never aliases a current mapping", "[mapping][regression]") {
  PageMapping f(Geometry(1, 1, 1, 2, 4), 1);
  for (std::size_t p = 0; p < 8; ++p) {
    REQUIRE(f.write(0, p) == p);
    REQUIRE(f.physical(0) == p);
    REQUIRE(f.read(0) == p);
    REQUIRE_NOTHROW(f.assert_consistent());
  }
  REQUIRE(f.free_pages() == 0);
  REQUIRE_THROWS_AS(f.write(0, 99), atlas::NoFreePage);
  REQUIRE(f.read(0) == 7);
}

TEST_CASE("FTL full NAND rejected overwrite preserves old data and mappings",
          "[mapping][regression]") {
  PageMapping f(Geometry(1, 1, 1, 1, 2), 2);
  f.write(0, 29);
  f.write(1, 31);
  REQUIRE_THROWS_AS(f.write(0, 37), atlas::NoFreePage);
  REQUIRE(f.read(0) == 29);
  REQUIRE(f.read(1) == 31);
  REQUIRE(f.physical(0) == 0);
  REQUIRE(f.owner(0) == 0);
  REQUIRE(f.free_pages() == 0);
  REQUIRE(f.nand().erase_count(0) == 0);
  REQUIRE_NOTHROW(f.assert_consistent());
}

TEST_CASE("FTL logical and physical bounds errors consume no page", "[mapping][boundary]") {
  PageMapping f(Geometry(1, 1, 1, 1, 2), 1);
  for (const auto l : {std::size_t{1}, std::numeric_limits<std::size_t>::max()}) {
    REQUIRE_THROWS_AS(f.write(l, 41), std::out_of_range);
    REQUIRE_THROWS_AS(f.read(l), std::out_of_range);
    REQUIRE_THROWS_AS(f.physical(l), std::out_of_range);
  }
  REQUIRE_THROWS_AS(f.owner(2), std::out_of_range);
  REQUIRE(f.free_pages() == 2);
  REQUIRE(f.write(0, 43) == 0);
  REQUIRE_NOTHROW(f.assert_consistent());
}
