#include "atlas/nand/nand.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace atlas {
namespace {
std::size_t checked_product(std::size_t a, std::size_t b) {
  if (a > std::numeric_limits<std::size_t>::max() / b) {
    throw std::overflow_error("NAND geometry capacity overflow");
  }
  return a * b;
}
} // namespace

Geometry::Geometry(std::size_t channels, std::size_t chips_per_channel, std::size_t dies_per_chip,
                   std::size_t blocks_per_die, std::size_t pages_per_block)
    : channels_(channels), chips_(chips_per_channel), dies_(dies_per_chip), blocks_(blocks_per_die),
      pages_(pages_per_block), total_blocks_(0), total_pages_(0) {
  if (channels_ == 0 || chips_ == 0 || dies_ == 0 || blocks_ == 0 || pages_ == 0) {
    throw std::invalid_argument("NAND geometry dimensions must be positive");
  }
  total_blocks_ =
      checked_product(checked_product(checked_product(channels_, chips_), dies_), blocks_);
  total_pages_ = checked_product(total_blocks_, pages_);
}
std::size_t Geometry::total_pages() const { return total_pages_; }
std::size_t Geometry::total_blocks() const { return total_blocks_; }
std::size_t Geometry::pages_per_block() const { return pages_; }
std::size_t Geometry::encode(Address a) const {
  if (a.channel >= channels_ || a.chip >= chips_ || a.die >= dies_ || a.block >= blocks_ ||
      a.page >= pages_) {
    throw std::out_of_range("NAND address outside geometry");
  }
  return ((((a.channel * chips_) + a.chip) * dies_ + a.die) * blocks_ + a.block) * pages_ + a.page;
}
Address Geometry::decode(std::size_t ppn) const {
  if (ppn >= total_pages_) {
    throw std::out_of_range("NAND page outside geometry");
  }
  Address a{};
  a.page = ppn % pages_;
  ppn /= pages_;
  a.block = ppn % blocks_;
  ppn /= blocks_;
  a.die = ppn % dies_;
  ppn /= dies_;
  a.chip = ppn % chips_;
  a.channel = ppn / chips_;
  return a;
}

Nand::Nand(Geometry geometry)
    : geometry_(geometry), pages_(geometry.total_pages()), erases_(geometry.total_blocks(), 0) {}
const Geometry &Nand::geometry() const { return geometry_; }
PageState Nand::state(std::size_t ppn) const { return pages_.at(ppn).state; }
std::uint64_t Nand::read(std::size_t ppn) const {
  const auto &page = pages_.at(ppn);
  if (page.state != PageState::Valid) {
    throw std::logic_error("cannot read non-valid NAND page");
  }
  return page.value;
}
std::uint64_t Nand::erase_count(std::size_t block) const { return erases_.at(block); }
void Nand::program(std::size_t ppn, std::uint64_t value) {
  auto &page = pages_.at(ppn);
  if (page.state != PageState::Free) {
    throw std::logic_error("cannot program non-free NAND page");
  }
  page.value = value;
  page.state = PageState::Valid;
}
void Nand::invalidate(std::size_t ppn) {
  auto &page = pages_.at(ppn);
  if (page.state != PageState::Valid) {
    throw std::logic_error("cannot invalidate non-valid NAND page");
  }
  page.state = PageState::Invalid;
}
void Nand::erase(std::size_t block) {
  const auto count = erases_.at(block);
  const auto begin = block * geometry_.pages_per_block();
  const auto end = begin + geometry_.pages_per_block();
  for (auto p = begin; p < end; ++p) {
    if (pages_[p].state == PageState::Valid) {
      throw std::logic_error("cannot erase block with valid NAND pages");
    }
  }
  if (count == std::numeric_limits<std::uint64_t>::max()) {
    throw std::overflow_error("NAND erase counter overflow");
  }
  for (auto p = begin; p < end; ++p) {
    pages_[p] = Page{};
  }
  erases_[block] = count + 1;
}
} // namespace atlas
