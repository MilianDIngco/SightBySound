# Get all test files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Create test executable
add_executable(sbstests ${TEST_SOURCES})

find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENAL REQUIRED openal)

# Link with library and google test
target_link_libraries(
  sbstests
  sbs-classes
  GTest::gtest_main
  ${OpenCV_LIBS}
  ${OPENAL_LIBRARIES}
)

# Include directories for tests
target_include_directories(sbstests PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${OpenCV_INCLUDE_DIRS}
  ${OPENAL_INCLUDE_DIRS}
)

include(GoogleTest)
gtest_discover_tests(sbstests)
