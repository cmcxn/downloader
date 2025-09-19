#pragma once

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#endif


inline int CreateDir(const std::string& dir) {
#ifdef WIN32
    if (CreateDirectoryA(dir.c_str(), nullptr) == TRUE) {
        return 0;
    }
    else {
        return GetLastError();
    }
#else
    return ::mkdir(dir.c_str(), 0755);
#endif
        
}

inline bool ExistDir(const std::string& dir) {
#ifdef WIN32
    DWORD attributes = GetFileAttributes(dir.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    struct stat st;
    if (::stat(dir.c_str(), &st) != 0)
        return false;

    return S_ISDIR(st.st_mode);
#endif

}