cmake_minimum_required(VERSION 3.16)
project(riscv_tcg_emu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/emulator.cpp
    ${CMAKE_SOURCE_DIR}/src/arch/riscv.cpp
    ${CMAKE_SOURCE_DIR}/src/IR/tcg.cpp
    ${CMAKE_SOURCE_DIR}/src/target/host_x86_64.cpp
)

add_executable(riscv_emu ${SOURCES})
target_include_directories(riscv_emu PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(riscv_emu PRIVATE pthread)
