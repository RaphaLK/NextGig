cmake_minimum_required(VERSION 3.14)
project(NextGigBackend LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Firebase SDK paths
link_directories(${CMAKE_SOURCE_DIR}/../firebase_cpp_sdk/libs/linux/x86_64)
include_directories(${CMAKE_SOURCE_DIR}/../firebase_cpp_sdk/include)

add_executable(NextGigBackend
    main.cpp
    server.cpp
    server.h
)

target_link_libraries(NextGigBackend
    firebase_auth
    firebase_database
    firebase_app
)