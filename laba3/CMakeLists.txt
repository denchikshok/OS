cmake_minimum_required(VERSION 3.10)
project(laba3)

set(CMAKE_CXX_STANDARD 17)

set(SRC
    src/main.cpp
)

if(WIN32)
    list(APPEND SRC
        src/ipc_windows.cpp
        src/mutex_windows.cpp
        src/leader_windows.cpp
    )
elseif(UNIX)
    list(APPEND SRC
        src/ipc_posix.cpp
        src/mutex_posix.cpp
        src/leader_posix.cpp
    )
endif()

add_executable(laba3 ${SRC})

target_include_directories(laba3 PRIVATE include)

if(UNIX)
    target_link_libraries(laba3 pthread rt)
endif()
