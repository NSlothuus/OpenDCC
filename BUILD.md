# OpenDCC Build Guide

This guide provides comprehensive instructions for building OpenDCC across all supported platforms (Windows, Linux, macOS) with the new unified build system.

## 🚀 Quick Start

The fastest way to build OpenDCC is using our new Python build script:

```bash
# Install dependencies automatically
python setup_dependencies.py

# Build the project
python build.py
```

That's it! The build system will auto-detect your platform and configure everything appropriately.

## 📋 Table of Contents

- [Prerequisites](#prerequisites)
- [Platform-Specific Setup](#platform-specific-setup)
- [Build Configuration](#build-configuration)
- [Advanced Usage](#advanced-usage)
- [Troubleshooting](#troubleshooting)
- [CI/CD Integration](#ci-cd-integration)
- [Best Practices](#best-practices)

## Prerequisites

### System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **CMake** | 3.18 | 3.25+ |
| **Python** | 3.9 | 3.11+ |
| **Memory** | 8 GB RAM | 16 GB+ RAM |
| **Storage** | 10 GB free | 20 GB+ free |

### Core Dependencies

- **OpenUSD 23.11** (upgraded from 22.05)
- **Qt 5.15.2**
- **TBB 2021.7**
- **Boost 1.82.0**
- **OpenImageIO 2.4.17**
- **OpenColorIO 2.2.1**
- **Embree 4.3.0**
- **GLEW 2.2.0**
- **Eigen 3.4.0**

See [dependencies.yaml](dependencies.yaml) for complete version matrix.

## Platform-Specific Setup

### 🐧 Linux (Ubuntu/Debian)

```bash
# System dependencies
sudo apt update
sudo apt install -y cmake build-essential pkg-config python3-dev \
    qtbase5-dev qttools5-dev qtmultimedia5-dev libqt5svg5-dev \
    libqt5opengl5-dev libtbb-dev libglew-dev libeigen3-dev libembree-dev

# Auto-install remaining dependencies
python setup_dependencies.py full

# Build
python build.py --config Release --jobs $(nproc)
```

### 🍎 macOS

```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Auto-install dependencies
python setup_dependencies.py full

# Build
python build.py --config Release --jobs $(sysctl -n hw.ncpu)
```

**macOS Notes:**
- Minimum macOS version: 10.15 (Catalina)
- Apple Silicon (M1/M2) supported
- Xcode Command Line Tools required

### 🪟 Windows

```batch
REM Install vcpkg (one-time setup)
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
setx VCPKG_ROOT "C:\vcpkg"

REM Auto-install dependencies
python setup_dependencies.py full

REM Build
python build.py --config Release --jobs 4
```

**Windows Notes:**
- Visual Studio 2022 Build Tools required
- vcpkg for dependency management
- PowerShell or Command Prompt supported

## Build Configuration

### Basic Usage

```bash
# Default build (Release mode)
python build.py

# Specific configuration
python build.py --config Debug
python build.py --config RelWithDebInfo

# Clean build
python build.py --clean

# Parallel build with specific job count
python build.py --jobs 8

# Verbose output
python build.py --verbose

# Install after building
python build.py --install
```

### Advanced Options

```bash
# Custom dependency paths
python build.py --usd-root /path/to/usd --qt-root /path/to/qt

# Specific CMake generator
python build.py --generator "Unix Makefiles"
python build.py --generator "Ninja"

# Enable tests
python build.py --tests

# Development build with all features
python build.py --config Debug --tests --verbose --install
```

### Environment Setup

After running dependency setup, source the environment script:

```bash
# Linux/macOS
source setup_env.sh

# Windows
setup_env.bat
```

## Advanced Usage

### Dependency Profiles

Choose different dependency sets based on your needs:

```bash
# Minimal build - core USD support only
python setup_dependencies.py minimal

# Full build - all rendering backends
python setup_dependencies.py full

# Developer build - includes tests and debug tools
python setup_dependencies.py developer
```

### Manual CMake Build

If you prefer traditional CMake workflow:

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release \
      -DUSD_ROOT=/path/to/usd \
      -DQt5_DIR=/path/to/qt/lib/cmake/Qt5

# Build
cmake --build build --config Release --parallel 4

# Install
cmake --install build --config Release
```

### Dependency Version Management

Update dependency versions in [dependencies.yaml](dependencies.yaml):

```yaml
usd:
  version: "24.03"  # Upgrade USD version
  
qt:
  version: "6.2.0"  # Upgrade to Qt6
```

Then regenerate the build configuration:

```bash
python setup_dependencies.py full
python build.py --clean
```

## Troubleshooting

### Common Issues

#### USD Not Found
```bash
# Manually specify USD path
python build.py --usd-root /usr/local/USD

# Or set environment variable
export USD_ROOT=/usr/local/USD
```

#### Qt Not Found (macOS)
```bash
# Link Qt5 explicitly
brew link qt@5 --force

# Or specify Qt path
python build.py --qt-root $(brew --prefix qt@5)
```

#### Out of Memory During Build
```bash
# Reduce parallel jobs
python build.py --jobs 2

# Use less memory-intensive build type
python build.py --config Release  # instead of Debug
```

#### Windows vcpkg Issues
```batch
REM Update vcpkg
cd C:\vcpkg
git pull
.\bootstrap-vcpkg.bat

REM Reinstall packages
vcpkg remove --outdated
python setup_dependencies.py full
```

### Platform-Specific Issues

#### Linux: Missing System Libraries
```bash
# Install additional development packages
sudo apt install -y libgl1-mesa-dev libx11-dev libxext-dev \
    libxfixes-dev libxi-dev libxrender-dev libxcb1-dev
```

#### macOS: Apple Silicon Compatibility
```bash
# For x86_64 compatibility
arch -x86_64 python build.py

# Check architecture
file build/src/bin/render_view/render_view
```

#### Windows: MSVC Version Issues
```batch
REM Ensure Visual Studio 2022 Build Tools are installed
REM Download from: https://visualstudio.microsoft.com/downloads/
```

## CI/CD Integration

### GitHub Actions

The project includes multi-platform CI with automatic building:

- **Linux**: Ubuntu 22.04
- **Windows**: Windows Server 2022
- **macOS**: macOS 12

Triggers:
- Release tags (`v*`)
- Manual dispatch
- Pull requests (optional)

### Custom CI Systems

```yaml
# Example GitLab CI integration
build:
  stage: build
  script:
    - python setup_dependencies.py full
    - python build.py --config Release --jobs 4
  artifacts:
    paths:
      - build/
    expire_in: 1 week
```

## Best Practices

### 🏗️ Build Management

1. **Use Virtual Environments**
   ```bash
   python -m venv venv
   source venv/bin/activate  # Linux/macOS
   # or
   venv\Scripts\activate     # Windows
   ```

2. **Dependency Caching**
   - Cache USD builds (30+ minutes compile time)
   - Use ccache/sccache for C++ compilation
   - Store vcpkg/Homebrew packages in CI cache

3. **Build Types**
   - Use `Release` for production
   - Use `RelWithDebInfo` for debugging with optimization
   - Use `Debug` only for development
   - Use `Hybrid` for mixed optimization levels

### 🔧 Development Workflow

1. **Incremental Builds**
   ```bash
   # Only rebuild what changed
   python build.py  # (without --clean)
   ```

2. **Testing Integration**
   ```bash
   python build.py --tests
   cd build && ctest --output-on-failure
   ```

3. **Parallel Development**
   ```bash
   # Build specific targets only
   cmake --build build --target render_view
   ```

### 🎯 Performance Optimization

1. **Compiler Optimization**
   - Use `-march=native` for CPU-specific optimizations
   - Enable LTO (Link Time Optimization) for release builds
   - Use `ninja` generator instead of `make` on Linux

2. **Memory Usage**
   - Limit parallel jobs on memory-constrained systems
   - Use `gold` linker on Linux for faster linking
   - Consider using `sccache` for Windows builds

3. **Build Time Reduction**
   ```bash
   # Use precompiled headers
   export DCC_USE_PCH=ON
   
   # Use ccache (Linux/macOS)
   export CC="ccache gcc"
   export CXX="ccache g++"
   ```

### 🔒 Security Considerations

1. **Dependency Verification**
   - Pin dependency versions in `dependencies.yaml`
   - Verify checksums for downloaded packages
   - Use official package repositories when possible

2. **Build Environment**
   - Use clean build environments for releases
   - Avoid mixing debug and release libraries
   - Validate all external dependencies

### 📦 Distribution

1. **Packaging**
   ```bash
   # Create distributable package
   python build.py --install
   cpack --config build/CPackConfig.cmake
   ```

2. **Cross-Platform Compatibility**
   - Test on all target platforms
   - Include all required runtime dependencies
   - Provide platform-specific installation instructions

## Contributing

When contributing build system changes:

1. Update `dependencies.yaml` for version changes
2. Test on all three platforms (Windows, Linux, macOS)  
3. Update this documentation for new features
4. Ensure CI/CD passes on all platforms

## Support

For build-related issues:

1. Check this documentation first
2. Review [troubleshooting](#troubleshooting) section
3. Search existing GitHub issues
4. Open a new issue with:
   - Platform and version
   - Complete error output
   - Steps to reproduce

---

*Built with ❤️ for the OpenDCC community*