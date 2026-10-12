cmake_minimum_required(VERSION 3.20)
project(tcp-server LANGUAGES C)
add_executable(client client.c)
add_executable(server server.c)
