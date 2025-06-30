cmake_minimum_required(VERSION 3.22)

# 项目名称和使用的语言
project(thread_safe_print LANGUAGES CXX VERSION 1.0.0)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 引入 GNUInstallDirs 模块以规范安装路径
include(GNUInstallDirs)

# 定义库
add_library(thread_safe_print SHARED
  thread_safe_print.cc
)

# 设置头文件路径
target_include_directories(thread_safe_print PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> # 构建时使用源目录
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # 安装后使用安装目录
)

# 安装库文件
install(TARGETS thread_safe_print
  EXPORT thread_safe_print-targets # 导出目标信息
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # 动态库路径
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # 静态库路径（如果有）
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # 可执行文件路径（如果是可执行目标）
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # 头文件路径
)

# 安装头文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ # 头文件目录
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # 安装到 include 路径
  FILES_MATCHING PATTERN "*.h" # 仅匹配 .h 文件
)

# 导出目标信息以供 find_package 使用
install(EXPORT thread_safe_print-targets
  FILE thread_safe_printConfig.cmake # 生成配置文件
  NAMESPACE thread_safe_print:: # 设置命名空间
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/thread_safe_print
)

# 导出构建目录的目标信息（供未安装时使用）
export(EXPORT thread_safe_print-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/thread_safe_printConfig.cmake"
  NAMESPACE thread_safe_print::
)
