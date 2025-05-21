cmake_minimum_required(VERSION 3.16)

project(NextGig VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
add_subdirectory(frontend)
add_subdirectory(backend)

add_executable(NextGig
    main.cpp
)

target_link_libraries(NextGig PRIVATE Qt5::Widgets)