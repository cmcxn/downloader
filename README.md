# XL Downloader SDK

This repository contains the XL Downloader SDK for integrating Thunder download capabilities into your applications.

## 🚀 Quick Start

### Windows (Full Support)
1. Download the latest release from the [Releases page](../../releases)
2. Extract the Windows package (`xl-downloader-sdk-windows.zip`)
3. Use the pre-built libraries or build from source

### Linux/macOS (Source Only)
1. Download the source package from the [Releases page](../../releases)
2. Build from source using the demo project as reference

## 📦 What's Included

- **xl_dl_sdk.h** - C++ SDK header file
- **api.txt** - Complete API documentation (Chinese)
- **dk.dll** & **dk.lib** - Windows binary libraries
- **cpp_xl_dl_demo/** - Complete demo project with examples

## 🏗️ Building

### Windows
```bash
cd cpp_xl_dl_demo
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Linux/macOS
Note: Binary libraries are not available for these platforms. You can:
- Use the header files for reference
- Adapt the demo code for your platform
- Contact the SDK provider for platform-specific binaries

## 📚 Documentation

Refer to `api.txt` for complete API documentation and usage examples.

## 🔄 Automated Releases

This repository uses GitHub Actions for automated builds and releases:

### Creating a Release
1. **Tag-based Release**: Push a git tag starting with 'v' (e.g., `v1.0.0`)
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

2. **Manual Release**: Use the "Actions" tab and trigger the "Build and Release" workflow manually

### Continuous Integration
- Automatically builds and tests on push/PR to main branches
- Validates SDK files and build process
- Currently supports Windows builds only

## 📁 Repository Structure

```
.
├── xl_dl_sdk.h           # SDK header file
├── api.txt               # API documentation
├── dk.dll                # Windows library
├── dk.lib                # Windows library
├── cpp_xl_dl_demo/       # Demo project
│   ├── main.cpp          # Demo application
│   ├── CMakeLists.txt    # Build configuration
│   └── readme.txt        # Build instructions
└── .github/workflows/    # CI/CD workflows
    ├── release.yml       # Release workflow
    └── ci.yml            # Continuous integration
```

## 🛠️ Workflow Details

### Release Workflow (`release.yml`)
- Triggers on version tags (`v*`) or manual dispatch
- Builds Windows demo executable
- Packages SDK for multiple platforms
- Creates GitHub release with downloadable assets

### CI Workflow (`ci.yml`)
- Triggers on push/PR to main branches
- Validates repository structure
- Tests build process (Windows only)
- Ensures SDK integrity

## 🏷️ Release Assets

Each release includes:
- Platform-specific packages (Windows, Linux, macOS)
- Complete SDK package with all files
- Both tar.gz and zip formats for compatibility

## ⚠️ Platform Notes

This SDK is primarily designed for Windows environments. Linux and macOS packages contain headers and source code only, as binary libraries are Windows-specific.