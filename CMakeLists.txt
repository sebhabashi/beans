cmake_minimum_required(VERSION 3.22)

project(beans
        VERSION 1.0.0
        DESCRIPTION "Open dependency injection library for modern C++"
        LANGUAGES CXX
        )

# ==================================
# Beans library
# ==================================

# Library compilation
include(GNUInstallDirs)

set(includes "${CMAKE_CURRENT_SOURCE_DIR}/include"
             "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
             "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
             )

set(sources "src/beans.cpp")

add_library(beans STATIC ${sources})
target_include_directories(beans PUBLIC ${includes})

# Target properties
set_target_properties(beans PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${PROJECT_VERSION_MAJOR}
                      PUBLIC_HEADER include/beans.hpp
                      )

# Installation
install(TARGETS beans
        EXPORT BeansTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/beans.hpp" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# ==================================
# Examples
# ==================================

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

# Example 1 (Simple beans creation)
add_executable(example1 examples/example1/src/main.cpp)
target_include_directories(example1 PRIVATE "include" examples/example1/include)
target_link_libraries(example1 beans)

# Example 2 (Calculator)
add_executable(example2 examples/example2/src/main.cpp)
target_include_directories(example2 PRIVATE "include" examples/example2/include)
target_link_libraries(example2 beans)
add_executable(example2_unit_tests
                    "examples/example2/test/catch2_impl.cpp"
                    "examples/example2/test/test_calculator.cpp"
              )
target_include_directories(example2_unit_tests PRIVATE "include" examples/example2/include)
target_link_libraries(example2_unit_tests beans Catch2::Catch2)
