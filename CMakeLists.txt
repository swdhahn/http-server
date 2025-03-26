cmake_minimum_required(VERSION 3.20)
project(HttpServer)

set(CMAKE_BUILD_TYPE Debug)

add_executable(HttpServer src/main.c src/http_manager.c src/http_manager.h
                          src/file_tree.h src/file_tree.c)
target_link_libraries(HttpServer m)
