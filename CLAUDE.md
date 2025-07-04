# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

WCCommon is a C++20 header-only library providing utilities for high-frequency trading (HFT) development. The project uses CMake as its build system and is designed to be installed as a system library.

## Build System

### Main Build Commands
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j8
```

### Installation
```bash
cmake --install . --prefix ~/opt/WCCommon
export WCCommon_DIR=~/opt/WCCommon/lib/cmake/WCCommon
```

### Testing
```bash
cd build
ctest
```

### Individual Test Execution
Tests can be run individually from the build directory:
```bash
./tests/NumericTimeTest
./tests/CsvIOTest.generic
./tests/CsvIOTest.specialized
```

## Architecture

### Core Components

- **Header-only Library**: All implementations are in `/include/` directory
- **Two Main Targets**: 
  - `WCCommon::WCCommon` - Core utilities (always available)
  - `LogConfig::LogConfig` - Logging configuration (requires Boost)

### Key Utilities

- **CsvIO**: Fast CSV file I/O with progress bar
- **H5IO**: HDF5 file operations wrapper
- **NumericTime**: Trading time representation and arithmetic
- **LogConfig**: spdlog configuration via YAML
- **HJLogFormat**: Structured logging with macros
- **DefEnum/DefTuple**: Reflection macros for enums and tuples
- **YAMLGetField**: YAML configuration parsing with `${today}` substitution

### Dependencies

Required:
- C++20 compiler (g++-13 or newer recommended)
- CMake 3.12+
- fmt, yaml-cpp, spdlog, date libraries

Optional:
- Boost (required for LogConfig and reflection macros)
- HDF5 (required for H5IO)
- Catch2 (required for tests)

### Build Configuration

The build system automatically detects available dependencies:
- Tests are conditionally compiled based on dependency availability
- LogConfig target only exists when Boost is found
- H5IO functionality only available when HDF5 is found

### Testing Framework

Uses Catch2 for unit testing. Each major component has its own test file in `/tests/` directory. Tests are configured to run with `ctest` and can also be executed individually.

## Development Notes

- All code is header-only and located in `/include/`
- Uses modern C++20 features extensively
- Designed for HFT applications with performance-critical components
- YAML configuration files support `${today}` placeholder replacement
- Logging system supports both simple and structured (HJ format) logging