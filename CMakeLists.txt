cmake_minimum_required(VERSION 3.5.1)
project(rpc VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 用于调试
# set(CMAKE_BUILD_TYPE Debug)

option(ENABLE_EXAMPLES "Enable building examples" ON)
option(ENABLE_TEST "Enable building test" ON)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/build/install")
file(GLOB_RECURSE RPC_HEADERS 
    "${CMAKE_CURRENT_LIST_DIR}/include/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/include/*.hpp" 
    "${CMAKE_CURRENT_LIST_DIR}/include/*.inl" 
)

file(GLOB_RECURSE RPC_SOURCES 
    "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
)

find_package(Boost REQUIRED COMPONENTS system)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost library.")
endif()

set(msgpack-cxx_DIR "/usr/local/lib/cmake/msgpack-cxx")
find_package(msgpack-cxx REQUIRED CONFIG)
if(NOT msgpack-cxx_FOUND)
    message(FATAL_ERROR "Could not find msgpack-cxx library.")
endif()



add_library(rpc_ SHARED ${RPC_SOURCES} ${RPC_HEADERS})
target_include_directories(rpc_ PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include" ${Boost_INCLUDE_DIRS})
target_link_libraries(rpc_ PUBLIC ${Boost_LIBRARIES} msgpack-cxx)

install(TARGETS rpc_ 
    LIBRARY DESTINATION lib
)

# cmake -DENABLE_EXAMPLES=ON ..
if(ENABLE_EXAMPLES)
    add_subdirectory(example)
endif()

if(ENABLE_TEST)
    add_subdirectory(test)
endif()
