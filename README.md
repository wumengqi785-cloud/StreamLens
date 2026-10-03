# StreamLens

A lightweight C++ video stream relay and diagnostics toolkit.

## Project layout

```text
StreamLens/
├── CMakeLists.txt
├── include/streamlens/   # Public headers
├── src/                  # Application source files
├── tests/                # Tests
├── docs/                 # Design and protocol notes
└── cmake/                # CMake helper modules
```

## Build on WSL

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
./build/streamlens
```

Expected output:

```text
StreamLens 0.1.0
Video stream relay and diagnostics toolkit
```
