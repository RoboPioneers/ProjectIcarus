#==============================
# Requirements
#==============================

cmake_minimum_required(VERSION 3.10)

#==============================
# Project Settings
#==============================

set(TARGET_NAME "Detector")
set(TARGET_NAMESPACE "Icarus")
set(TARGET_VERSION 0.9.1)

if (NOT PROJECT_DECLARED)
    project("${TARGET_NAMESPACE} ${TARGET_NAME}" LANGUAGES CXX VERSION ${TARGET_VERSION})
    set(PROJECT_DECLARED)
endif()

if (NOT CMAKE_CUDA_COMPILER)
    set(CMAKE_CUDA_COMPILER /usr/local/cuda/bin/nvcc)
endif()
enable_language(CUDA)

#==============================
# Command Lines
#==============================

set(CMAKE_CXX_STANDARD 17)

#==============================
# Source
#==============================

# C++ Source Files
FILE(GLOB_RECURSE TARGET_SOURCE *.cpp)
# C++ Header Files
FILE(GLOB_RECURSE TARGET_HEADER *.hpp)
# Protobuf Files
FILE(GLOB_RECURSE TARGET_PROTO *.proto)
# CUDA Source Files
FILE(GLOB_RECURSE TARGET_CUDA_SOURCE *.cu)
# CUDA Header Files
FILE(GLOB_RECURSE TARGET_CUDA_HEADER *.cuh)

#==============================
# Compile Targets
#==============================

find_package(Protobuf REQUIRED)
if (TARGET_PROTO)
    protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS ${TARGET_PROTO})
endif()

# To lower the running cost, this module will be compiled as a static library.
add_executable(${TARGET_NAME}
        ${TARGET_SOURCE} ${TARGET_HEADER}
        ${TARGET_CUDA_SOURCE} ${TARGET_CUDA_HEADER}
        ${PROTO_SOURCES} ${PROTO_HEADERS})

# Set version property.
set_target_properties(${TARGET_NAME} PROPERTIES
        VERSION ${TARGET_VERSION}
        OUTPUT_NAME ${TARGET_NAMESPACE}${TARGET_NAME})

# Enable 'DEBUG' Macro in Debug Mode
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DDEBUG)
endif()

if(OFFLINE_MODE)
    target_compile_definitions(${TARGET_NAME} PRIVATE -DOFFLINE)
endif()

#==============================
# Dependencies
#==============================

# Macro for adding a custom module to a specific target.
macro(add_custom_module target_name visibility module_name)
    find_path(${module_name}_INCLUDE_DIRS "${module_name}")
    find_library(${module_name}_LIBS "${module_name}")
    target_include_directories(${target_name} ${visibility} ${${module_name}_INCLUDE_DIRS})
    target_link_libraries(${target_name} ${visibility} ${${module_name}_LIBS})
endmacro()

find_package(GaiaFramework 0.9)
target_include_directories(${TARGET_NAME} PUBLIC ${GaiaFramework_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC Gaia::Framework)

find_package(GaiaTags 0.9)
target_include_directories(${TARGET_NAME} PUBLIC ${GaiaTags_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC Gaia::Tags)

find_package(GaiaReflection 0.9)
target_include_directories(${TARGET_NAME} PUBLIC ${GaiaReflection_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC Gaia::Reflection)

find_package(GaiaComponents 0.9)
target_include_directories(${TARGET_NAME} PUBLIC ${GaiaComponents_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC Gaia::Components)

find_package(GaiaBehaviorTree 0.9)
target_include_directories(${TARGET_NAME} PUBLIC ${GaiaBehaviorTree_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC Gaia::BehaviorTree)

find_package(GaiaBlackboards 0.9)
target_include_directories(${TARGET_NAME} PUBLIC ${GaiaBlackboards_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC Gaia::Blackboards)

find_package(GaiaExceptions 0.9)
target_include_directories(${TARGET_NAME} PUBLIC ${GaiaExceptions_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC Gaia::Exceptions)

find_package(GaiaBackground 0.9)
target_include_directories(${TARGET_NAME} PUBLIC ${GaiaBackground_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC Gaia::Background)

add_custom_module(${TARGET_NAME} PUBLIC GaiaCameraClient)
add_custom_module(${TARGET_NAME} PUBLIC GaiaSerialIOClient)
add_custom_module(${TARGET_NAME} PUBLIC GaiaInspectionClient)
add_custom_module(${TARGET_NAME} PUBLIC GaiaSharedPicture)

# Protobuf
target_include_directories(${TARGET_NAME} PUBLIC ${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${TARGET_NAME} PUBLIC ${Protobuf_LIBRARIES})

# OpenCV
find_package(OpenCV REQUIRED)
target_include_directories(${TARGET_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC ${OpenCV_LIBRARIES})

# TBB
find_path(TBB_INCLUDE "tbb/tbb.h")
find_library(TBB_LIB "libtbb.so")
target_include_directories(${TARGET_NAME} PRIVATE ${TBB_INCLUDE})
target_link_libraries(${TARGET_NAME} PRIVATE ${TBB_LIB})

# Boost
find_package(Boost 1.65 REQUIRED COMPONENTS system filesystem program_options stacktrace_backtrace)
target_include_directories(${TARGET_NAME} PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE ${Boost_LIBRARIES})

# redis-plus-plus
find_path(REDIS_INCLUDE_DIRS "sw")
find_library(REDIS_LIBRARIES "redis++")
target_include_directories(${TARGET_NAME} PUBLIC ${REDIS_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC ${REDIS_LIBRARIES})

# In Linux, 'Threads' need to explicitly linked.
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(Threads)
    target_link_libraries(${TARGET_NAME} PUBLIC ${CMAKE_THREAD_LIBS_INIT})
endif()

install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)