cmake_minimum_required(VERSION 3.20)
project(HttpServer)

add_executable(HttpServer src/main.c src/http_manager.c src/http_manager.h)
