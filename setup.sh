#!/bin/sh
# Run once after cloning to generate and build the TunnelFeverCPP static library.
# Requires CMake (brew install cmake) and the Firebase C++ SDK at
# ~/firebase_cpp_sdk (or update FIREBASE_INCLUDE_DIR in
# TunnelFeverCPP/CMakeLists.txt to match your path).

set -e

echo "==> Generating TunnelFeverCPP Xcode project..."
cmake -B TunnelFeverCPP/build -S TunnelFeverCPP -G Xcode

echo ""
echo "==> Building TunnelFeverCPP (Debug)..."
xcodebuild \
  -project TunnelFeverCPP/build/TunnelFeverCPP.xcodeproj \
  -target TunnelFeverCPP \
  -configuration Debug \
  -sdk iphonesimulator \
  SYMROOT="$(pwd)/TunnelFeverCPP/build" \
  BUILD_DIR="$(pwd)/TunnelFeverCPP/build" \
  build | grep -E "^(Build|error:|warning: ld)" || true

echo ""
echo "Done. Open TunnelFever.xcworkspace in Xcode."
