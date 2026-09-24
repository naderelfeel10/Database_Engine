cmake_minimum_required(VERSION 3.10)
project(Database_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include_directories(${CMAKE_SOURCE_DIR})

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE SOURCES "src/*.c++")

add_library(database_lib ${SOURCES})

enable_testing()

add_executable(page_tests
    Testing/Storage/Page/field_test.c++
    Testing/Storage/Page/tuple_test.c++
    Testing/Storage/Page/page_test.c++
)
target_link_libraries(page_tests database_lib gtest_main gtest)
gtest_discover_tests(page_tests)