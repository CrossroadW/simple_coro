cmake_minimum_required(VERSION 3.21)
project(simple_coro)


find_package(spdlog CONFIG REQUIRED)


add_subdirectory(examples)


if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()