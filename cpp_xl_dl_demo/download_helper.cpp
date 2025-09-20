#include "download_helper.h"
#include "http.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <csignal>
#include <filesystem>
#include "http.h"
#include "json.hpp"
#include "fs_utility.h"
#include "xl_dl_sdk.h"

#include <cstdio>
#include <chrono>
#include <thread>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <csignal>  // Ctrl+C
#include <ctime>    // ʱ���
#include <sys/stat.h>  // stat() ��ȡ�ļ���С
#ifdef _WIN32
#include <direct.h>  // _getcwd() ������������
#include <windows.h>
#else
#include <unistd.h>  // getcwd() on Linux/Unix
#include <limits.h>  // PATH_MAX on Linux
#endif
using json = nlohmann::json;


#include <filesystem>
 
#include <iostream>

std::string get_exe_dir() {
#ifdef _WIN32
    char path[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::filesystem::path(path).parent_path().string();
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        path[count] = '\0';
        return std::filesystem::path(path).parent_path().string();
    }
    return "./";  // fallback
#endif
}

int64_t get_remote_file_size(const std::string& url) {
    std::string headers;
    int ret = http_head(url, headers);
    if (ret != 0) return -1;

    int64_t content_length = -1;
    const std::string key = "Content-Length:";
    std::istringstream iss(headers);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find(key) != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string val = line.substr(pos + 1);
                while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) {
                    val.erase(val.begin());
                }
                content_length = std::stoll(val);
                break;
            }
        }
    }
    return content_length;
}

int64_t get_local_file_size(const std::string& path) {
    try {
        return static_cast<int64_t>(std::filesystem::file_size(path));
    } catch (...) {
        return -1;
    }
}

void print_with_timestamp(const char* message) {
    std::time_t now = std::time(nullptr);
    char time_str[20];
#ifdef _WIN32
    struct tm time_info;
    localtime_s(&time_info, &now);
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &time_info);
#else
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
#endif
    printf("[%s] %s", time_str, message);
}

std::string get_file_name_from_url(const std::string& url) {
    size_t last_slash = url.find_last_of('/');
    if (last_slash != std::string::npos) {
        return url.substr(last_slash + 1);
    }
    return "default_filename";
}

std::map<uint64_t, std::string> load_task_map() {
    std::map<uint64_t, std::string> result;
    std::ifstream ifs("tasks_map.json");
    if (!ifs.is_open()) return result;

    try {
        json j;
        ifs >> j;
        for (auto& item : j.items()) {
            uint64_t task_id = std::stoull(item.key());
            result[task_id] = item.value().get<std::string>();
        }
    } catch (...) {}

    return result;
}

void save_task_map(const std::map<uint64_t, std::string>& m) {
    json j;
    for (auto& kv : m) {
        j[std::to_string(kv.first)] = kv.second;
    }
    std::ofstream ofs("tasks_map.json", std::ios::trunc);
    ofs << j.dump(2);
}

std::string get_login_token(const std::string& url, const std::string& app_id) {
    std::ostringstream oss;
    oss << "{\"app_id\":\"" << app_id << "\"}";
    std::string response;
    http_post(url, oss.str(), response);

    try {
        json data = json::parse(response);
        return data["token"];
    } catch (...) {
        return "";
    }
}

std::string get_task_token(const std::string& url, const std::string& app_id, const std::string& session, const std::string& task_url) {
    std::ostringstream oss;
    oss << "{\"app_id\":\"" << app_id << "\",\"session\":\"" << session << "\",\"url\":\"" << task_url << "\"}";
    std::string response;
    http_post(url, oss.str(), response);

    try {
        json data = json::parse(response);
        return data["token"];
    } catch (...) {
        return "";
    }
}

void signal_handler(int signal) {
    printf("\n[Debug] Caught signal %d, exiting...\n", signal);
#ifdef XL_DL_SDK_H // Only call if we have the header included
    xl_dl_uninit();
#endif
    exit(0);
}

bool is_magnet_link(const std::string& url) {
    return url.length() > 8 && url.substr(0, 8) == "magnet:?";
}
