cmake_minimum_required(VERSION 3.16)
message(STATUS "Initiating compile using CMake ${CMAKE_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(openmediaplayer
        VERSION 0.0.1
        DESCRIPTION "Open Media Player"
        HOMEPAGE_URL https://github.com/kroketio/kotki
        LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)

#set(CONVERSATIONS_DESKTOP_DIR /usr/share/applications/hildon)
set(THREADS_PREFER_PTHREAD_FLAG ON)
if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_SKIP_RPATH ON)
endif()

#configure_file("cmake/config-conversations.h.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/src/config-conversations.h")

include(FindCcache)
include(CheckIncludeFiles)
include(CheckFunctionExists)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(GLIB REQUIRED)

pkg_search_module(LIBMAFW REQUIRED mafw-shared)
pkg_search_module(LIBPLAYBACK1 REQUIRED libplayback-1)
pkg_search_module(LIBPIXBUF2 REQUIRED gdk-pixbuf-2.0)
pkg_search_module(LIBHILDON REQUIRED hildon-1)
pkg_search_module(LIBHILDON_THUMBNAIL REQUIRED hildon-thumbnail)
pkg_search_module(LIBDBUS1 REQUIRED dbus-1)
pkg_search_module(LIBDBUS1_GLIB REQUIRED dbus-glib-1)

find_package(X11 REQUIRED)
include_directories(${X11_INCLUDE_DIR})
link_directories(${X11_LIBRARIES})

# find_package(DBus1 REQUIRED)

add_subdirectory(player)
#add_subdirectory(desktop)
