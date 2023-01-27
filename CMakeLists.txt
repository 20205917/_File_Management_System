cmake_minimum_required(VERSION 3.21)
project(_File_Management_System C)

set(CMAKE_C_STANDARD 99)

add_executable(_File_Management_System main.c ramfs.h ramfs.c hashmap.h hashmap.c)
