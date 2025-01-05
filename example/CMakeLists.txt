cmake_minimum_required(VERSION 3.5.1)
project(sample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(server_example "${CMAKE_CURRENT_LIST_DIR}/subscribe_control_server.cpp")
add_executable(client_example "${CMAKE_CURRENT_LIST_DIR}/subscribe_client.cpp")

target_link_libraries(server_example PRIVATE rpc_)
target_link_libraries(client_example PRIVATE rpc_)

install(TARGETS server_example DESTINATION bin)
install(TARGETS client_example DESTINATION bin)
# 使用.表示安装到 CMAKE_INSTALL_PREFIX
# 这是安装目录下的内容
# 去掉script/ 中的 / 好像是安装目录
install(DIRECTORY script/ DESTINATION .)