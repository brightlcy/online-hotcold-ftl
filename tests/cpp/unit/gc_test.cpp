#include "atlas/ftl/page_mapping.hpp"
#include "atlas/gc/greedy.hpp"
#include <array>
#include <catch.hpp>
#include <limits>

TEST_CASE("GC victim requires garbage and space outside victim", "[gc][boundary]") {
  REQUIRE_FALSE(atlas::select_greedy_victim({}));
  const std::array no_garbage{atlas::BlockSummary{3, 0, 1}, atlas::BlockSummary{0, 0, 4}};
  REQUIRE_FALSE(atlas::select_greedy_victim(no_garbage));
  const std::array self_space_only{atlas::BlockSummary{1, 2, 1}};
  REQUIRE_FALSE(atlas::select_greedy_victim(self_space_only));
  const std::array no_copy_needed{atlas::BlockSummary{0, 3, 1}};
  REQUIRE(atlas::select_greedy_victim(no_copy_needed) == 0);
}

TEST_CASE("GC greedy selection ranks garbage then live copies then block id", "[gc]") {
  const std::array garbage{atlas::BlockSummary{2, 2, 0}, atlas::BlockSummary{1, 3, 0},
                           atlas::BlockSummary{0, 0, 4}};
  REQUIRE(atlas::select_greedy_victim(garbage) == 1);
  const std::array live_tie{atlas::BlockSummary{2, 1, 1}, atlas::BlockSummary{1, 1, 2},
                            atlas::BlockSummary{0, 0, 4}};
  REQUIRE(atlas::select_greedy_victim(live_tie) == 1);
  const std::array id_tie{atlas::BlockSummary{1, 2, 1}, atlas::BlockSummary{1, 2, 1}};
  REQUIRE(atlas::select_greedy_victim(id_tie) == 0);
}

TEST_CASE("GC free space summary overflow is rejected", "[gc][boundary]") {
  const std::array blocks{atlas::BlockSummary{0, 0, std::numeric_limits<std::size_t>::max()},
                          atlas::BlockSummary{0, 0, 1}};
  REQUIRE_THROWS_AS(atlas::select_greedy_victim(blocks), std::overflow_error);
}

TEST_CASE("GC trigger honors inclusive reserve boundary and rejects impossible reserve", "[gc]") {
  atlas::PageMapping f(atlas::Geometry(1, 1, 1, 2, 3), 2);
  REQUIRE_FALSE(f.needs_gc(4));
  f.write(0, 1);
  REQUIRE_FALSE(f.needs_gc(4));
  f.write(1, 2);
  REQUIRE(f.needs_gc(4));
  REQUIRE(f.needs_gc(6));
  REQUIRE_FALSE(f.needs_gc(0));
  REQUIRE_THROWS_AS(f.needs_gc(7), std::invalid_argument);
}
