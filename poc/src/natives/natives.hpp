#pragma once

#include <string>
#include <vector>
#include <jni.h>

namespace natives
{
    enum class native_method_translate
    {
        NT_INIT_CB,
        NT_ACTIVITY_CB,
        NT_ACTIVITY_CB2,
        NT_ON_PAUSE_CB,
        NT_STRING_CB,
        NT_ERROR_CB,

        NT_stringIndexerNativeInitializer,
        NT_getStringIndexerDB,
        NT_getApplicationIDAppdome,
        NT_getStringFromConfig,
        NT_appRegisteredToEvent,
        NT_appUnRegisteredFromEvent,
        NT_dynamicInitDone,
    };

    struct native_method_t
    {
        std::string name;                       // class->method
        std::string clazz;                      // class
        std::string signature;                  // sig
        native_method_translate translate_type; // real name
    };

    std::vector<native_method_t> get_native_methods_to_register();
    std::string get_appdome_classes_obf_mapping();
    std::string get_native_methods_to_register_obf();
    JNINativeMethod* map_to_jni_arr(const std::vector<native_method_t> &methods);
}