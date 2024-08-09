#!/bin/bash

set -e

compile() {
    if [ -d build ]; then
        echo "Removing old build directory..."
        rm -rf build/
    fi
    mkdir -p build

    echo "Starting build process..."
    cd build && cmake .. && make
    if [ -f zsign ]; then
        echo "Build completed successfully"
        mv zsign ../Test/zsign
        test_zsign
    else
        echo "Build failed. Please check the build output for errors."
        exit 1
    fi
}

test_zsign() {
    cd ../
    echo "Running tests..."
    if ctest --rerun-failed --output-on-failure; then
        echo "All tests passed successfully."
        echo "Installing binary to /usr/local/bin/"
        mv -v Test/zsign /usr/local/bin/zsign
    else
        echo "One or more tests failed. Check test output for details."
        exit 1
    fi

    echo "Cleaning up build directory..."
    rm -rf build
}

chk_brew_pkg() {
    if brew list --formula | grep -q "$1"; then
        echo "$1 is already installed."
    else
        echo "$1 is not installed. Installing now..."
        brew install "$1" -q
    fi
}

chk_apt_pkg() {
    if dpkg -l | grep -q "$1"; then
        echo "$1 is already installed."
    else
        echo "$1 is not installed. Installing now..."
        sudo apt-get update -q
        sudo apt-get install -y "$1"
    fi
}

if [[ "$(uname)" == "Darwin" ]]; then
    echo "Setting up macOS environment..."
    if ! command -v brew &> /dev/null; then
        echo "Homebrew is not installed. Please install it before proceeding."
        exit 1
    fi

    echo "Checking macOS dependencies..."
    chk_brew_pkg zip
    chk_brew_pkg unzip
    chk_brew_pkg openssl@3
    chk_brew_pkg cmake

    compile

else
    echo "Updating Linux environment..."
    echo "Checking Linux dependencies..."
    chk_apt_pkg zip
    chk_apt_pkg unzip
    chk_apt_pkg build-essential
    chk_apt_pkg checkinstall
    chk_apt_pkg zlib1g-dev
    chk_apt_pkg libssl-dev
    chk_apt_pkg cmake
    compile
fi
