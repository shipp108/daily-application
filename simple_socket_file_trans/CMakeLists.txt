cmake_minimum_required(VERSION 2.8)
project(simpleSocketFileTransfer)


add_executable(client_test client.c)

add_executable(server_test server.c)
target_link_libraries(server_test -lpthread)

