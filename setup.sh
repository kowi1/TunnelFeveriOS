#!/bin/sh
# Run once after cloning to generate the TunnelFeverCPP Xcode project.
# Requires CMake (brew install cmake) and the Firebase C++ SDK at
# /Users/mac/firebase_cpp_sdk (or update FIREBASE_INCLUDE_DIR in
# TunnelFeverCPP/CMakeLists.txt to match your path).

set -e

cmake -B TunnelFeverCPP/build -S TunnelFeverCPP -G Xcode

echo ""
echo "Done. Open TunnelFever.xcworkspace in Xcode."
