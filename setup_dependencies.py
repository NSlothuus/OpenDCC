#!/usr/bin/env python3
"""
OpenDCC Dependency Setup Script

Automatically installs and configures dependencies based on the current platform
and the versions specified in dependencies.yaml.

Usage:
    python setup_dependencies.py [profile]

Examples:
    python setup_dependencies.py minimal    # Minimal dependencies
    python setup_dependencies.py full       # All dependencies
    python setup_dependencies.py developer  # Development dependencies
"""

import argparse
import os
import platform
import subprocess
import sys
import yaml
from pathlib import Path
from typing import Dict, List, Optional


class DependencyManager:
    def __init__(self):
        self.system = platform.system().lower()
        self.root_dir = Path(__file__).parent.absolute()
        self.deps_config = self._load_dependencies_config()
        
    def _load_dependencies_config(self) -> Dict:
        """Load dependencies configuration from YAML"""
        config_path = self.root_dir / "dependencies.yaml"
        if not config_path.exists():
            raise FileNotFoundError(f"Dependencies config not found: {config_path}")
            
        with open(config_path, 'r') as f:
            return yaml.safe_load(f)
            
    def log(self, message: str, level: str = "INFO"):
        """Log message with level"""
        print(f"[{level}] {message}")
        
    def run_command(self, cmd: List[str], check: bool = True) -> bool:
        """Run command and return success status"""
        self.log(f"Running: {' '.join(cmd)}")
        try:
            result = subprocess.run(cmd, check=check, capture_output=True, text=True)
            if result.stdout.strip():
                self.log(f"Output: {result.stdout.strip()}")
            return True
        except subprocess.CalledProcessError as e:
            self.log(f"Command failed: {e}", "ERROR")
            if e.stderr:
                self.log(f"Error output: {e.stderr}", "ERROR")
            return False
            
    def command_exists(self, cmd: str) -> bool:
        """Check if command exists in PATH"""
        try:
            subprocess.run([cmd, "--version"], capture_output=True, check=True)
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            return False
            
    def install_linux_dependencies(self, profile: str = "minimal"):
        """Install dependencies on Linux using apt"""
        self.log("Installing Linux dependencies...")
        
        # System packages
        system_pkgs = self.deps_config.get("build_requirements", {}).get("linux", {}).get("system_packages", [])
        if system_pkgs:
            cmd = ["sudo", "apt-get", "update"]
            self.run_command(cmd)
            
            cmd = ["sudo", "apt-get", "install", "-y"] + system_pkgs
            self.run_command(cmd)
            
        # Core dependencies
        deps_to_install = []
        
        for dep_name, dep_config in self.deps_config.items():
            if isinstance(dep_config, dict) and "platforms" in dep_config:
                linux_config = dep_config["platforms"].get("linux", {})
                if "apt" in linux_config:
                    deps_to_install.extend(linux_config["apt"].split())
                    
        if deps_to_install:
            cmd = ["sudo", "apt-get", "install", "-y"] + deps_to_install
            self.run_command(cmd)
            
    def install_macos_dependencies(self, profile: str = "minimal"):
        """Install dependencies on macOS using Homebrew"""
        self.log("Installing macOS dependencies...")
        
        # Check if Homebrew is installed
        if not self.command_exists("brew"):
            self.log("Homebrew not found. Please install Homebrew first:", "ERROR")
            self.log("https://brew.sh/", "INFO")
            return False
            
        # Update Homebrew
        self.run_command(["brew", "update"])
        
        # Core dependencies
        deps_to_install = []
        
        for dep_name, dep_config in self.deps_config.items():
            if isinstance(dep_config, dict) and "platforms" in dep_config:
                macos_config = dep_config["platforms"].get("macos", {})
                if "homebrew" in macos_config:
                    deps_to_install.append(macos_config["homebrew"])
                    
        if deps_to_install:
            cmd = ["brew", "install"] + deps_to_install
            self.run_command(cmd)
            
        # Special handling for Qt5
        self.run_command(["brew", "link", "qt@5", "--force"])
        
        return True
        
    def install_windows_dependencies(self, profile: str = "minimal"):
        """Install dependencies on Windows using vcpkg"""
        self.log("Installing Windows dependencies...")
        
        # Check if vcpkg is available
        vcpkg_root = os.environ.get("VCPKG_ROOT")
        if not vcpkg_root or not Path(vcpkg_root).exists():
            self.log("vcpkg not found. Skipping vcpkg dependencies.", "WARN")
            self.log("For full dependency support, install vcpkg:", "INFO")
            self.log("  git clone https://github.com/Microsoft/vcpkg.git C:\\vcpkg", "INFO")
            self.log("  cd C:\\vcpkg && .\\bootstrap-vcpkg.bat", "INFO")
            self.log("  setx VCPKG_ROOT C:\\vcpkg", "INFO")
            self.log("You can still build with manual dependency paths.", "INFO")
            return True  # Don't fail, just skip vcpkg deps
            
        vcpkg_exe = Path(vcpkg_root) / "vcpkg.exe"
        if not vcpkg_exe.exists():
            self.log(f"vcpkg.exe not found at {vcpkg_exe}", "WARN")
            return True  # Don't fail, just skip
            
        # Core dependencies
        deps_to_install = []
        
        for dep_name, dep_config in self.deps_config.items():
            if isinstance(dep_config, dict) and "platforms" in dep_config:
                windows_config = dep_config["platforms"].get("windows", {})
                if "vcpkg" in windows_config:
                    deps_to_install.extend(windows_config["vcpkg"].split())
                    
        if deps_to_install:
            cmd = [str(vcpkg_exe), "install", "--triplet", "x64-windows"] + deps_to_install
            self.run_command(cmd)
            
        return True
        
    def check_dependency_versions(self):
        """Check installed dependency versions against requirements"""
        self.log("Checking dependency versions...")
        
        # Check CMake
        cmake_config = self.deps_config.get("cmake", {})
        min_version = cmake_config.get("min_version", "3.18")
        
        try:
            result = subprocess.run(["cmake", "--version"], capture_output=True, text=True)
            if result.returncode == 0:
                version_line = result.stdout.split('\n')[0]
                # Extract version number
                import re
                version_match = re.search(r'(\d+\.\d+\.\d+)', version_line)
                if version_match:
                    version = version_match.group(1)
                    self.log(f"CMake version: {version} (required: {min_version})")
                    
        except FileNotFoundError:
            self.log("CMake not found", "ERROR")
            
        # Check Python
        python_config = self.deps_config.get("python", {})
        min_version = python_config.get("min_version", "3.9")
        current_version = f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"
        self.log(f"Python version: {current_version} (required: {min_version})")
        
    def setup_environment(self, profile: str = "minimal"):
        """Setup environment variables for the build"""
        self.log("Setting up environment...")
        
        # Create environment setup script
        if self.system == "windows":
            script_path = self.root_dir / "setup_env.bat"
            with open(script_path, 'w') as f:
                f.write("@echo off\n")
                f.write("echo Setting up OpenDCC build environment...\n")
                
                # Set vcpkg toolchain if available
                vcpkg_root = os.environ.get("VCPKG_ROOT")
                if vcpkg_root:
                    f.write(f"set CMAKE_TOOLCHAIN_FILE={vcpkg_root}\\scripts\\buildsystems\\vcpkg.cmake\n")
                
                f.write("echo Environment setup complete.\n")
                
        else:  # Linux/macOS
            script_path = self.root_dir / "setup_env.sh"
            with open(script_path, 'w') as f:
                f.write("#!/bin/bash\n")
                f.write("echo Setting up OpenDCC build environment...\n")
                
                # macOS-specific Qt paths
                if self.system == "darwin":
                    f.write("export Qt5_DIR=$(brew --prefix qt@5)/lib/cmake/Qt5\n")
                    f.write("export PATH=$(brew --prefix qt@5)/bin:$PATH\n")
                
                f.write("echo Environment setup complete.\n")
                
            # Make script executable
            os.chmod(script_path, 0o755)
            
        self.log(f"Environment setup script created: {script_path}")
        
    def setup_usd_from_source(self):
        """Build USD from source if needed"""
        self.log("Checking USD installation...")
        
        usd_config = self.deps_config.get("usd", {})
        version = usd_config.get("version", "23.11")
        
        # Check if USD is already installed
        try:
            result = subprocess.run(["python", "-c", "import pxr; print(pxr.__version__)"], 
                                  capture_output=True, text=True)
            if result.returncode == 0:
                installed_version = result.stdout.strip()
                self.log(f"USD already installed: {installed_version}")
                return True
        except:
            pass
            
        self.log(f"USD not found. Consider installing USD {version} manually:")
        
        if self.system == "linux":
            self.log("  - Use package manager: sudo apt install libusd-dev")
            self.log("  - Or build from source: https://github.com/PixarAnimationStudios/USD")
        elif self.system == "darwin":
            self.log("  - Use Homebrew: brew install pixar/usd/usd")
            self.log("  - Or build from source: https://github.com/PixarAnimationStudios/USD")
        elif self.system == "windows":
            self.log("  - Use vcpkg: vcpkg install usd")
            self.log("  - Or download prebuilt binaries from Pixar")
            
        return False
        
    def run(self, profile: str = "minimal"):
        """Main setup process"""
        self.log(f"Setting up dependencies for profile: {profile}")
        self.log(f"Platform: {platform.system()} {platform.machine()}")
        
        # Install platform-specific dependencies
        success = True
        if self.system == "linux":
            success = self.install_linux_dependencies(profile)
        elif self.system == "darwin":
            success = self.install_macos_dependencies(profile)
        elif self.system == "windows":
            success = self.install_windows_dependencies(profile)
        else:
            self.log(f"Unsupported platform: {self.system}", "ERROR")
            return False
            
        if not success:
            self.log("Dependency installation failed", "ERROR")
            return False
            
        # Setup USD (may require manual intervention)
        self.setup_usd_from_source()
        
        # Check versions
        self.check_dependency_versions()
        
        # Setup environment
        self.setup_environment(profile)
        
        self.log("Dependency setup completed!")
        self.log("You can now run: python build.py")
        
        return True


def main():
    parser = argparse.ArgumentParser(
        description="Setup OpenDCC build dependencies",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    
    parser.add_argument(
        "profile",
        nargs="?",
        default="minimal",
        choices=["minimal", "full", "developer"],
        help="Dependency profile to install (default: minimal)"
    )
    
    args = parser.parse_args()
    
    try:
        manager = DependencyManager()
        success = manager.run(args.profile)
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"[ERROR] Setup failed: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()