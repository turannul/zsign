#!/usr/bin/env bash
# shellcheck shell=bash
set -e

function print_info { printf "\033[34m%s\033[0m\n" "$1"; }
function print_success { printf "\033[32m✔️  %s\033[0m\n" "$1"; }
function print_warning { printf "\033[33m⚠️  %s\033[0m\n" "$1"; }
function print_error { printf "\033[31m❌  %s\033[0m\n" "$1"; }

compile() {
    if [ -d build ]; then
        print_info "Removing old build directory..."
        rm -rf build/
    fi
    mkdir -p build

    print_info "Starting build process..."
    cd build && cmake .. && make
    if [ -f zsign ]; then
        print_success "Build completed successfully"
        mv zsign ../Test/zsign
        cd ../
        test_zsign
    else
        print_error "Build failed. Please check the build output for errors."
        exit 1
    fi
}

test_zsign() {
    print_success "Running tests..."
    if ctest --rerun-failed --output-on-failure; then
        print_success "All tests passed."
        install_zsign
    else
        print_error "One or more tests failed. Check test output for details."
        exit 1
    fi
    print_info "Cleaning up..."
    rm -rf Testing
}

install_zsign() {
    if [[ -t 0 ]]; then
        read -rp "Do you want to install zsign to /usr/local/bin? (y/n): " install_choice
        if [[ $install_choice =~ ^[Yy]$ ]]; then
            print_info "Installing binary to /usr/local/bin/"
            if sudo mv Test/zsign /usr/local/bin/zsign; then
                print_success "zsign has been successfully installed to $(which zsign)"
            else
                print_error "Failed to install zsign to /usr/local/bin/ directory.\nzsign will be moved to build directory instead."
                mv Test/zsign build/zsign
                print_warning "zsign has been moved to the build directory."
            fi
        else
            print_info "Moving zsign to the build directory..."
            mv Test/zsign build/zsign
            print_success "zsign has been moved to the build directory."
        fi
    else
        print_warning "Non-interactive environment detected: Moving zsign to the build directory..."
        mv Test/zsign build/zsign
        print_success "zsign has been moved to the build directory."
    fi
}

chk_brew_pkg() {
    if brew list --formula | grep -q "$1"; then
        print_success "$1 is already installed."
    else
        print_warning "$1 is not installed. Installing now..."
        brew install "$1" -q
    fi
}

chk_apt_pkg() {
    if dpkg -l | grep -q "$1"; then
        print_success "$1 is already installed."
    else
        print_warning "$1 is not installed. Installing now..."
        sudo apt update &>/dev/null
        sudo apt install -y "$1"
    fi
}

chk_yum_pkg() {
    if yum list installed "$1" &>/dev/null; then
        print_success "$1 is already installed."
    else
        print_warning "$1 is not installed. Installing now..."
        sudo yum install -y "$1"
    fi
}

chk_dnf_pkg() {
    if dnf list installed "$1" &>/dev/null; then
        print_success "$1 is already installed."
    else
        print_warning "$1 is not installed. Installing now..."
        sudo dnf install -y "$1"
    fi
}

chk_pacman_pkg() {
    if pacman -Qi "$1" &>/dev/null; then
        print_success "$1 is already installed."
    else
        print_warning "$1 is not installed. Installing now..."
        sudo pacman -S --noconfirm "$1"
    fi
}

chk_zypper_pkg() {
    if zypper se -i -x "$1" &>/dev/null; then
        print_success "$1 is already installed."
    else
        print_warning "$1 is not installed. Installing now..."
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
                print_error "Unsupported package manager: $pkg_manager"
                exit 1
                ;;
        esac
    done
}

if [[ "$(uname)" == "Darwin" ]]; then
    print_info "Setting up macOS environment..."
    if ! command -v brew &> /dev/null; then
        print_error "Homebrew is not installed. Please install it before proceeding."
        exit 1
    fi

    chk_brew_pkg p7zip
    chk_brew_pkg openssl@3
    chk_brew_pkg cmake

    compile

else
    print_info "Setting up Linux environment..."
    if command -v apt-get &> /dev/null; then
        print_info "Debian/Ubuntu based system detected."
        install_dependencies apt p7zip build-essential checkinstall zlib1g-dev libssl-dev cmake
    elif command -v yum &> /dev/null; then
        print_info "Red Hat/CentOS based system detected."
        install_dependencies yum p7zip gcc-c++ make zlib-devel openssl-devel cmake
    elif command -v dnf &> /dev/null; then
        print_info "Fedora based system detected."
        install_dependencies dnf p7zip gcc-c++ make zlib-devel openssl-devel cmake
    elif command -v pacman &> /dev/null; then
        print_info "Arch Linux based system detected."
        install_dependencies pacman p7zip base-devel zlib openssl cmake
    elif command -v zypper &> /dev/null; then
        print_info "OpenSUSE based system detected."
        install_dependencies zypper p7zip gcc-c++ make zlib-devel libopenssl-devel cmake
    else
        print_error "Unsupported Linux distribution. Please install the required packages manually."
        exit 1
    fi
    compile
fi