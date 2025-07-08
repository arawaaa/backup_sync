cmake_minimum_required(VERSION 3.25)

project(backup_sync)

set(CMAKE_CXX_STANDARD 20)
add_executable(backup_sync main.cpp directorystate.cpp)

install(TARGETS backup_sync RUNTIME DESTINATION bin)
