# Thunder Downloader GUI

这是一个基于 Win32 API 的 Thunder 下载器 GUI 版本，提供了图形化界面来配置和监控下载任务。

## 功能特性

- 图形化用户界面，使用原生 Win32 API 实现
- 支持填写和编辑以下配置：
  - APP_ID：应用程序 ID
  - Token Server：令牌服务器地址
  - Download URL：下载链接
- 开始下载按钮
- 实时进度条显示下载进度
- 状态显示区域显示详细下载信息
- 最小体积设计，仅依赖系统 DLL

## 编译说明

### Windows 平台

1. 确保已安装 Visual Studio 或 MinGW 编译器
2. 在项目根目录执行：

```bash
cd cpp_xl_dl_demo
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

或者使用 MinGW：

```bash
cd cpp_xl_dl_demo
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make
```

3. 编译成功后会生成两个可执行文件：
   - `cpp_xl_dl_demo.exe` - 原始控制台版本
   - `cpp_xl_dl_demo_gui.exe` - 新的 GUI 版本

## 使用说明

1. 运行 `cpp_xl_dl_demo_gui.exe`
2. 程序启动后会显示一个包含以下元素的窗口：
   - APP ID 输入框（预填默认值）
   - Token Server 输入框（预填默认值）
   - Download URL 输入框（预填默认值）
   - "Start Download" 按钮
   - 进度条
   - 状态显示区域

3. 可以修改任意配置字段，然后点击 "Start Download" 开始下载
4. 进度条会实时显示下载进度
5. 状态区域会显示详细的下载信息，包括下载百分比、文件大小和下载速度

## 默认配置

程序预设了以下默认值：

- **APP_ID**: `eGwta3o3SzEwMTYzAAAAA7MnAAA=`
- **TOKEN_SERVER**: `http://1.94.243.230:8080`
- **DOWNLOAD_URL**: `https://down.sandai.net/thunder11/XunLeiSetup12.0.12.2510.exe`

## 技术特点

- 使用原生 Win32 API，无需额外的 UI 框架
- 多线程设计，GUI 响应不会被下载阻塞
- 复用原有的下载逻辑和 SDK 集成
- 最小化依赖，追求极小体积

## 注意事项

- 此程序仅支持 Windows 平台
- 需要 `dk.lib` 库和 `xl_dl_sdk.h` 头文件
- 确保网络连接正常，能够访问令牌服务器