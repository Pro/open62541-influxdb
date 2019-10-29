cmake_minimum_required(VERSION 3.2)
project(open62541-influxdb)


find_package(open62541 1.1 REQUIRED)

find_package(CLI11 CONFIG REQUIRED)
include_directories($<TARGET_PROPERTY:CLI11::CLI11,INTERFACE_INCLUDE_DIRECTORIES>)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/deps/spdlog/include)

add_executable(open62541-influxdb
               src/main.cpp
               src/ServerWatcher.cpp
               )

target_link_libraries(open62541-influxdb open62541::open62541 config++)