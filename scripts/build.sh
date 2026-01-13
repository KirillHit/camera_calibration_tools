#!/usr/bin/env sh

set -eu

print_section() {
    echo ""
    echo "==> $1"
}

print_section "Starting build process..."

# Detect platform
UNAME="$(uname 2>/dev/null | tr '[:upper:]' '[:lower:]' || echo unknown)"
case "$UNAME" in
    linux*) PLATFORM="linux" ;;
    darwin*) PLATFORM="macos" ;;
    mingw*|msys*|cygwin*) PLATFORM="windows" ;;
    *) PLATFORM="unknown" ;;
esac

print_section "Detected platform: $PLATFORM"

# Directories
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
SRC_DIR="$ROOT_DIR/calib_tools"
ARTIFACTS_DIR="$ROOT_DIR/artifacts/$PLATFORM"
BUILD_DIR="$ARTIFACTS_DIR/build"
INSTALL_DIR="$ARTIFACTS_DIR/install"
DIST_DIR="$ARTIFACTS_DIR/dist"

mkdir -p "$BUILD_DIR" "$INSTALL_DIR" "$DIST_DIR"

# CMake generator
CMAKE_GEN="Ninja"

# Optional compilers for Windows
CMAKE_COMPILER_C=""
CMAKE_COMPILER_CXX=""
if [ "$PLATFORM" = "windows" ]; then
    CMAKE_COMPILER_C="x86_64-w64-mingw32-gcc"
    CMAKE_COMPILER_CXX="x86_64-w64-mingw32-g++"
fi

# CMake configuration
CMAKE_ARGS="-S $SRC_DIR -B $BUILD_DIR -G $CMAKE_GEN -DCMAKE_BUILD_TYPE=Release"

[ -n "$CMAKE_COMPILER_C" ] && CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_C_COMPILER=$CMAKE_COMPILER_C"
[ -n "$CMAKE_COMPILER_CXX" ] && CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_CXX_COMPILER=$CMAKE_COMPILER_CXX"

print_section "Configuring project..."
cmake $CMAKE_ARGS

if [ "$PLATFORM" != "windows" ]; then
    print_section "Building lupdate..."
    cmake --build "$BUILD_DIR" --target calib_tools_lupdate
else
    echo "Skipping lupdate on Windows (Qt DLLs not available in MSYS2)"
fi

print_section "Building project..."
cmake --build "$BUILD_DIR" --parallel

print_section "Installing project to $INSTALL_DIR..."
cmake --install "$BUILD_DIR" --prefix "$INSTALL_DIR" --config Release

# Package using CPack
case "$PLATFORM" in
    linux) FORMATS="DEB" ;;
    windows) FORMATS="NSIS" ;;
    macos) FORMATS="DMG" ;;
    *) FORMATS="TGZ" ;;
esac

print_section "Packing formats: $FORMATS"

for fmt in $FORMATS; do
    print_section "Packing format: $fmt"
    cd "$BUILD_DIR" && cpack -B "$DIST_DIR" -G "$fmt"
done

rm -rf "$DIST_DIR/_CPack_Packages"

print_section "Done! Installed files and packages are in $DIST_DIR"
