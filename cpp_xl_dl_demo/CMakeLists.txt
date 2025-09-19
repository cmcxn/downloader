cmake_minimum_required(VERSION 3.14)

project(cpp_xl_dl_demo)

file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

set(CMAKE_CXX_STANDARD 11)

# 设置默认构建类型为 Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(WIN32)
  set(CURL_USE_SCHANNEL ON)
  set(CURL_WINDOWS_SSPI ON)
else()
  message(FATAL_ERROR "This SDK only supports Windows platforms")
endif()

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

include_directories("../")
link_directories("../")

add_executable(cpp_xl_dl_demo main.cpp)

if (WIN32)
  target_link_libraries(cpp_xl_dl_demo libcurl dk.lib)
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}. This SDK only supports Windows.")
endif()




