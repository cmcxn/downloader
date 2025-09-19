include("/home/runner/work/downloader/downloader/cpp_xl_dl_demo/build/cmake/CPM.cmake")
CPMAddPackage("NAME;curl;GITHUB_REPOSITORY;curl/curl;GIT_TAG;curl-7_82_0;OPTIONS;CURL_USE_LIBSSH2 OFF;CURL_USE_MBEDTLS OFF;BUILD_CURL_EXE OFF;BUILD_SHARED_LIBS OFF;BUILD_TESTING OFF")
set(curl_FOUND TRUE)