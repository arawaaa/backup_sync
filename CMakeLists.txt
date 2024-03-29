cmake_minimum_required(VERSION 3.0)

project(backup_sync)

add_executable(backup_sync main.cpp)

install(TARGETS backup_sync RUNTIME DESTINATION bin)
