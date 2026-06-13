cmake_minimum_required(VERSION 3.26)
project(async_client_server)

set(CMAKE_CXX_STANDARD 14)

find_package(Threads REQUIRED)

add_executable(async_client_server
        src/main.cpp
        src/client/client.cpp
        src/client/client.h
        src/client/client_network.cpp
        src/client/client_network.h
        src/fake_transport.cpp
        src/fake_transport.h
        src/server/command_executor.cpp
        src/server/command_executor.h
        src/server/request_tracker.cpp
        src/server/request_tracker.h
        src/server/server.cpp
        src/server/server.h
        src/server/server_network.cpp
        src/server/server_network.h
        src/options_parser.hpp)

target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
target_include_directories(${PROJECT_NAME} PRIVATE src)
