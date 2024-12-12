cmake_minimum_required(VERSION 3.5.1)
project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(msgpack-cxx_DIR "/usr/local/lib/cmake/msgpack-cxx")
find_package(msgpack-cxx REQUIRED CONFIG)
if(NOT msgpack-cxx_FOUND)
    message(FATAL_ERROR "Could not find msgpack-cxx library.")
endif()

add_executable(test "${CMAKE_CURRENT_LIST_DIR}/test.cpp")

target_link_libraries(test PUBLIC msgpack-cxx)


install(TARGETS test DESTINATION bin)
