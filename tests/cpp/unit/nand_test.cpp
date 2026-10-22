#include "atlas/nand/nand.hpp"
#include <catch.hpp>
#include <limits>
#include <stdexcept>

using atlas::Address;
using atlas::Geometry;
using atlas::Nand;
using atlas::PageState;

TEST_CASE("NAND geometry rejects zero dimensions and overflow", "[nand][boundary]") {
  REQUIRE_THROWS_AS(Geometry(0, 1, 1, 1, 1), std::invalid_argument);
  REQUIRE_THROWS_AS(Geometry(1, 0, 1, 1, 1), std::invalid_argument);
  REQUIRE_THROWS_AS(Geometry(1, 1, 0, 1, 1), std::invalid_argument);
  REQUIRE_THROWS_AS(Geometry(1, 1, 1, 0, 1), std::invalid_argument);
  REQUIRE_THROWS_AS(Geometry(1, 1, 1, 1, 0), std::invalid_argument);
  const auto max = std::numeric_limits<std::size_t>::max();
  REQUIRE_THROWS_AS(Geometry(max, 2, 1, 1, 1), std::overflow_error);
  REQUIRE_THROWS_AS(Geometry(1, 1, 1, max, 2), std::overflow_error);
}

TEST_CASE("NAND addresses round trip across every hierarchy boundary", "[nand]") {
  const Geometry g(2, 3, 2, 3, 4);
  REQUIRE(g.total_blocks() == 36);
  REQUIRE(g.total_pages() == 144);
  for (std::size_t ppn = 0; ppn < g.total_pages(); ++ppn) {
    const auto a = g.decode(ppn);
    REQUIRE(a.channel == ppn / 72);
    REQUIRE(a.chip == (ppn / 24) % 3);
    REQUIRE(a.die == (ppn / 12) % 2);
    REQUIRE(a.block == (ppn / 4) % 3);
    REQUIRE(a.page == ppn % 4);
    REQUIRE(g.encode(a) == ppn);
  }
}

TEST_CASE("NAND rejects every out of range address coordinate", "[nand][boundary]") {
  const Geometry g(2, 3, 2, 3, 4);
  for (const auto a : {Address{2, 0, 0, 0, 0}, Address{0, 3, 0, 0, 0}, Address{0, 0, 2, 0, 0},
                       Address{0, 0, 0, 3, 0}, Address{0, 0, 0, 0, 4}}) {
    REQUIRE_THROWS_AS(g.encode(a), std::out_of_range);
  }
  REQUIRE_THROWS_AS(g.decode(144), std::out_of_range);
  REQUIRE_THROWS_AS(g.decode(std::numeric_limits<std::size_t>::max()), std::out_of_range);
}

TEST_CASE("NAND initial pages are free and counters zero", "[nand]") {
  const Nand n(Geometry(1, 1, 1, 2, 3));
  REQUIRE(n.geometry().total_pages() == 6);
  for (std::size_t p = 0; p < 6; ++p) {
    REQUIRE(n.state(p) == PageState::Free);
    REQUIRE_THROWS_AS(n.read(p), std::logic_error);
  }
  REQUIRE(n.erase_count(0) == 0);
  REQUIRE(n.erase_count(1) == 0);
}

TEST_CASE("NAND program preserves payload extremes", "[nand]") {
  Nand n(Geometry(1, 1, 1, 1, 2));
  n.program(0, 0);
  n.program(1, std::numeric_limits<std::uint64_t>::max());
  REQUIRE(n.state(0) == PageState::Valid);
  REQUIRE(n.read(0) == 0);
  REQUIRE(n.read(1) == std::numeric_limits<std::uint64_t>::max());
}

TEST_CASE("NAND double program and reprogram invalid are rejected", "[nand][regression]") {
  Nand n(Geometry(1, 1, 1, 1, 2));
  n.program(0, 19);
  REQUIRE_THROWS_AS(n.program(0, 23), std::logic_error);
  REQUIRE(n.read(0) == 19);
  n.invalidate(0);
  REQUIRE_THROWS_AS(n.program(0, 29), std::logic_error);
  REQUIRE(n.state(0) == PageState::Invalid);
  REQUIRE(n.state(1) == PageState::Free);
}

TEST_CASE("NAND invalidate rejects unreadable states and preserves neighbors", "[nand]") {
  Nand n(Geometry(1, 1, 1, 1, 3));
  REQUIRE_THROWS_AS(n.invalidate(0), std::logic_error);
  n.program(0, 31);
  n.program(1, 37);
  n.invalidate(0);
  REQUIRE(n.state(0) == PageState::Invalid);
  REQUIRE_THROWS_AS(n.read(0), std::logic_error);
  REQUIRE_THROWS_AS(n.invalidate(0), std::logic_error);
  REQUIRE(n.read(1) == 37);
  REQUIRE(n.state(2) == PageState::Free);
}

TEST_CASE("NAND erase resets complete block increments once and permits reuse", "[nand]") {
  Nand n(Geometry(1, 1, 1, 2, 3));
  n.program(0, 41);
  n.program(2, 43);
  n.program(3, 47);
  n.invalidate(0);
  n.invalidate(2);
  n.erase(0);
  for (std::size_t p = 0; p < 3; ++p) {
    REQUIRE(n.state(p) == PageState::Free);
  }
  REQUIRE(n.erase_count(0) == 1);
  REQUIRE(n.erase_count(1) == 0);
  REQUIRE(n.read(3) == 47);
  n.program(2, 53);
  REQUIRE(n.read(2) == 53);
  n.invalidate(2);
  n.erase(0);
  REQUIRE(n.erase_count(0) == 2);
}

TEST_CASE("NAND live block erase fails without partial mutation", "[nand][regression]") {
  Nand n(Geometry(1, 1, 1, 1, 3));
  n.program(0, 59);
  n.invalidate(0);
  n.program(2, 61);
  REQUIRE_THROWS_AS(n.erase(0), std::logic_error);
  REQUIRE(n.state(0) == PageState::Invalid);
  REQUIRE(n.state(1) == PageState::Free);
  REQUIRE(n.read(2) == 61);
  REQUIRE(n.erase_count(0) == 0);
}

TEST_CASE("NAND bounds errors never change valid data", "[nand][boundary]") {
  Nand n(Geometry(1, 1, 1, 1, 1));
  n.program(0, 67);
  for (const auto p : {std::size_t{1}, std::numeric_limits<std::size_t>::max()}) {
    REQUIRE_THROWS_AS(n.state(p), std::out_of_range);
    REQUIRE_THROWS_AS(n.read(p), std::out_of_range);
    REQUIRE_THROWS_AS(n.program(p, 71), std::out_of_range);
    REQUIRE_THROWS_AS(n.invalidate(p), std::out_of_range);
    REQUIRE_THROWS_AS(n.erase(p), std::out_of_range);
    REQUIRE_THROWS_AS(n.erase_count(p), std::out_of_range);
  }
  REQUIRE(n.read(0) == 67);
  REQUIRE(n.erase_count(0) == 0);
}
