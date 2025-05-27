cmake_minimum_required(VERSION 3.14)
project(NextGigFrontend LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Network )

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
    message(FATAL_ERROR "Unsupported operating system")
endif()


add_executable(NextGigFrontend
    main.cpp
    HomeWindow.cpp
    LoginWindow.cpp
    JobFeed.cpp
    SignUpWindow.cpp
    HiringManagerPortal.cpp
    FreelancerPortal.cpp
    client.cpp
    FreelancerProfileEdit.cpp
    UserManager.cpp
    ../src/models/User.cpp 
)

target_include_directories(NextGigFrontend PRIVATE 
    ../src/models 
    ../backend
)

if(UNIX)
    target_link_libraries(NextGigFrontend PRIVATE 
        Qt5::Widgets
        Qt5::Network
        firebase_auth
        firebase_database
        firebase_app
        secret-1
        glib-2.0
        NextGigBackendLib # backend library
    )
elseif(WIN32 OR APPLE)
    target_link_libraries(NextGigFrontend PRIVATE 
        Qt5::Widgets
        Qt5::Network
        firebase_auth
        firebase_database
        firebase_app
        NextGigBackendLib
    )
endif()