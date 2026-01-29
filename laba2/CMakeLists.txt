cmake_minimum_required(VERSION 3.10)

project(laba2_process)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(process_lib
    process_lib/process.cpp
)

target_include_directories(process_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/process_lib
)

add_executable(test_app
    test_app/main.cpp
)

target_link_libraries(test_app
    process_lib
)
