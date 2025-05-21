cmake_minimum_required(VERSION 3.14)
project(NextGigBackend LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Firebase SDK Paths -- Anshu and Josh, check ur own 
if(WIN32)
    link_directories(${CMAKE_SOURCE_DIR}/firebase_cpp_sdk/libs/windows/x64)
    include_directories(${CMAKE_SOURCE_DIR}/firebase_cpp_sdk/include)
elseif(APPLE)
    link_directories(${CMAKE_SOURCE_DIR}/firebase_cpp_sdk/libs/macos)
    include_directories(${CMAKE_SOURCE_DIR}/firebase_cpp_sdk/include)
elseif(UNIX)
    link_directories(${CMAKE_SOURCE_DIR}/firebase_cpp_sdk/libs/linux/x86_64/cxx11)
    include_directories(${CMAKE_SOURCE_DIR}/firebase_cpp_sdk/include)
else()
    message(FATAL_ERROR "How??????????????")
endif()

add_executable(NextGigBackend
    main.cpp
    server.cpp
    server.h
)

target_link_libraries(NextGigBackend
    firebase_auth
    firebase_database
    firebase_app
    secret-1
    glib-2.0
)