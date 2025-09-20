Build:

Prerequisites:
- libtorrent-rasterbar library (install via vcpkg or system package manager)

For Windows with vcpkg:
vcpkg install libtorrent

Build:
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build .

For Linux/macOS:
Install libtorrent-rasterbar via your package manager, then:
mkdir build
cd build
cmake ..
cmake --build .
