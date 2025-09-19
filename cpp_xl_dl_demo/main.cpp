#include "http.h"
#include "json.hpp"
#include "fs_utility.h"

#include "xl_dl_sdk.h"

#include <cstdio>
#include <chrono>
#include <thread>
#include <string>
#include <sstream>
#include <string>

using json = nlohmann::json;

const std::string APP_ID = "eGwtcVo4SDEwMDMwAAAAAy4nAAA=";   //TODO: replace with your own app_id
const std::string APP_VER = "1.2.3";                         //TODO

const std::string TOKEN_SERVER = "http://10.10.136.24:8080"; //TODO: replace with your own token server address
const std::string LOGIN_TOKEN_URL = TOKEN_SERVER + "/login_token";

#ifdef WIN32
std::string SDK_CONFIG_DIR = "d:\\xl_dl_sdk_conf";
std::string FILE_SAVE_DIR = "D:\\ThunderDownload";
#else
std::string SDK_CONFIG_DIR = "/tmp/xl_dl_sdk_conf";
std::string FILE_SAVE_DIR = "/tmp/ThunderDownload";
#endif

std::string get_login_token(const std::string& url, const std::string& app_id)
{

    std::ostringstream oss;
    oss << "{";
    oss << "\"app_id\":\"" << app_id << "\"";
    oss << "}";

    std::string post_data = oss.str();
    std::string response;
    std::string token;

    auto ret = http_post(url, post_data, response);
    try {
        json data = json::parse(response);
        token = data["token"];
    }
    catch (std::exception& e) {

    }

    return token;
}

int main(int n, char *argv[]) {

    if (!ExistDir(SDK_CONFIG_DIR)) {
        CreateDir(SDK_CONFIG_DIR);
    }
    if (!ExistDir(FILE_SAVE_DIR)) {
        CreateDir(FILE_SAVE_DIR);
    }

    xl_dl_init_param init_param;
    
    init_param.app_id = APP_ID.c_str();
    init_param.app_version = APP_VER.c_str();
    init_param.save_tasks = 0;
    //The SDK saves configuration files and task information in cfg_path, which must be an existing and writable directory.
    init_param.cfg_path = SDK_CONFIG_DIR.c_str();

    auto code = xl_dl_init(&init_param);
    if (code != 0) {
        printf("init sdk failed, error code:%d\n", code);
        return 0;
    }

    std::string login_token = get_login_token(LOGIN_TOKEN_URL, APP_ID);
    char session[XL_DL_MAX_SESSION_ID_LEN] = {0};
    code = xl_dl_login(login_token.c_str(), session);

    xl_dl_create_p2sp_info create_info;
    create_info.save_name = "XunLeiSetup12.0.12.2510.exe";
    //The SDK saves downloaded files in save_path, which must be an existing and writable directory.
    create_info.save_path = FILE_SAVE_DIR.c_str();
    create_info.url = "https://down.sandai.net/thunder11/XunLeiSetup12.0.12.2510.exe";

    uint64_t task_id = 0;
    code = xl_dl_create_p2sp_task(&create_info, &task_id);
    printf("create task return %d, task id:%lld\n", code, task_id);
    
    code = xl_dl_start_task(task_id);
    
    while (true) {
        xl_dl_task_state state;
        code = xl_dl_get_task_state(task_id, &state);
        printf("\rtotal size: %lld, downloaded: %lld, speed:%lldKB/S, state: %d, err:%d, token err:%d       ", 
            state.total_size, state.downloaded_size, state.speed/1024, state.state_code, state.task_err_code, state.task_token_err);
        fflush(stdout);
        if (state.state_code == XL_DL_TASK_STATUS_SUCCEEDED || state.state_code == XL_DL_TASK_STATUS_FAILED)
            break;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    code = xl_dl_uninit();

    printf("\ndownload process terminated.\n");

    return 0;
}
