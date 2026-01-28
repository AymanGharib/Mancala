# 🎮 Mancala 3D

A 3D implementation of the classic Mancala board game built with OpenGL and C++.

## 📋 Table of Contents

- [Requirements](#-requirements)
- [Quick Start](#-quick-start)
- [Build Instructions](#-build-instructions)
  - [Windows](#windows)
  - [Linux](#linux)
  - [macOS](#macos)
- [Troubleshooting](#-troubleshooting)
- [Project Structure](#-project-structure)
- [License](#-license)

## 🛠️ Requirements

Before building the project, ensure you have the following installed:

- **CMake** (version 3.10 or higher)
- **C++ compiler** with C++11 support or higher
  - Windows: MinGW-w64 or MSVC
  - Linux: GCC or Clang
  - macOS: Clang (Xcode Command Line Tools)
- **OpenGL-compatible GPU** with driver support
- **Git** (for cloning the repository)

## ⚡ Quick Start

```bash
# Clone the repository (if not already done)
git clone <repository-url>
cd Mancala3D

# Build and run (Windows)
mkdir build && cmake -S . -B build && cmake --build build && .\build\bin\Mancala3D.exe

# Build and run (Linux/macOS)
mkdir build && cmake -S . -B build && cmake --build build && ./build/bin/Mancala3D
```

## 🚀 Build Instructions

### Windows

1. **Create a build directory**
   ```bash
   mkdir build
   ```

2. **Generate build files with CMake**
   ```bash
   cmake -S . -B build
   ```
   
   *Optional: Specify a generator*
   ```bash
   cmake -S . -B build -G "MinGW Makefiles"
   # or
   cmake -S . -B build -G "Visual Studio 17 2022"
   ```

3. **Build the project**
   ```bash
   cmake --build build
   ```
   
   *Optional: Build in Release mode*
   ```bash
   cmake --build build --config Release
   ```

4. **Run the application**
   ```bash
   .\build\bin\Mancala3D.exe
   ```

### Linux

1. **Install dependencies**
   ```bash
   # Ubuntu/Debian
   sudo apt-get update
   sudo apt-get install build-essential cmake libgl1-mesa-dev libglu1-mesa-dev
   
   # Fedora
   sudo dnf install gcc-c++ cmake mesa-libGL-devel mesa-libGLU-devel
   
   # Arch Linux
   sudo pacman -S base-devel cmake mesa
   ```

2. **Build the project**
   ```bash
   mkdir build
   cmake -S . -B build
   cmake --build build
   ```

3. **Run the application**
   ```bash
   ./build/bin/Mancala3D
   ```

### macOS

1. **Install Xcode Command Line Tools** (if not already installed)
   ```bash
   xcode-select --install
   ```

2. **Install CMake** (via Homebrew)
   ```bash
   brew install cmake
   ```

3. **Build the project**
   ```bash
   mkdir build
   cmake -S . -B build
   cmake --build build
   ```

4. **Run the application**
   ```bash
   ./build/bin/Mancala3D
   ```
