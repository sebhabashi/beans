cmake_minimum_required(VERSION 3.22)
# set(CC gcc)
# set(CCX g++)

project(beans)

set(includes "include")
set(sources "src/beans.cpp")
add_library(beans ${sources})
target_include_directories(beans PUBLIC ${includes})

# Examples
add_executable(example1 examples/example1.cpp)
target_include_directories(example1 PRIVATE "include")
target_link_libraries(example1 PRIVATE beans)

