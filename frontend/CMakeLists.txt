cmake_minimum_required(VERSION 3.14)
project(NextGigFrontend LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(NextGigFrontend
    main.cpp
    LoginWindow.cpp
    JobFeed.cpp
    # add other frontend .cpp files here
)

target_link_libraries(NextGigFrontend PRIVATE Qt6::Widgets)
target_include_directories(NextGigFrontend PRIVATE ../src/models)