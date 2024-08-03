#!/bin/bash

create_Dir() {

    if [ -d build ]; then
        echo "Removing old build directory..."
        rm -rf build/
    fi

    echo "Creating build directory"
    mkdir -p build
}
compile() {
    create_Dir
    echo "Compiling..."
    cd build && cmake .. && make
}

if [[ "$(uname)" == "Darwin" ]]; then
    echo "Installing macOS dependencies..."
    brew install zip unzip openssl@1.1 -q || { echo "Failed to install macOS dependencies"; exit 1; }
    build
elif command -v brew > /dev/null; then
    echo "Updating and installing Linux (with apt) dependencies..."
    sudo apt-get update
    sudo apt-get install -y zip unzip build-essential checkinstall zlib1g-dev libssl-dev pkg-config || { echo "Failed to install Linux dependencies"; exit 1; }
    brew update
    brew install openssl@1.1 || { echo "Failed to install OpenSSL@1.1"; exit 2; }
    ssl_lib="-L$(brew --prefix openssl@1.1)/*/lib"
    ssl_incl="-I$(brew --prefix openssl@1.1)/*/include"
    create_Dir
    g++ src/zsign/*.cpp -lcrypto "${ssl_incl}" "${ssl_lib}" -O3 -o build/zsign || { echo "Build failed"; exit 2; }
else
    echo "Homebrew is required for openssl@1.1."
    exit 4
fi
