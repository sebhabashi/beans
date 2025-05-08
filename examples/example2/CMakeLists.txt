cmake_minimum_required(VERSION 3.22)

project(beans_example_2
        VERSION 1.0.0
        DESCRIPTION "A simple example for Beans, implementing a calculator with a Lexer/Parser"
        LANGUAGES CXX
        )

# Fetch Catch2 for examples' unit tests
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.1)
FetchContent_MakeAvailable(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(CTest)
include(Catch)

# Example 2 (Calculator)
add_executable(example2 src/main.cpp)
target_include_directories(example2 PRIVATE "include" include)
target_link_libraries(example2 beans)
add_executable(example2_unit_tests
                    "test/catch2_impl.cpp"
                    "test/test_calculator.cpp"
              )
target_include_directories(example2_unit_tests PRIVATE "include" include)
target_link_libraries(example2_unit_tests beans Catch2::Catch2)
