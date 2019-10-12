cmake_minimum_required(VERSION 3.12)
project(virtenums-ag)

set(CMAKE_CXX_STANDARD 17)

include_directories(../../thirdparty/ctre/include)

add_executable(virtenums-ag
        src/main.cpp)