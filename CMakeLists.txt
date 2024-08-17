cmake_minimum_required(VERSION 3.10)

project(hyperbloom)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

find_package(xxHash CONFIG REQUIRED)

add_executable(bloom bloom/bloom_test.c bloom/bloom.c)
target_link_libraries(bloom PRIVATE xxHash::xxhash)

add_executable(naive_bloom naive-bloom/naive_test.c naive-bloom/naive.c)
target_link_libraries(naive_bloom PRIVATE xxHash::xxhash)