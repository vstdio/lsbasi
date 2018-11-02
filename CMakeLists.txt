cmake_minimum_required(VERSION 2.8)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(main main.cpp)
target_link_libraries(main ${Boost_LIBRARIES})
