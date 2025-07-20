cmake_minimum_required(VERSION 3.10)
project(jwarn CXX)

# Add build options
option(VERBOSE_BUILD "Enable verbose build output" OFF)
option(TIME_BUILD "Show build time for each target" OFF)
option(EXPERIMENTAL "Enable experimental features" OFF)

# Set Release as default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()
# Set allowed build type values
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

# Set C++14 standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configure verbose build output
if(VERBOSE_BUILD)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

# Enable build timing
if(TIME_BUILD)
    # Create timing wrapper script
    file(WRITE "${CMAKE_BINARY_DIR}/time_wrapper.cmake" "
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E time \"$ENV{MAKE}\" \"$@\"
            RESULT_VARIABLE result
        )
        if(result)
            message(FATAL_ERROR \"Build failed: \${result}\")
        endif()
    ")
    
    # Setup custom command for build timing
    set(CMAKE_RULE_MESSAGES OFF)
    set(CMAKE_BUILD_TYPE_INIT "${CMAKE_BUILD_TYPE}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")
endif()

# Display build configuration
message(STATUS "=== ${PROJECT_NAME} Build Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build timing: ${TIME_BUILD}")
message(STATUS "Verbose build: ${VERBOSE_BUILD}")
message(STATUS "Experimental features: ${EXPERIMENTAL}")
message(STATUS "========================================")

# Display available options
message(STATUS "=== Available Build Options ===")
message(STATUS "VERBOSE_BUILD: ${VERBOSE_BUILD}")
message(STATUS "  Description: Enable verbose build output")
message(STATUS "  Usage: cmake -DVERBOSE_BUILD=ON|OFF")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Description: Set build configuration")
message(STATUS "  Usage: cmake -DCMAKE_BUILD_TYPE=Debug|Release|MinSizeRel|RelWithDebInfo")
message(STATUS "TIME_BUILD: ${TIME_BUILD}")
message(STATUS "  Description: Show build time for each compilation step")
message(STATUS "  Usage: cmake -DTIME_BUILD=ON|OFF")
message(STATUS "EXPERIMENTAL: ${EXPERIMENTAL}")
message(STATUS "  Description: Enable experimental features")
message(STATUS "  Usage: cmake -DEXPERIMENTAL=ON|OFF")
message(STATUS "========================================")

# Source files
set(SOURCES
    JWARN.C
    TABLE.C
    RULES.C
)

# Header files
set(HEADERS
    TABLE.H
    MESSAGE.H
    portab.h
)

# Create executable
add_executable(jwarn ${SOURCES} ${HEADERS})

# Add compiler definitions
if(EXPERIMENTAL)
    target_compile_definitions(jwarn PRIVATE EXPERIMENTAL)
    message(STATUS "Experimental features enabled")
endif()

# Add Visual Studio specific flags
if(MSVC)
    target_compile_definitions(jwarn PRIVATE 
        _CRT_SECURE_NO_WARNINGS
    )
    message(STATUS "Added Visual Studio specific definitions")
endif()
