cmake_minimum_required(VERSION 3.10)

# set the project name
project(Netlink)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_STANDARD 17)

include_directories(include/)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# add the executable
add_executable(netmonitor ${SRC_FILES})
