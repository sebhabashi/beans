cmake_minimum_required(VERSION 3.22)

project(Beans
        VERSION 1.0.0
        DESCRIPTION "Open dependency injection library for modern C++"
        LANGUAGES CXX
        )

# ==================================
# Beans library
# ==================================

# Library compilation
include(GNUInstallDirs)

set(includes "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
             "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
   )

set(sources "src/beans.cpp")

add_library(Beans STATIC ${sources})
target_include_directories(Beans PUBLIC ${includes})

# Target properties
set_target_properties(Beans PROPERTIES
                      CMAKE_CXX_STANDARD 11
                      CMAKE_CXX_STANDARD_REQUIRED ON
                      CMAKE_CXX_EXTENSIONS OFF
                      VERSION ${PROJECT_VERSION}
                      SOVERSION ${PROJECT_VERSION_MAJOR}
                      )

# Installation
install(TARGETS Beans
        EXPORT BeansTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/beans.hpp" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(EXPORT BeansTargets
        FILE BeansTargets.cmake
        NAMESPACE Beans::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Beans"
        )

include(CMakePackageConfigHelpers)
configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/BeansConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/BeansConfig.cmake"
                              INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Beans"
                              )

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/BeansConfig.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Beans"
        )