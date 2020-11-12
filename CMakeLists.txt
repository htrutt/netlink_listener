cmake_minimum_required(VERSION 3.10)

# set the project name
project(Netlink)

include_directories(include/)

#set(SOURCE
#    src/*)
set(SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp
              ${PROJECT_SOURCE_DIR}/src/netlink_socket.cpp
            )

# add the executable
add_executable(netmonitor ${SRC_FILES})
