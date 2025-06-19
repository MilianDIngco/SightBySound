cmake_minimum_required(VERSION 3.22)
project(sbs VERSION 2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for nvim LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find OpenAL
find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENAL REQUIRED openal)

# Debug output (optional - remove if not needed)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenAL libraries: ${OPENAL_LIBRARIES}")
message(STATUS "OpenAL include dirs: ${OPENAL_INCLUDE_DIRS}")

# Store all source files in LIB_SOURCES variable (except for main.cpp)
file(GLOB_RECURSE LIB_SOURCES "src/*.cpp")
list(REMOVE_ITEM LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Create a library with all the classes (except for main.cpp)
add_library(sbs-classes ${LIB_SOURCES})
target_include_directories(sbs-classes PUBLIC 
    include
    ${OpenCV_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
)
target_link_libraries(sbs-classes 
    ${OpenCV_LIBS}
    ${OPENAL_LIBRARIES}
)

# Create main executable
add_executable(main src/main.cpp)
target_include_directories(main PRIVATE 
    "${PROJECT_SOURCE_DIR}/include"
    ${OpenCV_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIRS}
)
target_link_libraries(main 
    sbs-classes 
    ${OpenCV_LIBS}
    ${OPENAL_LIBRARIES}
)

# Create build_tests variable
option(BUILD_TESTS "Build tests" ON)
if (BUILD_TESTS)
    enable_testing()
    
    # Fetch Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    
    # Make gtest available
    FetchContent_MakeAvailable(googletest)
    
    # Include test directory
    add_subdirectory(gtests)
endif()
