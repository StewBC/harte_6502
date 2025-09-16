cmake_minimum_required(VERSION 3.16)
project(6502 LANGUAGES C)

add_executable(${PROJECT_NAME}
    src/6502.c
    src/cJSON.c
    src/harte.c
    src/log.c
)
