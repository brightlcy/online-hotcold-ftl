#include "atlas/ftl/page_mapping.hpp"
#include "atlas/gc/greedy.hpp"
#include <limits>

namespace atlas {
namespace {
Geometry checked_logical_capacity(Geometry geometry, std::size_t logical_pages) {
  if (logical_pages == 0 || logical_pages > geometry.total_pages()) {
    throw std::invalid_argument("logical page capacity must be within physical capacity");
  }
  return geometry;
}
} // namespace

PageMapping::PageMapping(Geometry geometry, std::size_t logical_pages)
    : nand_(checked_logical_capacity(geometry, logical_pages)), l2p_(logical_pages),
      p2l_(geometry.total_pages()) {}
std::size_t PageMapping::logical_pages() const { return l2p_.size(); }
const Nand &PageMapping::nand() const { return nand_; }
std::optional<std::size_t> PageMapping::physical(std::size_t lpn) const { return l2p_.at(lpn); }
std::optional<std::size_t> PageMapping::owner(std::size_t ppn) const { return p2l_.at(ppn); }
std::uint64_t PageMapping::read(std::size_t lpn) const {
  const auto ppn = l2p_.at(lpn);
  if (!ppn) {
    throw std::out_of_range("logical page has no data");
  }
  return nand_.read(*ppn);
}
std::size_t PageMapping::free_pages() const {
  std::size_t count = 0;
  for (std::size_t p = 0; p < p2l_.size(); ++p) {
    count += nand_.state(p) == PageState::Free;
  }
  return count;
}
std::size_t PageMapping::write(std::size_t lpn, std::uint64_t value) {
  const auto old = l2p_.at(lpn); // Validate LPN before consuming physical space.
  auto dest = next_;
  for (std::size_t tried = 0; tried < p2l_.size(); ++tried) {
    if (nand_.state(dest) == PageState::Free) {
      // Fixed-size maps make the remaining commit sequence allocation-free.
      nand_.program(dest, value);
      l2p_[lpn] = dest;
      p2l_[dest] = lpn;
      if (old) {
        nand_.invalidate(*old);
        p2l_[*old].reset();
      }
      next_ = dest == p2l_.size() - 1 ? 0 : dest + 1;
      return dest;
    }
    dest = dest == p2l_.size() - 1 ? 0 : dest + 1;
  }
  throw NoFreePage();
}
bool PageMapping::needs_gc(std::size_t reserve_pages) const {
  if (reserve_pages > p2l_.size()) {
    throw std::invalid_argument("GC reserve exceeds physical capacity");
  }
  return free_pages() <= reserve_pages;
}
GcResult PageMapping::collect() {
  const auto &g = nand_.geometry();
  std::vector<BlockSummary> blocks(g.total_blocks());
  for (std::size_t p = 0; p < p2l_.size(); ++p) {
    auto &block = blocks[p / g.pages_per_block()];
    switch (nand_.state(p)) {
    case PageState::Free:
      ++block.free;
      break;
    case PageState::Valid:
      ++block.valid;
      break;
    case PageState::Invalid:
      ++block.invalid;
      break;
    }
  }
  const auto victim = select_greedy_victim(blocks);
  if (!victim) {
    throw GcBlocked();
  }
  if (nand_.erase_count(*victim) == std::numeric_limits<std::uint64_t>::max()) {
    throw std::overflow_error("GC victim erase counter overflow");
  }
  struct Move {
    std::size_t source, dest, lpn;
    std::uint64_t value;
  };
  std::vector<std::size_t> destinations;
  destinations.reserve(blocks[*victim].valid);
  for (std::size_t p = 0; p < p2l_.size() && destinations.size() < blocks[*victim].valid; ++p) {
    if (p / g.pages_per_block() != *victim && nand_.state(p) == PageState::Free) {
      destinations.push_back(p);
    }
  }
  std::vector<Move> plan;
  plan.reserve(blocks[*victim].valid);
  const auto begin = *victim * g.pages_per_block();
  for (auto p = begin; p < begin + g.pages_per_block(); ++p) {
    if (nand_.state(p) == PageState::Valid) {
      plan.push_back({p, destinations.at(plan.size()), *p2l_[p], nand_.read(p)});
    }
  }
  // All allocations, bounds checks, payload reads and erase preflight precede mutation.
  for (const auto &move : plan) {
    nand_.program(move.dest, move.value);
    l2p_[move.lpn] = move.dest;
    p2l_[move.dest] = move.lpn;
    nand_.invalidate(move.source);
    p2l_[move.source].reset();
  }
  nand_.erase(*victim);
  return {*victim, plan.size(), blocks[*victim].invalid};
}
void PageMapping::assert_consistent() const {
  for (std::size_t l = 0; l < l2p_.size(); ++l) {
    if (l2p_[l] && (p2l_.at(*l2p_[l]) != l || nand_.state(*l2p_[l]) != PageState::Valid)) {
      throw std::logic_error("L2P/P2L/NAND mismatch");
    }
  }
  for (std::size_t p = 0; p < p2l_.size(); ++p) {
    if (p2l_[p]) {
      if (nand_.state(p) != PageState::Valid || l2p_.at(*p2l_[p]) != p) {
        throw std::logic_error("P2L/L2P/NAND mismatch");
      }
    } else if (nand_.state(p) == PageState::Valid) {
      throw std::logic_error("unowned valid NAND page");
    }
  }
}
} // namespace atlas
