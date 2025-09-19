#ifndef DOWNLOAD_HELPER_H
#define DOWNLOAD_HELPER_H

#include <string>
#include <map>
#include <cstdint>

std::string get_exe_dir();
int64_t get_remote_file_size(const std::string& url);
int64_t get_local_file_size(const std::string& path);
void print_with_timestamp(const char* message);
std::string get_file_name_from_url(const std::string& url);
std::map<uint64_t, std::string> load_task_map();
void save_task_map(const std::map<uint64_t, std::string>& m);
std::string get_login_token(const std::string& url, const std::string& app_id);
std::string get_task_token(const std::string& url, const std::string& app_id, const std::string& session, const std::string& task_url);
void signal_handler(int signal);

#endif  // DOWNLOAD_HELPER_H
