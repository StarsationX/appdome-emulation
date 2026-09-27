#pragma once

#include <assets/assets.hpp>
#include <natives/natives.hpp>
#include <exports/exports.hpp>


#define APP_LIB_NAME ""
#define BLOBS_CONFIG_NAME "uTPEauDexK34zwVRiCRp"
#define ASSETS_PACKAGES_DIR "j1O1pP4cpnaLPxs2xoSf/"
#define STRING_INDEXER_DB_SUFFIX "_JavaStringIndexerStringDb_"
#define ASSET_PREFIX "native_methods_to_register_"
#define ASSET_APPDOME_CLASSES "appdome_classes_obf_mapping"

#if defined( __aarch64__ )
#define _ARCH_NAME "arm64-v8a"
#define _OBJECTS_ASSET_NAME "shared_objects_code_restore_arm64"
#elif defined( __arm__ )
#define _ARCH_NAME "armeabi-v7a"
#define _OBJECTS_ASSET_NAME "shared_objects_code_restore_arm32"
#elif defined( __x86_64__ )
#define _ARCH_NAME "x86_64"
#define _OBJECTS_ASSET_NAME "shared_objects_code_restore_x86_64"
#endif

#if defined(__arm__)
#define THUMB_BIT 1
#else
#define THUMB_BIT 0
#endif

#define NATIVE_METHODS ASSET_PREFIX _ARCH_NAME