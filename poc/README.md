# Replacement library

This source accompanies the [Appdome investigation](../README.md). It replaces the native protection library and implements the initialization, data interfaces, and export-restoration behavior described in the writeup.

I developed this during the February–April 2026 investigation. My tests covered ARM64 on a Samsung Galaxy A71 running Android 13 and an ARM64 emulator. Other architecture paths are present in the source, but weren't part of those tests. Expect to adapt the code and extracted material for other protected builds.

## Source navigation

| Area | Responsibility |
| --- | --- |
| [entry.cpp](src/entry.cpp) | Fixed `lkjhgf()V` registration and subsequent native initialization |
| [assets](src/assets/README.md) | APK access, asset-name lookup, and loading data |
| [crypt](src/crypt/README.md) | Asset decryption implementation |
| [natives](src/natives/README.md) | Native identity mapping, JNI registration, and callbacks |
| [exports](src/exports/README.md) | Code records, initialization, allocation, and export resolution |
| [replaceable](src/replaceable/README.md) | Data that varies with the application or build |

For each application, I extracted the filename-hashing salt and AES material from the original library, substituted `libloader.so`, and rebuilt and re-signed the APK. The blog article explains how I arrived at that implementation and what happened when I tried it in other applications.

`APP_LIB_NAME` in [appdome.hpp](src/appdome.hpp) is intentionally empty. Set it to the application's native library name when using the export-restoration path, alongside the extracted inputs in [replaceable](src/replaceable/README.md).

The ARM64 [build helper](scripts/build-arm64.sh) and [CMake configuration](src/CMakeLists.txt) use an Android NDK toolchain to build the `loader` shared library.

## Images and supporting samples

The article's screenshots are in [assets/images](../assets/images/README.md). Mappings, recovered classes, string data, and extracted blobs are in [assets/samples](../assets/samples/README.md). I've removed identifying names from these copies; the originals and private notes stay outside the repository.
