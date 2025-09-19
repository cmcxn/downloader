# XL Downloader SDK

This repository contains the XL Downloader SDK for integrating Thunder download capabilities into your applications.

## 🚀 Quick Start

### Windows (Full Support)
1. Download the latest release from the [Releases page](../../releases)
2. Extract the Windows package (`xl-downloader-sdk-windows.zip`)
3. Use the pre-built libraries or build from source

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
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release
```

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
- Supports Windows builds only

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
- Tests build process on Windows
- Ensures SDK integrity

### Troubleshooting Workflows

**Release Creation Issues:**
- Ensure the workflow has `permissions: contents: write` for release creation
- Default `GITHUB_TOKEN` requires explicit permissions for repository modifications
- Manual workflow dispatch requires proper version input format (e.g., `v1.0.0`)

**Build Issues:**
- Windows builds require Visual Studio 2022 and CMake 3.21+
- Verify all SDK files are present before building
- Check that `dk.dll` and `dk.lib` are accessible during compilation

## 🏷️ Release Assets

Each release includes:
- Windows package with complete SDK and pre-built demo
- Complete SDK package with all files
- Both tar.gz and zip formats for compatibility

## ⚠️ Platform Notes

This SDK is designed for Windows environments only. The binary libraries (dk.dll, dk.lib) are Windows-specific.