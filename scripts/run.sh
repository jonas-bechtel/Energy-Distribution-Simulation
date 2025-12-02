#!/usr/bin/env bash

echo "Running program in Release"

script_path=$(readlink -f -- "$0")
script_dir=$(dirname -- "$script_path")

ROOTSYS=${script_dir}/../vendor/root_v6.32.04

#export PATH="$ROOTSYS/bin:$PATH"
#export PATH="$ROOTSYS/lib:$PATH"
export LD_LIBRARY_PATH="${ROOTSYS}/lib:${LD_LIBRARY_PATH}"
echo "${ROOTSYS}/lib"

#cd "${ROOTSYS}"
#./bin/thisroot.sh

cd "${script_dir}/../Energy-Distribution-Model"
./../bin/Release-linux-x86_64/Energy-Distribution-Model/Energy-Distribution-Model