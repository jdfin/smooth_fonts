cmake_minimum_required(VERSION 3.0.0)
project(make_font VERSION 0.1.0)

add_executable(make_font make_font.c)

target_link_libraries(make_font png)
