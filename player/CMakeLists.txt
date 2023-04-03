set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

# the source files
file(GLOB SOURCE_FILES
        "*.h"
        "*.cpp"
        "delegates/*.h"
        "delegates/*.cpp"
        "mafw/*.h"
        "mafw/*.cpp"
        "qml_carview/*.h"
        "qml_carview/*.cpp"
        "qml_entertainmentview/*.h"
        "qml_entertainmentview/*.cpp"
        )

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui Network Svg Xml Quick QuickWidgets OpenGL Qml QuickControls2 QuickCompiler)
find_package(Qt5 REQUIRED COMPONENTS Maemo5)

#qtquick_compiler_add_resources(RESOURCES
#        assets.qrc
#        qml/components/qml_components.qrc
#        qml/chat/chatty/chatty.qrc
#        qml/chat/whatsthat/whatsthat.qrc
#        qml/chat/irssi/irssi.qrc
#        )

#if(NOT MAEMO)
#    qtquick_compiler_add_resources(RESOURCES assets_localdev.qrc)
#endif()

set(EXECUTABLE_FLAG)

add_executable(omp ${EXECUTABLE_FLAG} main.cpp
        ${SOURCE_FILES}
        ${RESOURCES}
        )

set_property(TARGET omp PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

target_include_directories(omp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lyrics)
target_include_directories(omp PUBLIC ${Qt5Gui_PRIVATE_INCLUDE_DIRS})

target_compile_definitions(omp PRIVATE MAEMO=1)
target_compile_definitions(omp PRIVATE MAFW_WORKAROUNDS=1)

if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
    message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
    target_compile_definitions(omp PRIVATE DEBUG=1)
endif()

target_include_directories(omp PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${X11_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Gui_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${Qt5DBus_INCLUDE_DIRS}
        ${Qt5Svg_INCLUDE_DIRS}
        ${Qt5Xml_INCLUDE_DIRS}
        ${Qt5OpenGL_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS}
        ${LIBMAFW_INCLUDE_DIRS}
        ${DBus1_INCLUDE_DIRS}
        ${LIBPIXBUF2_INCLUDE_DIRS}
        ${LIBPLAYBACK1_INCLUDE_DIRS}
        ${LIBDBUS1_INCLUDE_DIRS}
        ${LIBDBUS1_GLIB_INCLUDE_DIRS}
        ${LIBHILDON_THUMBNAIL_INCLUDE_DIRS}
        )

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_NO_KEYWORDS)
if(NOT "${CMAKE_BUILD_TYPE}" EQUAL "Debug")
    add_definitions(-DQT_NO_DEBUG)
endif()

target_compile_definitions(omp
        PUBLIC
        ${Qt5Core_DEFINITIONS}
        ${Qt5Widgets_DEFINITIONS}
        ${Qt5Gui_DEFINITIONS}
        ${Qt5DBus_DEFINITIONS}
        ${Qt5Network_DEFINITIONS}
        ${Qt5Svg_DEFINITIONS}
        ${Qt5Xml_DEFINITIONS}
        ${DBus1_DEFINITIONS}
        )

set(CMAKE_CXX_FLAGS
        ${CMAKE_CXX_FLAGS}
        ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS})

target_link_libraries(omp PUBLIC
        ${CMAKE_DL_LIBS}
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::DBus
        Qt5::Network
        Qt5::Svg
        Qt5::QSvgIconPlugin
        Qt5::QSvgPlugin
        Qt5::Xml
        Qt5::Quick
        Qt5::QuickWidgets
        Qt5::Qml
        Qt5::QuickControls2
        Qt5::OpenGL
        Threads::Threads
        ${GLIB_LIBRARIES}
        ${LIBMAFW_LIBRARIES}
        ${LIBDBUS1_LIBRARIES}
        ${LIBDBUS1_GLIB_LIBRARIES}
        ${LIBPIXBUF2_LIBRARIES}
        ${LIBPLAYBACK1_LIBRARIES}
        ${LIBHILDON_THUMBNAIL_LIBRARIES}
        )

target_link_libraries(omp PUBLIC Qt5::Maemo5)

if(X11_FOUND)
    target_link_libraries(omp PUBLIC ${X11_LIBRARIES})
endif()

install(TARGETS omp
        DESTINATION bin
        )

message(STATUS "openmediaplayer: ${CMAKE_PROJECT_VERSION}")
message(STATUS "libplayback: ${LIBPLAYBACK1_VERSION}")
message(STATUS "mafw: ${LIBMAFW_VERSION} - ${LIBMAFW_LIBRARIES}")
message(STATUS "GLib: ${GLIB_VERSION}")
message(STATUS "gdk-pixbuf: ${LIBPIXBUF2_VERSION}")
