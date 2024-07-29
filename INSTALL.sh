#!/bin/bash

# Detect OS if it is macOS
if [[ "$(uname)" == "Darwin" ]]; then

    # Install dependencies
    brew install zip unzip openssl@1.1

    # Get the installation prefix for OpenSSL 1.1
    OPENSSL_PREFIX=$(brew --prefix openssl@1.1)
    # Define the include and library paths
    INCLUDE_PATH="$OPENSSL_PREFIX/include"
    LIB_PATH="$OPENSSL_PREFIX/lib"

    echo "Include path: $INCLUDE_PATH"
    echo "Library path: $LIB_PATH"

    if [ -f build ]; then
    # Remove old build directory
        rm -rf build/
    else
    # Create build folder
        mkdir -p build
    fi

    # Compile
    clang++ src/zsign/*.cpp -I"$INCLUDE_PATH" -L"$LIB_PATH" -lssl -lcrypto -O3 -o build/zsign

else

    # Dependencies for Linux
    sudo apt-get update
    sudo apt-get install wget zip unzip build-essential checkinstall zlib1g-dev libssl-dev -y
    # OpenSSL Information
    Linux_Include_Path=$(pkg-config --cflags openssl)
    Linux_Lib_Path=$(pkg-config --libs openssl)

    echo "Include path: $Linux_Include_Path"
    echo "Library path: $Linux_Lib_Path"

    if [ -f build ]; then
    # Remove old build directory
        rm -rf build/
    else
    # Create build folder
        mkdir -p build
    fi

    # Compile
    g++ src/zsign/*.cpp -I"$Linux_Include_Path" -L"$Linux_Lib_Path" -lssl -lcrypto -O3 -o build/zsign
fi