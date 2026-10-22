#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace atlas {

enum class PageState { Free, Valid, Invalid };

struct Address {
  std::size_t channel;
  std::size_t chip;
  std::size_t die;
  std::size_t block;
  std::size_t page;
  bool operator==(const Address &) const = default;
};

class Geometry {
public:
  Geometry(std::size_t channels, std::size_t chips_per_channel, std::size_t dies_per_chip,
           std::size_t blocks_per_die, std::size_t pages_per_block);
  [[nodiscard]] std::size_t total_pages() const;
  [[nodiscard]] std::size_t total_blocks() const;
  [[nodiscard]] std::size_t pages_per_block() const;
  [[nodiscard]] std::size_t encode(Address address) const;
  [[nodiscard]] Address decode(std::size_t ppn) const;

private:
  std::size_t channels_, chips_, dies_, blocks_, pages_, total_blocks_, total_pages_;
};

class Nand {
public:
  explicit Nand(Geometry geometry);
  [[nodiscard]] const Geometry &geometry() const;
  [[nodiscard]] PageState state(std::size_t ppn) const;
  [[nodiscard]] std::uint64_t read(std::size_t ppn) const;
  [[nodiscard]] std::uint64_t erase_count(std::size_t block) const;
  void program(std::size_t ppn, std::uint64_t value);
  void invalidate(std::size_t ppn);
  void erase(std::size_t block);

private:
  struct Page {
    PageState state = PageState::Free;
    std::uint64_t value = 0;
  };
  Geometry geometry_;
  std::vector<Page> pages_;
  std::vector<std::uint64_t> erases_;
};

} // namespace atlas
