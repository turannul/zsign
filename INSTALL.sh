#!/bin/bash

build() {
    if [ -d build ]; then
        echo "Removing old build directory..."
        rm -rf build/
    fi

    echo "Creating build directory"
    mkdir -p build
    echo "Building..."
    cd build && cmake .. && make
}

if [[ "$(uname)" == "Darwin" ]]; then
    echo "Installing macOS dependencies..."
    brew install zip unzip openssl@1.1 -q || { echo "Failed to install macOS dependencies"; exit 1; }
    build
else
    echo "Updating and installing Linux dependencies..."
    sudo apt-get update && sudo apt-get install -y zip unzip build-essential checkinstall zlib1g-dev libssl-dev pkg-config || { echo "Failed to install Linux dependencies"; exit 1; }
    build
fi
