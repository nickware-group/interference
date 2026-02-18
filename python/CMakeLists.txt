cmake_minimum_required(VERSION 3.15)
project(indk_python LANGUAGES CXX)

# C++17 is sufficient with GCC/MinGW (designated initializers supported since GCC 8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(INDK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG QUIET)

if(NOT pybind11_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        v2.13.6
    )
    FetchContent_MakeAvailable(pybind11)
endif()

# facefull-bridge dependency
set(FFB_QT5WEBKIT_EXAMPLE_BUILD OFF CACHE BOOL "" FORCE)
set(FFB_WXWIDGETS_EXAMPLE_BUILD OFF CACHE BOOL "" FORCE)
set(FFB_WB_EXAMPLE_BUILD OFF CACHE BOOL "" FORCE)
add_subdirectory(${INDK_ROOT}/deps/facefull-bridge ${CMAKE_CURRENT_BINARY_DIR}/facefull-bridge)

set(INDK_SOURCES
    ${INDK_ROOT}/src/neuron/neuron.cpp
    ${INDK_ROOT}/src/neuron/entry.cpp
    ${INDK_ROOT}/src/neuron/synapse.cpp
    ${INDK_ROOT}/src/neuron/receptor.cpp
    ${INDK_ROOT}/src/neuralnet.cpp
    ${INDK_ROOT}/src/error.cpp
    ${INDK_ROOT}/src/system.cpp
    ${INDK_ROOT}/src/position.cpp
    ${INDK_ROOT}/src/math.cpp
    ${INDK_ROOT}/src/interlink.cpp
    ${INDK_ROOT}/src/profiler.cpp
    ${INDK_ROOT}/src/instance.cpp
    ${INDK_ROOT}/src/backend.cpp
    ${INDK_ROOT}/src/backends/native.cpp
    ${INDK_ROOT}/src/backends/native_multithread.cpp
    ${INDK_ROOT}/src/backends/opencl.cpp
    ${INDK_ROOT}/src/backends/vulkan.cpp
    ${INDK_ROOT}/src/translators/cpu.cpp
    ${INDK_ROOT}/src/translators/cl.cpp
    ${INDK_ROOT}/src/translators/vk.cpp
)

# Build pybind11 module with interference sources compiled in
pybind11_add_module(_indk
    src/bindings.cpp
    ${INDK_SOURCES}
)

target_include_directories(_indk PRIVATE
    ${INDK_ROOT}/include
    ${INDK_ROOT}/3rdparty
    ${INDK_ROOT}/deps/facefull-bridge/include
)

target_link_libraries(_indk PRIVATE facefull-bridge_static)

find_package(Threads REQUIRED)
target_link_libraries(_indk PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(_indk PRIVATE ws2_32 atomic)
else()
    target_link_libraries(_indk PRIVATE atomic)
endif()

OPTION(INDK_OPENCL_SUPPORT "OpenCL compute backend support" OFF)
OPTION(INDK_VULKAN_SUPPORT "Vulkan compute backend support" OFF)

if(INDK_OPENCL_SUPPORT)
    find_package(OpenCL REQUIRED)
    target_link_libraries(_indk PRIVATE ${OpenCL_LIBRARIES})
    target_include_directories(_indk PRIVATE ${OpenCL_INCLUDE_DIRS})
    target_compile_definitions(_indk PRIVATE
        CL_HPP_TARGET_OPENCL_VERSION=120
        CL_HPP_MINIMUM_OPENCL_VERSION=120
        CL_TARGET_OPENCL_VERSION=120
        INDK_OPENCL_SUPPORT
    )
endif()

if(INDK_VULKAN_SUPPORT)
    find_package(Vulkan REQUIRED)
    target_link_libraries(_indk PRIVATE Vulkan::Vulkan)
    target_include_directories(_indk PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_compile_definitions(_indk PRIVATE INDK_VULKAN_SUPPORT)
endif()

install(TARGETS _indk DESTINATION indk)
