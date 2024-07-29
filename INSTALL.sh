#!/bin/bash

# Detect OS if it is macOS
if [[ "$(uname)" == "Darwin" ]]; then

    # Dependencies for macOS
    brew install zip unzip openssl@1.1 -q

    if [ -f build ]; then
        # Remove old build directory
        rm -rf build/
    else
        # Create build folder
        mkdir -p build
        # Compile
        cd build && cmake .. && make
    fi
else
    # Dependencies for Linux
    sudo apt-get update
    sudo apt-get install zip unzip build-essential checkinstall zlib1g-dev libssl-dev pkg-config -y

    if [ -f build ]; then
        # Remove old build directory
        rm -rf build/
    else
        # Create build folder
        mkdir -p build
        # Compile
        cd build && cmake .. && make
    fi
fi