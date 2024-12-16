cmake_minimum_required(VERSION 3.5.1)
project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(test "${CMAKE_CURRENT_LIST_DIR}/test.cpp")

target_link_libraries(test PUBLIC rpc_)


install(TARGETS test DESTINATION bin)
