cmake_minimum_required(VERSION 3.0.0)
project(JilGstPlayerTest VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(JilGstPlayer)

set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}")
find_package(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GSTREAMER REQUIRED gstreamer-1.0)

#include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})

include_directories(${GSTREAMER_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_SOURCE_DIR}/JilGstPlayer)

add_executable(JilGstPlayerTest JilGstPlayerTest.cpp)
target_link_libraries(JilGstPlayerTest JilGstPlayer)

