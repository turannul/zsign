#!/usr/bin/env bash

# shellcheck shell=bash
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
        cd ../
        test_zsign
    else
        echo "Build failed. Please check the build output for errors."
        exit 1
    fi
}

test_zsign() {
    echo "Running tests..."
    if ctest --rerun-failed --output-on-failure; then
        echo "All tests passed."
        
        read -rp "Do you want to install zsign to /usr/local/bin? (y/n): " install_choice
        if [[ $install_choice =~ ^[Yy]$ ]]; then
            echo "Installing binary to /usr/local/bin/"
            if sudo mv Test/zsign /usr/local/bin/zsign; then
                echo "zsign has been successfully installed to /usr/local/bin/"
            else
                echo "Failed to install zsign to /usr/local/bin/. You may need root privileges."
                echo "The binary will be moved to the build directory instead."
                mv Test/zsign build/zsign
                echo "zsign has been moved to the build directory."
            fi
        else
            echo "Moving zsign to the build directory..."
            mv Test/zsign build/zsign
            echo "zsign has been moved to the build directory."
        fi
    else
        echo "One or more tests failed. Check test output for details."
        exit 1
    fi

    echo "Cleaning up..."
    rm -rf Testing
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
        sudo apt update -q
        sudo apt install -y "$1"
    fi
}

chk_yum_pkg() {
    if yum list installed "$1" &>/dev/null; then
        echo "$1 is already installed."
    else
        echo "$1 is not installed. Installing now..."
        sudo yum install -y "$1"
    fi
}

chk_dnf_pkg() {
    if dnf list installed "$1" &>/dev/null; then
        echo "$1 is already installed."
    else
        echo "$1 is not installed. Installing now..."
        sudo dnf install -y "$1"
    fi
}

chk_pacman_pkg() {
    if pacman -Qi "$1" &>/dev/null; then
        echo "$1 is already installed."
    else
        echo "$1 is not installed. Installing now..."
        sudo pacman -S --noconfirm "$1"
    fi
}

chk_zypper_pkg() {
    if zypper se -i -x "$1" &>/dev/null; then
        echo "$1 is already installed."
    else
        echo "$1 is not installed. Installing now..."
        sudo zypper install -y "$1"
    fi
}

install_dependencies() {
    local pkg_manager=$1
    shift
    local packages=("$@")

    for package in "${packages[@]}"; do
        case $pkg_manager in
            apt)
                chk_apt_pkg "$package"
                ;;
            yum)
                chk_yum_pkg "$package"
                ;;
            dnf)
                chk_dnf_pkg "$package"
                ;;
            pacman)
                chk_pacman_pkg "$package"
                ;;
            zypper)
                chk_zypper_pkg "$package"
                ;;
            *)
                echo "Unsupported package manager: $pkg_manager"
                exit 1
                ;;
        esac
    done
}

if [[ "$(uname)" == "Darwin" ]]; then
    echo "Setting up macOS environment..."
    if ! command -v brew &> /dev/null; then
        echo "Homebrew is not installed. Please install it before proceeding."
        exit 1
    fi

    echo "Checking macOS dependencies..."
    chk_brew_pkg p7zip
    chk_brew_pkg openssl@3
    chk_brew_pkg cmake

    compile

else
    echo "Setting up Linux environment..."
    if command -v apt-get &> /dev/null; then
        echo "Debian/Ubuntu based system detected."
        install_dependencies apt p7zip build-essential checkinstall zlib1g-dev libssl-dev cmake
    elif command -v yum &> /dev/null; then
        echo "Red Hat/CentOS based system detected."
        install_dependencies yum p7zip gcc-c++ make zlib-devel openssl-devel cmake
    elif command -v dnf &> /dev/null; then
        echo "Fedora based system detected."
        install_dependencies dnf p7zip gcc-c++ make zlib-devel openssl-devel cmake
    elif command -v pacman &> /dev/null; then
        echo "Arch Linux based system detected."
        install_dependencies pacman p7zip base-devel zlib openssl cmake
    elif command -v zypper &> /dev/null; then
        echo "OpenSUSE based system detected."
        install_dependencies zypper p7zip gcc-c++ make zlib-devel libopenssl-devel cmake
    else
        echo "Unsupported Linux distribution. Please install the required packages manually."
        exit 1
    fi

    compile
fi