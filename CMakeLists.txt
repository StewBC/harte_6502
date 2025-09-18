cmake_minimum_required(VERSION 3.16)
project(6502 LANGUAGES C)

# ON sets 1 line per opcode, no error messages
# OFF shows all errors and prints multi-line status messages
set(SHORT_FORM ON)

if(SHORT_FORM)
  add_compile_options(-DSHORT_FORM=1)
endif()

add_executable(${PROJECT_NAME}
    src/6502.c
    src/cJSON.c
    src/harte.c
    src/log.c
)
