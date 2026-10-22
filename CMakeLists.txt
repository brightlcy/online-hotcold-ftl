cmake_minimum_required(VERSION 3.20)
project(atlas_ssd_core LANGUAGES CXX)

include(CTest)
option(ATLAS_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(ATLAS_COVERAGE "Enable LLVM source coverage" OFF)

add_library(atlas_core src/nand/nand.cpp src/ftl/page_mapping.cpp src/gc/greedy.cpp)
target_include_directories(atlas_core PUBLIC include)
target_compile_features(atlas_core PUBLIC cxx_std_20)
set_target_properties(atlas_core PROPERTIES CXX_EXTENSIONS OFF)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(atlas_core PRIVATE -Wall -Wextra -Wpedantic -Werror)
  if(ATLAS_SANITIZERS)
    target_compile_options(atlas_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(atlas_core PUBLIC -fsanitize=address,undefined)
  endif()
endif()
if(ATLAS_COVERAGE)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "ATLAS_COVERAGE requires Clang/LLVM")
  endif()
  target_compile_options(atlas_core PUBLIC -fprofile-instr-generate -fcoverage-mapping)
  target_link_options(atlas_core PUBLIC -fprofile-instr-generate)
endif()

if(BUILD_TESTING)
  add_executable(atlas_tests tests/cpp/catch_main.cpp tests/cpp/unit/nand_test.cpp
                 tests/cpp/property/nand_property_test.cpp tests/cpp/unit/mapping_test.cpp
                 tests/cpp/integration/mapping_property_test.cpp tests/cpp/unit/gc_test.cpp
                 tests/cpp/integration/gc_integration_test.cpp)
  target_include_directories(atlas_tests SYSTEM PRIVATE third_party/catch2)
  target_link_libraries(atlas_tests PRIVATE atlas_core)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/catch2")
  include(Catch)
  catch_discover_tests(atlas_tests)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  add_executable(atlas_mapping_probe tests/differential/mapping_probe.cpp)
  target_link_libraries(atlas_mapping_probe PRIVATE atlas_core)
  add_test(NAME mapping_python_differential
           COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/differential/test_mapping_differential.py
                   $<TARGET_FILE:atlas_mapping_probe>)
endif()
