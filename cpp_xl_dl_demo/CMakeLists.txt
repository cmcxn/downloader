cmake_minimum_required(VERSION 3.14)

project(cpp_xl_dl_demo)

# 下载 CPM.cmake 依赖管理工具
file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
  set(CURL_USE_SCHANNEL ON)
  set(CURL_WINDOWS_SSPI ON)
else()
  set(CURL_USE_OPENSSL ON)
endif()

# 添加 curl 依赖
CPMAddPackage(
    NAME curl
    GITHUB_REPOSITORY "curl/curl"
    GIT_TAG "curl-7_82_0"
    OPTIONS
        "CURL_USE_LIBSSH2 OFF"
        "CURL_USE_MBEDTLS OFF"
        "BUILD_CURL_EXE OFF"
        "BUILD_SHARED_LIBS OFF"
        "BUILD_TESTING OFF"
)

# 查找系统 libtorrent-rasterbar 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBTORRENT REQUIRED IMPORTED_TARGET libtorrent-rasterbar)

# 包含当前目录头文件和父目录（xl_dl_sdk.h位置）
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/..)
link_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_SOURCE_DIR}/..)

# 添加可执行文件，包含 main.cpp 和 download_helper.cpp
add_executable(cpp_xl_dl_demo main.cpp download_helper.cpp torrent_helper.cpp)

# 添加Windows GUI可执行文件
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  add_executable(cpp_xl_dl_demo_gui main_gui.cpp download_helper.cpp torrent_helper.cpp)
  target_include_directories(cpp_xl_dl_demo_gui PRIVATE ${CMAKE_SOURCE_DIR})
  target_include_directories(cpp_xl_dl_demo_gui PRIVATE ${CMAKE_SOURCE_DIR}/..)
  target_link_libraries(cpp_xl_dl_demo_gui libcurl dk.lib comctl32 PkgConfig::LIBTORRENT)
  # Set GUI subsystem for Windows application
  set_target_properties(cpp_xl_dl_demo_gui PROPERTIES
    WIN32_EXECUTABLE TRUE
  )
  # Add Windows-specific compile definitions
  target_compile_definitions(cpp_xl_dl_demo_gui PRIVATE WIN32 _WIN32)
endif()

# 设置 include 目录，确保头文件可用
target_include_directories(cpp_xl_dl_demo PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(cpp_xl_dl_demo PRIVATE ${CMAKE_SOURCE_DIR}/..)

# 链接必要的库
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  target_link_libraries(cpp_xl_dl_demo libcurl dk.lib PkgConfig::LIBTORRENT)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  # Linux libraries not provided in this Windows-only SDK, link only curl and libtorrent
  target_link_libraries(cpp_xl_dl_demo libcurl PkgConfig::LIBTORRENT)
  message(WARNING "This SDK is designed for Windows. Linux dk library not available.")
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  # macOS libraries not provided in this Windows-only SDK, link only curl and libtorrent
  target_link_libraries(cpp_xl_dl_demo libcurl PkgConfig::LIBTORRENT)
  message(WARNING "This SDK is designed for Windows. macOS dk library not available.")
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
