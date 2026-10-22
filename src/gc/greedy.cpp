#include "atlas/gc/greedy.hpp"
#include <limits>
#include <stdexcept>

namespace atlas {
std::optional<std::size_t> select_greedy_victim(std::span<const BlockSummary> blocks) {
  std::size_t total_free = 0;
  for (const auto &block : blocks) {
    if (block.free > std::numeric_limits<std::size_t>::max() - total_free) {
      throw std::overflow_error("GC free-page summary overflow");
    }
    total_free += block.free;
  }
  std::optional<std::size_t> best;
  for (std::size_t b = 0; b < blocks.size(); ++b) {
    const auto &block = blocks[b];
    if (block.invalid == 0 || block.valid > total_free - block.free) {
      continue;
    }
    if (!best || block.invalid > blocks[*best].invalid ||
        (block.invalid == blocks[*best].invalid && block.valid < blocks[*best].valid)) {
      best = b;
    }
  }
  return best;
}
} // namespace atlas
