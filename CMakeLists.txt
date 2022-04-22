cmake_minimum_required(VERSION 3.13)

# To compile call cmake as:
# cmake -DCMAKE_SYSTEM_PROCESSOR=msp430f1132 -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-msp430.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_C_COMPILER:FILEPATH=C:/Tools/gcc-msp430/bin/msp430-elf-gcc.exe -DCMAKE_CXX_COMPILER:FILEPATH=C:/Tools/gcc-msp430/bin/msp430-elf-gcc.exe -H. -B../build/MancsMosoLight-msp430-gcc-Debug -G Ninja

set(CMAKE_SYSTEM_NAME Generic)

project(mml.elf VERSION 0.1.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)

set(TESTAPP "1" CACHE STRING "Which test-app to build, 0 is real APP")
get_filename_component(COMPILER_DIR ${CMAKE_C_COMPILER} DIRECTORY)
include_directories(SYSTEM ${COMPILER_DIR}/../include)
link_directories(${COMPILER_DIR}/../include)

add_executable(${PROJECT_NAME}
  $<IF:$<BOOL:${TESTAPP}>,src/test.c,src/main.c>
  src/timer.c
  src/timer.h
  src/gpio.c
  src/gpio.h
  src/blinker.c
  src/blinker.h
  src/context.h
  src/common.c
)

# target_link_libraries(${PROJECT_NAME} PRIVATE )
target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
)

if("${TESTAPP}" GREATER "0")
  target_compile_definitions(${PROJECT_NAME} PRIVATE TEST=${TESTAPP})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})
