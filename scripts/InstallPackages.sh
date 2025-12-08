#!/usr/bin/env bash

# List of packages to install
PACKAGES=(
    "make"
    "build-essential"
    "libsdl2-dev"
    "libgsl-dev"
    "libsdl2-image-dev"
    "libgif-dev"
    "libxft2"
    "libtbb-dev"
)

# Install packages
sudo apt update
sudo apt install -y "${PACKAGES[@]}"