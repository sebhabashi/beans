cmake_minimum_required(VERSION 3.22)

project(beans_example_1
        VERSION 1.0.0
        DESCRIPTION "A simple example for Beans"
        LANGUAGES CXX
        )

# Example 1 (Simple beans creation)
add_executable(example1 src/main.cpp)
target_include_directories(example1 PRIVATE "include" include)
target_link_libraries(example1 beans)
