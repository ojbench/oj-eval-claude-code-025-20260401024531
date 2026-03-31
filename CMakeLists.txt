cmake_minimum_required(VERSION 3.10)
project(plagiarism_detector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

# The main executable that the OJ will use
add_executable(code main.cpp)
