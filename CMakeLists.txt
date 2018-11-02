cmake_minimum_required(VERSION 2.8)

project(lsbasi)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(lsbasi
	src/main.cpp
	src/Token.cpp
	src/TokenType.cpp
	src/Lexer.cpp
	src/AST.h
	src/Token.h
	src/TokenType.h
	src/Lexer.h
)
target_link_libraries(lsbasi ${Boost_LIBRARIES})
