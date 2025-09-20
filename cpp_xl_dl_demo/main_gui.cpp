#include "download_helper.h"
#include "fs_utility.h"
#include "xl_dl_sdk.h"
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <thread>
#include <chrono>
#include <map>

#pragma comment(lib, "comctl32.lib")

// Window controls IDs
#define ID_EDIT_APP_ID      1001
#define ID_EDIT_TOKEN_SERVER 1002
#define ID_EDIT_DOWNLOAD_URL 1003
#define ID_BUTTON_START     1004
#define ID_PROGRESS_BAR     1005
#define ID_STATIC_STATUS    1006

// Timer ID for progress updates
#define TIMER_PROGRESS      2001

// Global variables
HWND g_hWnd = NULL;
HWND g_hEditAppId = NULL;
HWND g_hEditTokenServer = NULL;
HWND g_hEditDownloadUrl = NULL;
HWND g_hButtonStart = NULL;
HWND g_hProgressBar = NULL;
HWND g_hStatusLabel = NULL;

// Download state
bool g_downloadInProgress = false;
bool g_lastDownloadCompleted = false;  // Track if last download was completed
bool g_sdkInitialized = false;  // Track if SDK has been initialized
uint64_t g_currentTaskId = 0;
std::thread g_downloadThread;

// Default values from original main.cpp
const std::string DEFAULT_APP_ID = "eGwta3o3SzEwMTYzAAAAA7MnAAA=";
const std::string DEFAULT_TOKEN_SERVER = "http://1.94.243.230:8080";
const std::string DEFAULT_DOWNLOAD_URL = "https://down.sandai.net/thunder11/XunLeiSetup12.0.12.2510.exe";
const std::string APP_VER = "1.2.3";

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeControls(HWND hwnd);
void OnStartDownload();
void UpdateProgress();
void DownloadWorkerThread(std::string appId, std::string tokenServer, std::string downloadUrl);
std::string GetWindowText(HWND hwnd);
void SetStatusText(const std::string& text);
void CheckForUnfinishedTasks();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Initialize common controls
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icc);

    // Register window class
    const char* CLASS_NAME = "DownloaderWindow";
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClass(&wc);

    // Create main window
    g_hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Thunder Downloader GUI",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL
    );

    if (g_hWnd == NULL) {
        return 0;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        InitializeControls(hwnd);
        CheckForUnfinishedTasks();
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_BUTTON_START) {
            OnStartDownload();
        }
        // Handle messages from download thread
        else if (LOWORD(wParam) == 0 && !g_downloadInProgress) {
            EnableWindow(g_hButtonStart, TRUE);
        }
        break;

    case WM_KEYDOWN:
        // Handle Ctrl+A for edit controls
        if (wParam == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
            HWND focusedWindow = GetFocus();
            if (focusedWindow == g_hEditAppId || 
                focusedWindow == g_hEditTokenServer || 
                focusedWindow == g_hEditDownloadUrl) {
                // Select all text in the focused edit control
                SendMessage(focusedWindow, EM_SETSEL, 0, -1);
                return 0; // Message handled
            }
        }
        break;

    case WM_TIMER:
        if (wParam == TIMER_PROGRESS) {
            UpdateProgress();
        }
        break;

    case WM_DESTROY:
        if (g_downloadInProgress && g_downloadThread.joinable()) {
            g_downloadInProgress = false;
            g_downloadThread.detach(); // Let the thread finish on its own
        }
        KillTimer(hwnd, TIMER_PROGRESS);
        if (g_sdkInitialized) {
            xl_dl_uninit(); // Clean up SDK
            g_sdkInitialized = false;
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void InitializeControls(HWND hwnd)
{
    // APP_ID label and edit
    CreateWindow("STATIC", "APP ID:",
        WS_VISIBLE | WS_CHILD,
        20, 20, 100, 20,
        hwnd, NULL, NULL, NULL);

    g_hEditAppId = CreateWindow("EDIT", DEFAULT_APP_ID.c_str(),
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        130, 18, 400, 24,
        hwnd, (HMENU)ID_EDIT_APP_ID, NULL, NULL);

    // Token Server label and edit
    CreateWindow("STATIC", "Token Server:",
        WS_VISIBLE | WS_CHILD,
        20, 60, 100, 20,
        hwnd, NULL, NULL, NULL);

    g_hEditTokenServer = CreateWindow("EDIT", DEFAULT_TOKEN_SERVER.c_str(),
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        130, 58, 400, 24,
        hwnd, (HMENU)ID_EDIT_TOKEN_SERVER, NULL, NULL);

    // Download URL label and edit
    CreateWindow("STATIC", "Download URL:",
        WS_VISIBLE | WS_CHILD,
        20, 100, 100, 20,
        hwnd, NULL, NULL, NULL);

    g_hEditDownloadUrl = CreateWindow("EDIT", DEFAULT_DOWNLOAD_URL.c_str(),
        WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
        130, 98, 400, 24,
        hwnd, (HMENU)ID_EDIT_DOWNLOAD_URL, NULL, NULL);

    // Start download button
    g_hButtonStart = CreateWindow("BUTTON", "Start Download",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        130, 140, 120, 30,
        hwnd, (HMENU)ID_BUTTON_START, NULL, NULL);

    // Progress bar
    CreateWindow("STATIC", "Progress:",
        WS_VISIBLE | WS_CHILD,
        20, 190, 100, 20,
        hwnd, NULL, NULL, NULL);

    g_hProgressBar = CreateWindow(PROGRESS_CLASS, NULL,
        WS_VISIBLE | WS_CHILD,
        130, 188, 400, 24,
        hwnd, (HMENU)ID_PROGRESS_BAR, NULL, NULL);

    // Set progress bar range
    SendMessage(g_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(g_hProgressBar, PBM_SETPOS, 0, 0);

    // Status label
    g_hStatusLabel = CreateWindow("STATIC", "Ready to download",
        WS_VISIBLE | WS_CHILD,
        20, 230, 520, 60,
        hwnd, (HMENU)ID_STATIC_STATUS, NULL, NULL);
}

std::string GetWindowText(HWND hwnd)
{
    int len = GetWindowTextLength(hwnd);
    if (len == 0) return "";
    
    char* buffer = new char[len + 1];
    GetWindowTextA(hwnd, buffer, len + 1);
    std::string result(buffer);
    delete[] buffer;
    return result;
}

void SetStatusText(const std::string& text)
{
    SetWindowTextA(g_hStatusLabel, text.c_str());
}

void OnStartDownload()
{
    if (g_downloadInProgress) {
        SetStatusText("Download already in progress...");
        return;
    }

    // Check if last download was completed - offer to start a new download
    if (g_lastDownloadCompleted) {
        // Get current download URL to check if it's the same
        std::string downloadUrl = GetWindowText(g_hEditDownloadUrl);
        int result = MessageBox(g_hWnd, 
            "Previous download was completed. Do you want to start a new download?", 
            "Download Completed", 
            MB_YESNO | MB_ICONQUESTION);
        
        if (result == IDNO) {
            SetStatusText("Download already completed! Check the download folder.");
            return;
        }
        
        // User wants to start a new download, reset the completion flag
        g_lastDownloadCompleted = false;
    }

    // Ensure previous thread is cleaned up before starting new one
    if (g_downloadThread.joinable()) {
        g_downloadThread.join();
    }

    // Get values from edit controls
    std::string appId = GetWindowText(g_hEditAppId);
    std::string tokenServer = GetWindowText(g_hEditTokenServer);
    std::string downloadUrl = GetWindowText(g_hEditDownloadUrl);

    if (appId.empty() || tokenServer.empty() || downloadUrl.empty()) {
        SetStatusText("Please fill in all fields");
        return;
    }

    // Disable start button and reset progress
    EnableWindow(g_hButtonStart, FALSE);
    SendMessage(g_hProgressBar, PBM_SETPOS, 0, 0);
    SetStatusText("Initializing download...");

    // Start download in separate thread
    g_downloadInProgress = true;
    g_downloadThread = std::thread(DownloadWorkerThread, appId, tokenServer, downloadUrl);
    
    // Start timer for progress updates
    SetTimer(g_hWnd, TIMER_PROGRESS, 1000, NULL);
}

void UpdateProgress()
{
    if (!g_downloadInProgress || g_currentTaskId == 0) {
        return;
    }

    xl_dl_task_state st;
    if (xl_dl_get_task_state(g_currentTaskId, &st) != 0) {
        return;
    }

    double percent = 0.0;
    if (st.total_size > 0) {
        percent = (100.0 * (double)st.downloaded_size) / (double)st.total_size;
    }

    // Update progress bar
    SendMessage(g_hProgressBar, PBM_SETPOS, (int)percent, 0);

    // Update status text
    char statusText[512];
    sprintf(statusText, "Progress: %.2f%% (%lld/%lld bytes) Speed: %lld KB/s",
        percent,
        (long long)st.downloaded_size,
        (long long)st.total_size,
        (long long)(st.speed / 1024));
    SetStatusText(statusText);

    // Check if download completed
    if (st.state_code == XL_DL_TASK_STATUS_SUCCEEDED) {
        SetStatusText("Download completed successfully!");
        g_downloadInProgress = false;
        g_lastDownloadCompleted = true;  // Mark that last download was completed
        EnableWindow(g_hButtonStart, TRUE);
        KillTimer(g_hWnd, TIMER_PROGRESS);
        
        // Properly join the download thread
        if (g_downloadThread.joinable()) {
            g_downloadThread.join();
        }
    }
    else if (st.state_code == XL_DL_TASK_STATUS_FAILED) {
        SetStatusText("Download failed!");
        g_downloadInProgress = false;
        g_lastDownloadCompleted = false;  // Reset completion flag on failure
        EnableWindow(g_hButtonStart, TRUE);
        KillTimer(g_hWnd, TIMER_PROGRESS);
        
        // Properly join the download thread
        if (g_downloadThread.joinable()) {
            g_downloadThread.join();
        }
    }
}

void DownloadWorkerThread(std::string appId, std::string tokenServer, std::string downloadUrl)
{
    try {
        // Initialize directories (reused from original main.cpp)
        std::string SDK_CONFIG_DIR = get_exe_dir() + "\\conf";
        std::string FILE_SAVE_DIR = get_exe_dir() + "\\download";
        
        if (!ExistDir(SDK_CONFIG_DIR)) CreateDir(SDK_CONFIG_DIR);
        if (!ExistDir(FILE_SAVE_DIR)) CreateDir(FILE_SAVE_DIR);

        // SDK initialization - only initialize once
        if (!g_sdkInitialized) {
            xl_dl_init_param init_param;
            init_param.app_id = appId.c_str();
            init_param.app_version = APP_VER.c_str();
            init_param.save_tasks = 1;
            init_param.cfg_path = SDK_CONFIG_DIR.c_str();
            
            if (xl_dl_init(&init_param) != 0) {
                SetStatusText("Failed to initialize SDK");
                g_downloadInProgress = false;
                PostMessage(g_hWnd, WM_COMMAND, MAKEWPARAM(0, 0), 0); // Trigger UI update
                return;
            }
            g_sdkInitialized = true;
            SetStatusText("SDK initialized successfully");
        }

        // Check existing tasks
        auto task_map = load_task_map();
        uint32_t task_count = 0;
        xl_dl_get_unfinished_tasks(nullptr, &task_count);
        uint64_t task_id = 0;
        bool found_same = false;

        if (task_count > 0) {
            uint64_t* arr = new uint64_t[task_count];
            xl_dl_get_unfinished_tasks(arr, &task_count);

            for (uint32_t i = 0; i < task_count; i++) {
                uint64_t tid = arr[i];
                xl_dl_start_task(tid);

                auto it = task_map.find(tid);
                if (it != task_map.end() && it->second == downloadUrl) {
                    found_same = true;
                    task_id = tid;
                    SetStatusText("Resuming previous download...");
                }
            }
            delete[] arr;
        }

        // Create new task if needed
        if (!found_same) {
            std::string save_name = get_file_name_from_url(downloadUrl);
            xl_dl_create_p2sp_info create_info;
            create_info.save_name = save_name.c_str();
            create_info.save_path = FILE_SAVE_DIR.c_str();
            create_info.url = downloadUrl.c_str();

            if (xl_dl_create_p2sp_task(&create_info, &task_id) != 0) {
                SetStatusText("Failed to create download task");
                g_downloadInProgress = false;
                PostMessage(g_hWnd, WM_COMMAND, MAKEWPARAM(0, 0), 0); // Trigger UI update
                return;
            }

            task_map[task_id] = downloadUrl;
            save_task_map(task_map);
        }

        g_currentTaskId = task_id;

        // Login and get tokens
        std::string LOGIN_TOKEN_URL = tokenServer + "/login_token";
        std::string TASK_TOKEN_URL = tokenServer + "/task_token";
        
        std::string login_token = get_login_token(LOGIN_TOKEN_URL, appId);
        if (login_token.empty()) {
            SetStatusText("Failed to get login token");
            g_downloadInProgress = false;
            PostMessage(g_hWnd, WM_COMMAND, MAKEWPARAM(0, 0), 0); // Trigger UI update
            return;
        }

        char session[XL_DL_MAX_SESSION_ID_LEN] = {0};
        if (xl_dl_login(login_token.c_str(), session) != 0) {
            SetStatusText("Failed to login");
            g_downloadInProgress = false;
            PostMessage(g_hWnd, WM_COMMAND, MAKEWPARAM(0, 0), 0); // Trigger UI update
            return;
        }

        std::string t_token = get_task_token(TASK_TOKEN_URL, appId, session, downloadUrl);
        if (t_token.empty()) {
            SetStatusText("Failed to get task token");
            g_downloadInProgress = false;
            PostMessage(g_hWnd, WM_COMMAND, MAKEWPARAM(0, 0), 0); // Trigger UI update
            return;
        }

        xl_dl_set_task_token(task_id, t_token.c_str());
        xl_dl_start_task(task_id);

        // Enable P2P upload
        xl_dl_set_upload_switch(1);
        xl_dl_set_upload_speed_limit(UINT32_MAX);

        SetStatusText("Download started successfully!");

    } catch (...) {
        SetStatusText("An error occurred during download setup");
        g_downloadInProgress = false;
        PostMessage(g_hWnd, WM_COMMAND, MAKEWPARAM(0, 0), 0); // Trigger UI update
    }
}

void CheckForUnfinishedTasks()
{
    // Check if there are any task mapping files indicating previous downloads
    auto task_map = load_task_map();
    if (!task_map.empty()) {
        SetStatusText("Previous download tasks found. Click Start Download to resume.");
    } else {
        SetStatusText("Ready to download");
    }
}