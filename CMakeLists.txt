cmake_minimum_required(VERSION 2.8)

project(lsbasi)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(lsbasi main.cpp)
target_link_libraries(lsbasi ${Boost_LIBRARIES})
