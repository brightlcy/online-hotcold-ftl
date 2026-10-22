#pragma once
#include <cstddef>
#include <optional>
#include <span>

namespace atlas {
struct BlockSummary {
  std::size_t valid = 0;
  std::size_t invalid = 0;
  std::size_t free = 0;
};
[[nodiscard]] std::optional<std::size_t> select_greedy_victim(std::span<const BlockSummary> blocks);
} // namespace atlas
