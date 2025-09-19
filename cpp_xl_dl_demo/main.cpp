#include "download_helper.h"
#include "fs_utility.h"
#include "xl_dl_sdk.h"
#include <cstdio>
#include <thread>
#include <chrono>
#include <string>

// ========== 你自己的信息 & 常量配置 ==========
const std::string APP_ID = "eGwta3o3SzEwMTYzAAAAA7MnAAA=";
const std::string APP_VER = "1.2.3";
const std::string TOKEN_SERVER = "http://1.94.243.230:8080";
const std::string LOGIN_TOKEN_URL = TOKEN_SERVER + "/login_token";
const std::string TASK_TOKEN_URL  = TOKEN_SERVER + "/task_token";
const std::string DOWNLOAD_URL    = "https://down.sandai.net/thunder11/XunLeiSetup12.0.12.2510.exe";

// ========== 全局目录 & 文件名 ==========
// 可执行文件所在目录
static const std::string SDK_CONFIG_DIR = get_exe_dir() + "\\conf";
static const std::string FILE_SAVE_DIR  = get_exe_dir() + "\\download";
static const std::string SAVE_NAME      = get_file_name_from_url(DOWNLOAD_URL);

// ========== main 函数 ==========

int main() {

    // Ctrl+C 退出
    signal(SIGINT, signal_handler);

    // 初始化所需目录
    if (!ExistDir(SDK_CONFIG_DIR)) CreateDir(SDK_CONFIG_DIR);
    if (!ExistDir(FILE_SAVE_DIR))  CreateDir(FILE_SAVE_DIR);

    // SDK 初始化
    xl_dl_init_param init_param;
    init_param.app_id       = APP_ID.c_str();
    init_param.app_version  = APP_VER.c_str();
    init_param.save_tasks   = 1;
    init_param.cfg_path     = SDK_CONFIG_DIR.c_str();
    xl_dl_init(&init_param);

    // 检查远程文件大小 & 本地文件大小
    int64_t remote_size = get_remote_file_size(DOWNLOAD_URL);
    std::string local_full_path = FILE_SAVE_DIR + "\\" + SAVE_NAME;
    int64_t local_size  = get_local_file_size(local_full_path);

    // 如果本地大小与远程相等，视为下载完成 => 直接进入“P2P做种”模式
    bool skip_download = false;
    if (remote_size > 0 && remote_size == local_size) {
        skip_download = true;
    }

    // 如果已经下载完成 => 进入P2P
    if (skip_download) {
        xl_dl_set_upload_switch(1);
        xl_dl_set_upload_speed_limit(UINT32_MAX);
        while (true) {
            print_with_timestamp("P2P Upload active... Press Ctrl+C to exit.\n");
            std::this_thread::sleep_for(std::chrono::minutes(5));
        }
    }

    // 如果文件未完成，需要继续下载

    // 读取本地任务映射，检查是否有同URL的未完成任务
    auto task_map = load_task_map();
    uint32_t task_count = 0;
    xl_dl_get_unfinished_tasks(nullptr, &task_count);
    uint64_t task_id = 0;
    bool found_same = false;

    if (task_count > 0) {
        // 恢复所有未完成任务
        uint64_t* arr = new uint64_t[task_count];
        xl_dl_get_unfinished_tasks(arr, &task_count);

        for (uint32_t i = 0; i < task_count; i++) {
            uint64_t tid = arr[i];
            // 启动未完成的任务
            xl_dl_start_task(tid);

            // 如果和当前 URL 相同，则复用
            auto it = task_map.find(tid);
            if (it != task_map.end() && it->second == DOWNLOAD_URL) {
                found_same = true;
                task_id = tid;
            }
        }
        delete[] arr;
    }

    // 如果没有相同URL的未完成任务 => 创建新任务
    if (!found_same) {
        xl_dl_create_p2sp_info create_info;
        create_info.save_name = SAVE_NAME.c_str();
        create_info.save_path = FILE_SAVE_DIR.c_str();
        create_info.url       = DOWNLOAD_URL.c_str();

        xl_dl_create_p2sp_task(&create_info, &task_id);

        // 新建任务加入本地映射
        task_map[task_id] = DOWNLOAD_URL;
        save_task_map(task_map);
    }

    // 登录，获取 session
    std::string login_token = get_login_token(LOGIN_TOKEN_URL, APP_ID);
    char session[XL_DL_MAX_SESSION_ID_LEN] = {0};
    xl_dl_login(login_token.c_str(), session);

    // 为当前任务设置 token
    std::string t_token = get_task_token(TASK_TOKEN_URL, APP_ID, session, DOWNLOAD_URL);
    xl_dl_set_task_token(task_id, t_token.c_str());
    xl_dl_start_task(task_id);

    // 开启P2P上传，不限速
    xl_dl_set_upload_switch(1);
    xl_dl_set_upload_speed_limit(UINT32_MAX);

    // 查询下载进度
    while (true) {
        xl_dl_task_state st;
        if (xl_dl_get_task_state(task_id, &st) != 0) {
            break;
        }
        double percent = 0.0;
        if (st.total_size > 0) {
            percent = (100.0 * (double)st.downloaded_size) / (double)st.total_size;
        }

        // 仅保留进度信息
        printf("\rtask_id=%llu, total=%lld, downloaded=%lld, progress=%.2f%%, speed=%lldKB/s ",
               (unsigned long long)task_id,
               (long long)st.total_size,
               (long long)st.downloaded_size,
               percent,
               (long long)(st.speed / 1024)
        );
        fflush(stdout);

        // 如果成功或失败，跳出循环 => 进入P2P做种
        if (st.state_code == XL_DL_TASK_STATUS_SUCCEEDED ||
            st.state_code == XL_DL_TASK_STATUS_FAILED) {
            printf("\nDownload complete, keep P2P active...\n");
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // 进入持续做种模式
    while (true) {
        print_with_timestamp("P2P Upload active... Press Ctrl+C to exit.\n");
        std::this_thread::sleep_for(std::chrono::minutes(5));
    }

    xl_dl_uninit();
    return 0;
}
