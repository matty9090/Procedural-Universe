cmake_minimum_required(VERSION 3.5)
project(nbody)

set(CMAKE_CXX_STANDARD 14)

option(ENABLE_TESTING "Turns on testing" OFF)

# CxxOpts
include_directories(lib/CxxOpts/include)

# DirectXTK
add_subdirectory(lib/DirectXTK)
include_directories(lib/DirectXTK/Inc)

# ImGUI
include_directories(lib/imgui)

add_library(imgui STATIC
	lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imconfig.h
    lib/imgui/imgui.h
    lib/imgui/imgui_internal.h
    lib/imgui/imstb_rectpack.h
    lib/imgui/imstb_textedit.h
    lib/imgui/imstb_truetype.h)

# NBody
add_definitions(-DUNICODE)
include_directories(src)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
list(REMOVE_ITEM SRC_FILES src/App/NBody.cpp)

add_library(nbody_lib STATIC ${SRC_FILES})
target_link_libraries(nbody_lib directxtk d3d11 dxgi imgui)

add_executable(nbody WIN32 src/App/NBody.cpp resources/Resource.rc)
target_link_libraries(nbody nbody_lib)

# Tests
if (ENABLE_TESTING)
    enable_testing()

    add_subdirectory(lib/googletest)
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

    file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)
    add_executable(nbody_tests ${TEST_SRC_FILES})

    target_link_libraries(nbody_tests gtest gtest_main nbody_lib)

    add_test(UnitTests nbody_tests)
endif ()