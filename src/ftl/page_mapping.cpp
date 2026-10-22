#include "atlas/ftl/page_mapping.hpp"

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
