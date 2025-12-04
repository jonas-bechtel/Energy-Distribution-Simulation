#!/usr/bin/env bash

# List of packages to install
PACKAGES=(
    "make"
    "build-essential"
    "libsdl2-dev"
    "libgsl-dev"
    "libgif-dev"
    "libxft2"
)

# Install packages
sudo apt update
sudo apt install -y "${PACKAGES[@]}"