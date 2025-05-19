cmake_minimum_required(VERSION 3.14)
project(NextGigFrontend LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(NextGigFrontend
    main.cpp
    HomeWindow.cpp
    LoginWindow.cpp
    JobFeed.cpp
)

target_link_libraries(NextGigFrontend PRIVATE Qt5::Widgets)
target_include_directories(NextGigFrontend PRIVATE ../src/models)