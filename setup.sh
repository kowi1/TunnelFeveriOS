#!/bin/sh
# Run once after cloning to generate and build the TunnelFeverCPP static library
# for both iOS simulator and device.
#
# Requirements:
#   brew install cmake ninja
#   Firebase C++ SDK at ~/firebase_cpp_sdk  (or update FIREBASE_INCLUDE_DIR in
#   TunnelFeverCPP/CMakeLists.txt to match your path)

set -e

SIM_SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)
DEV_SDK=$(xcrun --sdk iphoneos --show-sdk-path)
TARGETS="TunnelFeverCPP BulletDynamics BulletCollision LinearMath"

echo "==> Generating TunnelFeverCPP Xcode project (for workspace)..."
cmake -B TunnelFeverCPP/build -S TunnelFeverCPP -G Xcode

echo ""
echo "==> Building TunnelFeverCPP for iOS Simulator (arm64)..."
cmake -G "Ninja Multi-Config" \
  -DCMAKE_MAKE_PROGRAM="$(which ninja)" \
  -DCMAKE_OSX_SYSROOT="$SIM_SDK" \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  -B TunnelFeverCPP/build-iphonesimulator \
  -S TunnelFeverCPP
cmake --build TunnelFeverCPP/build-iphonesimulator \
  --target $TARGETS --config Debug

echo ""
echo "==> Building TunnelFeverCPP for iOS Device (arm64)..."
cmake -G "Ninja Multi-Config" \
  -DCMAKE_MAKE_PROGRAM="$(which ninja)" \
  -DCMAKE_OSX_SYSROOT="$DEV_SDK" \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  -B TunnelFeverCPP/build-iphoneos \
  -S TunnelFeverCPP
cmake --build TunnelFeverCPP/build-iphoneos \
  --target $TARGETS --config Debug

echo ""
echo "Done. Open TunnelFever.xcworkspace in Xcode."
