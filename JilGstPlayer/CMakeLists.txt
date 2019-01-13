cmake_minimum_required(VERSION 3.0.0)
project(JilGstPlayer VERSION 0.1.0)

set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}")
find_package(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GSTREAMER REQUIRED gstreamer-1.0)

include_directories(${GSTREAMER_INCLUDE_DIRS})
link_directories(${GSTREAMER_LIBRARY_DIRS})

add_library(JilGstPlayer JilGstPlayer.cpp)

target_link_libraries(JilGstPlayer ${GSTREAMER_LIBRARIES})