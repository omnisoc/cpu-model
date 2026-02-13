cmake_minimum_required(VERSION 3.16)
project(riscv_tcg_emu)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# 定义可执行文件输出目录 (生成到项目根目录下的 bin 文件夹)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# --- 修正部分开始 ---
# 显式列出源文件路径，避免 file(GLOB) 路径解析错误
set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/emulator.cpp
)

# 生成可执行文件
add_executable(riscv_emu ${SOURCES})

# 添加头文件搜索路径
target_include_directories(riscv_emu PRIVATE ${CMAKE_SOURCE_DIR}/src)
# --- 修正部分结束 ---

# 链接 pthread 库
target_link_libraries(riscv_emu PRIVATE pthread)

# 打印构建信息 (调试用)
message(STATUS "源文件: ${SOURCES}")
message(STATUS "输出目录: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
