#!/usr/bin/env bash

echo "Running program in Release"

script_path=$(readlink -f -- "$0")
script_dir=$(dirname -- "$script_path")
ROOTSYS=${script_dir}/../vendor/root_v6.32.04

export LD_LIBRARY_PATH="${ROOTSYS}/lib:${LD_LIBRARY_PATH}"

cd "${script_dir}/../Energy-Distribution-Model"
./../bin/Release-debug-x86_64/Energy-Distribution-Model/Energy-Distribution-Model