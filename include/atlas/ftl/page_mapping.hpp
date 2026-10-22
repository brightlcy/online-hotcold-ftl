#pragma once

#include "atlas/nand/nand.hpp"
#include <optional>
#include <stdexcept>

namespace atlas {
class NoFreePage : public std::runtime_error {
public:
  NoFreePage() : std::runtime_error("no free physical page; GC required") {}
};

class PageMapping {
public:
  PageMapping(Geometry geometry, std::size_t logical_pages);
  [[nodiscard]] std::size_t logical_pages() const;
  [[nodiscard]] const Nand &nand() const;
  [[nodiscard]] std::optional<std::size_t> physical(std::size_t lpn) const;
  [[nodiscard]] std::optional<std::size_t> owner(std::size_t ppn) const;
  [[nodiscard]] std::uint64_t read(std::size_t lpn) const;
  [[nodiscard]] std::size_t free_pages() const;
  std::size_t write(std::size_t lpn, std::uint64_t value);
  void assert_consistent() const;

private:
  Nand nand_;
  std::vector<std::optional<std::size_t>> l2p_, p2l_;
  std::size_t next_ = 0;
};
} // namespace atlas
