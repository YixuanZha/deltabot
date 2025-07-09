cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)
project(tests)
add_executable(tests main.cpp)
target_link_libraries(tests PRIVATE cldl)
