# OpenDCC Build Makefile
# Cross-platform build automation using our Python scripts

PYTHON := python3
BUILD_TYPE := Release
JOBS := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

.PHONY: help setup build clean install test package dev debug release all

# Default target
all: setup build

help:
	@echo "OpenDCC Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  help        - Show this help message"
	@echo "  setup       - Install dependencies"
	@echo "  build       - Build the project (Release mode)"
	@echo "  debug       - Build in Debug mode"
	@echo "  release     - Build in Release mode"
	@echo "  clean       - Clean build directory"
	@echo "  install     - Install after building"
	@echo "  test        - Build and run tests"
	@echo "  package     - Create distribution package"
	@echo "  dev         - Development setup (dependencies + debug build + tests)"
	@echo "  all         - Setup dependencies and build"
	@echo ""
	@echo "Environment variables:"
	@echo "  BUILD_TYPE  - Build configuration (default: $(BUILD_TYPE))"
	@echo "  JOBS        - Parallel build jobs (default: $(JOBS))"
	@echo ""
	@echo "Examples:"
	@echo "  make setup         # Install dependencies"
	@echo "  make build         # Quick build"
	@echo "  make dev           # Full development setup"
	@echo "  BUILD_TYPE=Debug make build  # Debug build"

setup:
	@echo "🔧 Installing dependencies..."
	$(PYTHON) setup_dependencies.py full

build:
	@echo "🏗️  Building OpenDCC ($(BUILD_TYPE))..."
	$(PYTHON) build.py --config $(BUILD_TYPE) --jobs $(JOBS)

debug:
	@echo "🐛 Building in Debug mode..."
	$(PYTHON) build.py --config Debug --jobs $(JOBS)

release:
	@echo "🚀 Building in Release mode..."
	$(PYTHON) build.py --config Release --jobs $(JOBS)

clean:
	@echo "🧹 Cleaning build directory..."
	$(PYTHON) build.py --clean --config $(BUILD_TYPE)

install: build
	@echo "📦 Installing OpenDCC..."
	$(PYTHON) build.py --config $(BUILD_TYPE) --install

test: 
	@echo "🧪 Building and running tests..."
	$(PYTHON) build.py --config $(BUILD_TYPE) --tests --jobs $(JOBS)
	@cd build && ctest --output-on-failure

package: build
	@echo "📦 Creating distribution package..."
	@cd build && cpack

dev: setup debug test
	@echo "✅ Development environment ready!"

# Convenience aliases
deps: setup
configure: setup
compile: build