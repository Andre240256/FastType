cmake_minimum_required(VERSION 3.10)

project(FastTyping)

set(CMAKE_C_STANDART 11)
set(CMAKE_C_STANDART_REQUIRED ON)

if(MSCV)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -g)
endif()

set(SOURCES
    src/main.c
    src/terminal.c
)

add_executable(ctt ${SOURCES})
target_include_directories(ctt PRIVATE src/include)

file(COPY ${CMAKE_SOURCE_DIR}/src/configs DESTINATION ${CMAKE_BINARY_DIR}/src)