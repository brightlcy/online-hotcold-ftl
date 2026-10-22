// Test-only line protocol: after every write emit committed mapping/NAND state.
#include "atlas/ftl/page_mapping.hpp"
#include <iostream>

int main() {
  atlas::PageMapping f(atlas::Geometry(1, 1, 1, 2, 4), 3);
  std::size_t lpn;
  std::uint64_t value;
  while (std::cin >> lpn >> value) {
    const char *status = "OK";
    try {
      f.write(lpn, value);
    } catch (const atlas::NoFreePage &) {
      status = "NO_SPACE";
    }
    f.assert_consistent();
    std::cout << "{\"status\":\"" << status << "\",\"l2p\":[";
    for (std::size_t l = 0; l < f.logical_pages(); ++l) {
      if (l)
        std::cout << ',';
      if (f.physical(l))
        std::cout << *f.physical(l);
      else
        std::cout << "null";
    }
    std::cout << "],\"p2l\":[";
    for (std::size_t p = 0; p < 8; ++p) {
      if (p)
        std::cout << ',';
      if (f.owner(p))
        std::cout << *f.owner(p);
      else
        std::cout << "null";
    }
    std::cout << "],\"states\":[";
    for (std::size_t p = 0; p < 8; ++p) {
      if (p)
        std::cout << ',';
      const auto state = f.nand().state(p);
      std::cout << (state == atlas::PageState::Free ? 0 : state == atlas::PageState::Valid ? 1 : 2);
    }
    std::cout << "],\"free\":" << f.free_pages() << ",\"erases\":[" << f.nand().erase_count(0)
              << ',' << f.nand().erase_count(1) << "]}\n";
  }
  return std::cin.eof() ? 0 : 2;
}
