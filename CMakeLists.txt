cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 14)
project(p2p_messenger)

# Compile options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

# Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS date_time system regex REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# application
add_library(application STATIC src/application/application.cpp)

# network
add_library(network STATIC src/network/network.cpp)

add_executable(chat src/json.hpp src/main.cpp)
target_link_libraries(chat application)
target_link_libraries(chat network)
target_link_libraries(chat ${Boost_LIBRARIES})
target_compile_options(chat INTERFACE ${CMAKE_CXX_FLAGS})