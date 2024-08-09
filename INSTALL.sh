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

check_brew_pkg() {
    if brew list --formula | grep -q "$1"; then
        echo "$1 is already installed"
    else
        echo "$1 is not installed, installing..."
        brew install "$1"
    fi
}

check_apt_pkg() {
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

    check_brew_package zip
    check_brew_package unzip
    check_brew_package openssl@3
    check_brew_package cmake

    compile

else
    echo "Updating and installing Linux dependencies..."
    check_apt_package zip
    check_apt_package unzip
    check_apt_package build-essential
    check_apt_package checkinstall
    check_apt_package zlib1g-dev
    check_apt_package libssl-dev
    check_apt_package cmake
    compile
fi
