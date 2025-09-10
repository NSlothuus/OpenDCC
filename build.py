#!/usr/bin/env python3
"""
OpenDCC Cross-Platform Build Script

A unified build system for OpenDCC that works across Windows, Linux, and macOS.
Automatically detects platform and configures dependencies accordingly.

Usage:
    python build.py [options]

Examples:
    python build.py                    # Basic build 
    python build.py --clean            # Clean build
    python build.py --config Release   # Specific build type
    python build.py --jobs 8           # Parallel build
    python build.py --install          # Install after build
    python build.py --verbose          # Verbose output
"""

import argparse
import os
import platform
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Optional


class OpenDCCBuilder:
    def __init__(self):
        self.system = platform.system().lower()
        self.machine = platform.machine().lower()
        self.root_dir = Path(__file__).parent.absolute()
        self.build_dir = self.root_dir / "build"
        self.install_dir = self.root_dir / "install"
        
        # Default configuration
        self.config = "Release"
        self.jobs = os.cpu_count() or 4
        self.verbose = False
        
    def parse_args(self):
        """Parse command line arguments"""
        parser = argparse.ArgumentParser(
            description="OpenDCC Cross-Platform Build Script",
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog=__doc__.split('Usage:')[1] if 'Usage:' in __doc__ else ""
        )
        
        parser.add_argument(
            "--config", "-c", 
            choices=["Debug", "Release", "RelWithDebInfo", "Hybrid"],
            default="Release",
            help="Build configuration (default: Release)"
        )
        
        parser.add_argument(
            "--clean", 
            action="store_true",
            help="Clean build directory before building"
        )
        
        parser.add_argument(
            "--jobs", "-j",
            type=int,
            default=self.jobs,
            help=f"Number of parallel jobs (default: {self.jobs})"
        )
        
        parser.add_argument(
            "--install",
            action="store_true", 
            help="Install after building"
        )
        
        parser.add_argument(
            "--verbose", "-v",
            action="store_true",
            help="Verbose output"
        )
        
        parser.add_argument(
            "--generator", "-g",
            help="CMake generator to use"
        )
        
        parser.add_argument(
            "--usd-root",
            help="Path to USD installation"
        )
        
        parser.add_argument(
            "--qt-root", 
            help="Path to Qt installation"
        )
        
        parser.add_argument(
            "--python-root",
            help="Path to Python installation"
        )
        
        parser.add_argument(
            "--tests",
            action="store_true",
            help="Build tests"
        )
        
        args = parser.parse_args()
        
        # Update instance variables
        self.config = args.config
        self.jobs = args.jobs
        self.verbose = args.verbose
        
        return args
        
    def log(self, message: str, level: str = "INFO"):
        """Log message with level"""
        if self.verbose or level == "ERROR":
            print(f"[{level}] {message}")
            
    def run_command(self, cmd: List[str], cwd: Optional[Path] = None) -> bool:
        """Run command and return success status"""
        if self.verbose:
            self.log(f"Running: {' '.join(cmd)}")
            
        try:
            result = subprocess.run(
                cmd,
                cwd=cwd or self.root_dir,
                check=True,
                capture_output=not self.verbose
            )
            return True
        except subprocess.CalledProcessError as e:
            self.log(f"Command failed: {e}", "ERROR")
            return False
            
    def detect_dependencies(self) -> Dict[str, Optional[str]]:
        """Auto-detect system dependencies"""
        deps = {
            "USD_ROOT": None,
            "Qt5_DIR": None, 
            "PYTHON_ROOT": None,
        }
        
        # Platform-specific detection logic
        if self.system == "darwin":  # macOS
            # Common Homebrew/MacPorts paths
            homebrew_prefix = subprocess.run(
                ["brew", "--prefix"], 
                capture_output=True, 
                text=True
            ).stdout.strip() if self._command_exists("brew") else "/usr/local"
            
            potential_usd_paths = [
                f"{homebrew_prefix}/lib/cmake/pxr",
                "/usr/local/lib/cmake/pxr",
                "/opt/local/lib/cmake/pxr"
            ]
            
            potential_qt_paths = [
                f"{homebrew_prefix}/lib/cmake/Qt5",
                "/usr/local/lib/cmake/Qt5",
                "/Applications/Qt/5.15/clang_64/lib/cmake/Qt5"
            ]
            
        elif self.system == "linux":
            potential_usd_paths = [
                "/usr/local/lib/cmake/pxr",
                "/usr/lib/cmake/pxr",
                os.path.expanduser("~/USD/lib/cmake/pxr")
            ]
            
            potential_qt_paths = [
                "/usr/lib/x86_64-linux-gnu/cmake/Qt5",
                "/usr/local/lib/cmake/Qt5"
            ]
            
        elif self.system == "windows":
            potential_usd_paths = [
                "C:/USD/lib/cmake/pxr",
                "C:/Program Files/USD/lib/cmake/pxr"
            ]
            
            potential_qt_paths = [
                "C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5",
                "C:/Qt/5.15/msvc2019_64/lib/cmake/Qt5"
            ]
        else:
            potential_usd_paths = []
            potential_qt_paths = []
            
        # Find first existing path
        for path in potential_usd_paths:
            if Path(path).exists():
                deps["USD_ROOT"] = str(Path(path).parent.parent.parent)
                break
                
        for path in potential_qt_paths:
            if Path(path).exists():
                deps["Qt5_DIR"] = path
                break
                
        return deps
        
    def _command_exists(self, cmd: str) -> bool:
        """Check if command exists in PATH"""
        try:
            subprocess.run([cmd, "--version"], capture_output=True, check=True)
            return True
        except (subprocess.CalledProcessError, FileNotFoundError):
            return False
            
    def get_cmake_args(self, args) -> List[str]:
        """Generate CMake configuration arguments"""
        cmake_args = [
            "cmake", "-B", str(self.build_dir),
            f"-DCMAKE_BUILD_TYPE={self.config}",
            f"-DCMAKE_INSTALL_PREFIX={self.install_dir}",
        ]
        
        # Generator
        if args.generator:
            cmake_args.extend(["-G", args.generator])
        elif self.system == "windows":
            cmake_args.extend(["-G", "Visual Studio 17 2022"])
        
        # Tests
        cmake_args.append(f"-DDCC_BUILD_TESTS={'ON' if args.tests else 'OFF'}")
        
        # Auto-detect dependencies
        deps = self.detect_dependencies()
        
        # Override with user-provided paths
        if args.usd_root:
            deps["USD_ROOT"] = args.usd_root
        if args.qt_root:
            deps["Qt5_DIR"] = f"{args.qt_root}/lib/cmake/Qt5"
        if args.python_root:
            deps["PYTHON_ROOT"] = args.python_root
            
        # Add dependency paths
        for key, value in deps.items():
            if value and Path(value).exists():
                cmake_args.append(f"-D{key}={value}")
                self.log(f"Using {key}: {value}")
            elif key == "USD_ROOT":
                self.log(f"Warning: Could not find USD installation", "WARN")
                
        # Platform-specific configuration
        if self.system == "darwin":
            # macOS specific settings
            cmake_args.extend([
                "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15",
                "-DCMAKE_CXX_STANDARD=17"
            ])
            
        elif self.system == "linux":
            # Linux specific settings
            cmake_args.extend([
                "-DCMAKE_CXX_STANDARD=17"
            ])
            
        elif self.system == "windows":
            # Windows specific settings
            cmake_args.extend([
                "-DCMAKE_CXX_STANDARD=17",
                "-A", "x64"
            ])
            
        return cmake_args
        
    def configure(self, args) -> bool:
        """Configure the build"""
        self.log("Configuring build...")
        
        if args.clean and self.build_dir.exists():
            self.log("Cleaning build directory")
            import shutil
            shutil.rmtree(self.build_dir)
            
        self.build_dir.mkdir(exist_ok=True)
        
        cmake_args = self.get_cmake_args(args)
        return self.run_command(cmake_args)
        
    def build(self) -> bool:
        """Build the project"""
        self.log("Building project...")
        
        build_args = [
            "cmake", "--build", str(self.build_dir),
            "--config", self.config,
            "--parallel", str(self.jobs)
        ]
        
        if self.verbose:
            build_args.append("--verbose")
            
        return self.run_command(build_args)
        
    def install(self) -> bool:
        """Install the project"""
        self.log("Installing project...")
        
        install_args = [
            "cmake", "--install", str(self.build_dir),
            "--config", self.config
        ]
        
        return self.run_command(install_args)
        
    def print_system_info(self):
        """Print system information"""
        self.log("System Information:")
        self.log(f"  OS: {platform.system()} {platform.release()}")
        self.log(f"  Architecture: {platform.machine()}")
        self.log(f"  Python: {sys.version}")
        self.log(f"  Build Directory: {self.build_dir}")
        self.log(f"  Install Directory: {self.install_dir}")
        
    def run(self):
        """Main build process"""
        args = self.parse_args()
        
        if self.verbose:
            self.print_system_info()
            
        # Configure
        if not self.configure(args):
            self.log("Configuration failed", "ERROR")
            return False
            
        # Build
        if not self.build():
            self.log("Build failed", "ERROR") 
            return False
            
        # Install
        if args.install:
            if not self.install():
                self.log("Installation failed", "ERROR")
                return False
                
        self.log("Build completed successfully!")
        return True


if __name__ == "__main__":
    builder = OpenDCCBuilder()
    success = builder.run()
    sys.exit(0 if success else 1)