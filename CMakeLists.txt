cmake_minimum_required(VERSION 3.16)
project(6502 LANGUAGES C)

# Short Form ON - 1 line per test file
set(SHORT_FORM ON)
# SHOW_INDIVIDUAL_TESTS ON - Show pass/fail (and reasson) per individual test, not just file level
set(SHOW_INDIVIDUAL_TESTS OFF)
# Runs the tests but doesn't check the cycles for success
set(DO_VALIDATE ON)

if(SHORT_FORM)
  add_compile_options(-DSHORT_FORM=1)
endif()

if(SHOW_INDIVIDUAL_TESTS)
  add_compile_options(-DSHOW_INDIVIDUAL_TESTS=1)
endif()

if(DO_VALIDATE)
  add_compile_options(-DDO_VALIDATE=1)
endif()

add_executable(${PROJECT_NAME}
    src/6502_core.c
    src/6502.c
    src/65c02.c
    src/cJSON.c
    src/harte.c
    src/log.c
)
