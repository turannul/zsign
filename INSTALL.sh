#!/bin/bash
make_dir(){
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
    mv -v zsign /usr/local/bin/zsign
    chmod -v 777 /usr/local/bin/zsign
    rm -rf build
}

if [[ "$(uname)" == "Darwin" ]]; then
    echo "Installing macOS dependencies..."
    brew install zip unzip openssl@3 cmake -q || { echo "Failed to install macOS dependencies"; exit 1; }
    compile

else
    echo "Updating and installing Linux dependencies..."
    sudo apt-get update 
    sudo apt-get install -y zip unzip build-essential checkinstall zlib1g-dev libssl-dev pkg-config cmake || { echo "Failed to install Linux dependencies"; exit 1; }
    compile
fi