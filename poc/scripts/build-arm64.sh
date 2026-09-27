#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/build-arm64.sh /path/to/android-ndk
project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
ndk_dir="${1:-${ANDROID_NDK_HOME:-${ANDROID_NDK_ROOT:-}}}"
build_dir="$project_dir/build/arm64-v8a"

if [[ -z "$ndk_dir" || ! -f "$ndk_dir/build/cmake/android.toolchain.cmake" ]]; then
    echo "Pass a valid Android NDK directory or set ANDROID_NDK_HOME." >&2
    exit 1
fi
ndk_dir="$(cd -- "$ndk_dir" && pwd)"

cmake -S "$project_dir/src" -B "$build_dir" \
    -DCMAKE_TOOLCHAIN_FILE="$ndk_dir/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM="android-${ANDROID_API:-21}" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" --config Release --parallel

echo "Built: $build_dir/libloader.so"
