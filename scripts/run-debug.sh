#!/usr/bin/env bash

echo "Running program in Debug"

ROOTSYS=$(dirname "$0")/../vendor/root_v6.32.04
export PATH="$ROOTSYS/bin:$PATH"

cd "$(dirname "$0")/../Energy-Distribution-Model"

./bin/Debug-linux-x86_64/Energy-Distribution-Model  # Note the escaping of spaces