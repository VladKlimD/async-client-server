cmake_minimum_required(VERSION 4.1)
project(async_client_server)

set(CMAKE_CXX_STANDARD 14)

add_executable(async_client_server
        src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE
        src)
