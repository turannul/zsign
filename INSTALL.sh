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
elif command -v brew > /dev/null; then
    echo "Homebrew found at: $(brew --prefix)"
    echo "Using Linux Brew to install dependencies..."
    brew update
    brew install cmake zip unzip make gcc zlib openssl@1.1 pkg-config || { echo "Failed to install Linux Brew dependencies"; exit 1; }
    sudo apt-get install -y checkinstall
    # export LDFLAGS="-L/usr/local/opt/openssl@1.1/lib"
    # export CPPFLAGS="-I/usr/local/opt/openssl@1.1/include"
    build
elif command -v apt-get > /dev/null; then
    echo "Updating and installing Linux (with apt) dependencies..."
    sudo apt-get update
    sudo apt-get install -y zip unzip build-essential checkinstall zlib1g-dev libssl-dev pkg-config || { echo "Failed to install Linux dependencies"; exit 1; }
    build
else
    echo "Unsupported."
    exit 1
fi
