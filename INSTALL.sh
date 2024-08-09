#!/bin/bash

set -e

make_dir() {
    if [ -d build ]; then
        echo "Removing old build directory..."
        rm -rf build/
    fi

    echo "Creating build directory"
    mkdir -p build
}

compile() {
    make_dir
    echo "Compiling..."
    cd build && cmake .. && make
    if [ -f zsign ]; then
        sudo mv zsign /usr/local/bin/zsign
        echo "zsign installed to /usr/local/bin/"
    else
        echo "Build failed: zsign not found"
        exit 1
    fi
    rm -rf ../build
}

chk_brew_pkg() {
    if brew list --formula | grep -q "$1"; then
        echo "$1 is already installed"
    else
        echo "$1 is not installed, installing..."
        brew install "$1" -q
    fi
}

chk_apt_pkg() {
    if dpkg -l | grep -q "$1"; then
        echo "$1 is already installed"
    else
        echo "$1 is not installed, installing..."
        sudo apt-get update
        sudo apt-get install -y "$1"
    fi
}

if [[ "$(uname)" == "Darwin" ]]; then
    echo "Installing macOS dependencies..."
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found, please install it first."
        exit 1
    fi

    chk_brew_pkg zip
    chk_brew_pkg unzip
    chk_brew_pkg openssl@3
    chk_brew_pkg cmake

    compile

else
    echo "Updating and installing Linux dependencies..."
    chk_apt_pkg zip
    chk_apt_pkg unzip
    chk_apt_pkg build-essential
    chk_apt_pkg checkinstall
    chk_apt_pkg zlib1g-dev
    chk_apt_pkg libssl-dev
    chk_apt_pkg cmake
    compile
fi
