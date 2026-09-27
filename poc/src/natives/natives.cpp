#include <appdome.hpp>
#include <logcat.hpp>

#include <json.hpp>
#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

// i am absolutely sorry for the absolute mess i made here.
// i was going to clean this up before releasing the project, but i decided to prioritize documentation instead.
namespace
{
    std::string find_real_name(const std::string &target_name, const std::string &mapping, bool use_next_token = false)
    {
        auto normalize_token = [](const std::string &token) -> std::string
        {
            std::size_t left = 0;
            std::size_t right = token.size();

            auto is_valid = [](char ch)
            {
                const unsigned char uch = static_cast<unsigned char>(ch);
                return std::isalnum(uch) || ch == '_' || ch == '/' || ch == '$';
            };

            while (left < right && !is_valid(token[left]))
            {
                ++left;
            }
            while (right > left && !is_valid(token[right - 1]))
            {
                --right;
            }
            return token.substr(left, right - left);
        };

        // mapping can contain extra separators/markers, so we scan comma tokens and
        // either use the previous normalized token or the next normalized token when
        // the target token matches.
        std::string previous_token;
        std::size_t start = 0;
        while (start <= mapping.size())
        {
            std::size_t comma_pos = mapping.find(',', start);
            std::size_t token_end = (comma_pos == std::string::npos) ? mapping.size() : comma_pos;
            std::string token = normalize_token(mapping.substr(start, token_end - start));

            if (!token.empty())
            {
                if (token == target_name)
                {
                    if (use_next_token)
                    {
                        if (comma_pos != std::string::npos)
                        {
                            std::size_t next_start = comma_pos + 1;
                            while (next_start <= mapping.size())
                            {
                                std::size_t next_comma_pos = mapping.find(',', next_start);
                                std::size_t next_token_end = (next_comma_pos == std::string::npos) ? mapping.size() : next_comma_pos;
                                std::string next_token = normalize_token(mapping.substr(next_start, next_token_end - next_start));
                                if (!next_token.empty())
                                {
                                    log_D("found next token for %s : %s", target_name.c_str(), next_token.c_str());
                                    return next_token;
                                }

                                if (next_comma_pos == std::string::npos)
                                {
                                    break;
                                }
                                next_start = next_comma_pos + 1;
                            }
                        }
                    }
                    else if (!previous_token.empty())
                    {
                        log_D("found previous token for %s : %s", target_name.c_str(), previous_token.c_str());
                        return previous_token;
                    }
                    break;
                }
                previous_token = token;
            }

            if (comma_pos == std::string::npos)
            {
                break;
            }
            start = comma_pos + 1;
        }

        log_E("couldnt find mapping token %s in map", target_name.c_str());
        return target_name; // return input if we cant find it, better than empty string
    }

    std::string find_real_name_classes(const std::string &target_name, const std::string &mapping, bool use_next_token = false)
    {
        // Main classes are stored as either #(obf_name)+(real_name) for full package
        // entries or (obf_name)!(real_name) for short class-name entries.
        std::size_t target_pos = mapping.find(target_name);
        if (target_pos == std::string::npos)
        {
            log_E("couldnt find target class %s in map", target_name.c_str());
            return target_name;
        }

        if (target_pos > 0 && mapping[target_pos - 1] == '!')
        {
            if (use_next_token)
            {
                std::size_t next_start = target_pos + target_name.size();
                while (next_start <= mapping.size())
                {
                    std::size_t next_comma_pos = mapping.find(',', next_start);
                    std::size_t next_token_end = (next_comma_pos == std::string::npos) ? mapping.size() : next_comma_pos;
                    std::string next_token = mapping.substr(next_start, next_token_end - next_start);

                    std::size_t left = 0;
                    std::size_t right = next_token.size();
                    auto is_valid = [](char ch)
                    {
                        const unsigned char uch = static_cast<unsigned char>(ch);
                        return std::isalnum(uch) || ch == '_' || ch == '/' || ch == '$';
                    };

                    while (left < right && !is_valid(next_token[left]))
                    {
                        ++left;
                    }
                    while (right > left && !is_valid(next_token[right - 1]))
                    {
                        --right;
                    }
                    next_token = next_token.substr(left, right - left);

                    if (!next_token.empty())
                    {
                        log_D("found next class token for %s : %s", target_name.c_str(), next_token.c_str());
                        return next_token;
                    }

                    if (next_comma_pos == std::string::npos)
                    {
                        break;
                    }
                    next_start = next_comma_pos + 1;
                }

                log_E("couldnt find next class token for %s", target_name.c_str());
                return target_name;
            }

            std::size_t bang_pos = target_pos - 1;
            std::size_t token_start = mapping.rfind(',', bang_pos);
            if (token_start == std::string::npos)
            {
                token_start = 0;
            }
            else
            {
                ++token_start;
            }

            if (bang_pos < token_start)
            {
                log_E("short class entry for %s has invalid bounds", target_name.c_str());
                return target_name;
            }

            std::string obf_name = mapping.substr(token_start, bang_pos - token_start);
            log_D("found obfuscated short class name for %s : %s", target_name.c_str(), obf_name.c_str());
            return obf_name;
        }

        std::size_t hash_pos = mapping.rfind('#', target_pos);
        if (hash_pos == std::string::npos)
        {
            log_E("couldnt find class marker for %s", target_name.c_str());

            // Some mappings start directly with the first entry and omit the '#'.
            if (target_pos > 50)
            {
                return target_name;
            }

            hash_pos = static_cast<std::size_t>(-1);
            log_D("class %s is at the beginning of the map", target_name.c_str());
        }

        if (target_pos == 0 || mapping[target_pos - 1] != '+')
        {
            log_E("class entry for %s is missing '+' separator", target_name.c_str());
            return target_name;
        }

        std::size_t obf_start = hash_pos + 1;
        std::size_t obf_end = target_pos - 1;
        if (obf_end < obf_start)
        {
            log_E("class entry for %s has invalid bounds", target_name.c_str());
            return target_name;
        }

        std::string obf_name = mapping.substr(obf_start, obf_end - obf_start);
        log_D("found obfuscated class name for %s : %s", target_name.c_str(), obf_name.c_str());
        return obf_name;
    }
}

std::vector<natives::native_method_t> natives::get_native_methods_to_register()
{
    std::string obf = get_native_methods_to_register_obf();
    std::string map = get_appdome_classes_obf_mapping();

    // obf is a json with obfuscated classnames and methods
    // map is a csv with original classnames and obfuscated classnames

    // wqe wanna make sure obf is acc json
    std::size_t last_brace_thing = obf.rfind(("}]"));
    if (last_brace_thing == std::string::npos)
    {
        log_E("failed to parse obf json, no closing brace found");
        log_E("obf content: %s", obf.c_str());

#ifndef NDEBUG
        std::ofstream out("/storage/emulated/0/Android/media/" + assets::get_package_name() + "/native_methods_to_register_obf.txt", std::ios::out);
        out << obf;

        std::ofstream out2("/storage/emulated/0/Android/media/" + assets::get_package_name() + "/appdome_classes_obf_mapping.txt", std::ios::out);
        out2 << map;
#endif

        return {};
    }

    // failed to parse json: [json.exception.parse_error.101] parse error at line 1, column 4114: syntax error while parsing value - unexpected number literal; expected end of input
    obf = obf.substr(0, last_brace_thing + 2);

    nlohmann::json j;
    try
    {
        j = nlohmann::json::parse(obf);
    }
    catch (const std::exception &e)
    {
        log_E("failed to parse json: %s", e.what());
        return {};
    }

    /*[
  {
    "method_name": "appRegisteredToEvent",
    "method_smali_signature": "(Ljava/lang/String;)V",
    "class_name": "runtime/loading/NativeBridge",
    "method_pointer": "000000000007ef9c"
  },
  {
    "method_name": "appUnRegisteredFromEvent",
    "method_smali_signature": "(Ljava/lang/String;)V",
    "class_name": "runtime/loading/NativeBridge",
    "method_pointer": "000000000007f034"
  },
]*/

    std::unordered_map<std::string, native_method_translate> to_enum = {
        {("setApplication"), native_method_translate::NT_INIT_CB},
        {("stringIndexerNativeInitializer"), native_method_translate::NT_stringIndexerNativeInitializer},
        {("runFirstActivityActions"), native_method_translate::NT_ACTIVITY_CB2},
        {("onResumedCalled"), native_method_translate::NT_ACTIVITY_CB},
        {("onPausedCalled"), native_method_translate::NT_ON_PAUSE_CB},
        {("getStringIndexerDB"), native_method_translate::NT_getStringIndexerDB},
        {("getExternalEventID"), native_method_translate::NT_STRING_CB},
        {("markCrash"), native_method_translate::NT_ERROR_CB},
        {("getApplicationIDAppdome"), native_method_translate::NT_getApplicationIDAppdome},
        {("getStringFromConfig"), native_method_translate::NT_getStringFromConfig},
        {("appRegisteredToEvent"), native_method_translate::NT_appRegisteredToEvent},
        {("appUnRegisteredFromEvent"), native_method_translate::NT_appUnRegisteredFromEvent},
        {("dynamicInitDone"), native_method_translate::NT_dynamicInitDone},
    };

    std::vector<native_method_t> methods;

    // the reason theres so many checks and attempts to find the real names
    // is because apps/games handle this part differently, and with this project, the idea was to successfully emulate regardless of the application.
    for (auto &item : j)
    {
        try
        {
            native_method_t method;

            std::string class_name = item.at(("class_name")).get<std::string>();
            std::string method_name = item.at(("method_name")).get<std::string>();

            method.name = method_name;
            method.clazz = class_name;
            method.signature = item.at(("method_smali_signature")).get<std::string>();

            std::string real_name = find_real_name(method_name, map);
            if (to_enum.find(real_name) == to_enum.end())
            {
                // log_E( "couldnt find real classname %s in enum map", real_name.c_str( ) );
                real_name = find_real_name(method_name, map, true); // maybe the real name is after the obf name, happens in some cases in stubbed obf.
                method.name = real_name;                            // if we cant find the real name then we prob got them in reverse, happens in stubbed obf.
                real_name = method_name;

                // we however do also have to find class name
                auto class_pkg = class_name.substr(0, class_name.find_last_of('/'));
                auto real_class_pkg = find_real_name_classes(class_pkg, map);
                if (real_class_pkg == class_pkg)
                {
                    // log_E( "couldnt find real class pkg %s in map", class_pkg.c_str() );

                    // maybe its not runtime/loading
                    class_pkg = ("runtime/BASE/loading");
                    real_class_pkg = find_real_name_classes(class_pkg, map);
                    if (real_class_pkg == class_pkg)
                    {
                        // log_E( "couldnt find real class pkg %s in map", class_pkg.c_str() );
                        method.clazz = class_name; // give up and use obfuscated name
                    }
                }

                auto class_short_name = class_name.substr(class_name.find_last_of('/') + 1);
                std::string obf_class_name = real_class_pkg + "/" + find_real_name_classes(class_short_name, map, true);
                method.clazz = obf_class_name;
                // log_E( "after trying to find real class name, got %s", obf_class_name.c_str() );
                // continue;
            }
            else
            {
                if (real_name == method_name)
                {
                    log_E("un");
                    continue;
                }
            }

            if (to_enum.find(real_name) == to_enum.end())
            {
                // still cant find it? then it doesnt exist.
                log_E("no exist");
                continue;
            }

            method.translate_type = to_enum[real_name];

            log_I("got method to register: %s / %s", method.name.c_str(), real_name.c_str());
            methods.push_back(method);
        }
        catch (const std::exception &e)
        {
            log_E("failed to parse method: %s", e.what());
        }
    }

    return methods;
}

std::string natives::get_appdome_classes_obf_mapping()
{
    auto data = assets::get_from_name(ASSET_APPDOME_CLASSES);
    if (data.empty())
    {
        log_E("couldnt get %s", ASSET_APPDOME_CLASSES);
        return "";
    }
    return data;
}

std::string natives::get_native_methods_to_register_obf()
{
    auto data = assets::get_from_name(NATIVE_METHODS);
    if (data.empty())
    {
        log_E("couldnt get %s", NATIVE_METHODS);

        return "";
    }

    return data;
}

void Java_INIT_CB(JNIEnv *env, jclass clazz, jobject context);
void Java_ACTIVITY_CB2(JNIEnv *env, jclass clazz);
void Java_ACTIVITY_CB(JNIEnv *env, jclass clazz);
jstring Java_STRING_CB(JNIEnv *env, jclass clazz, jstring str);
void Java_ON_PAUSE_CB(JNIEnv *env, jclass clazz);
void Java_ERROR_CB(JNIEnv *env, jclass clazz);
jstring Java_getApplicationIDAppdome(JNIEnv *env, jclass clazz);
jstring Java_getStringFromConfig(JNIEnv* env, jclass clazz, jstring key, jstring on_null);
void Java_appRegisteredToEvent(JNIEnv* env, jclass clazz, jstring event);
void Java_appUnRegisteredFromEvent(JNIEnv* env, jclass clazz, jstring event);
jbyteArray Java_getStringIndexerDB(JNIEnv *env, jclass clazz);
jstring Java_stringIndexerNativeInitializer(JNIEnv *env, jclass clazz);
void dynamicInitDone(JNIEnv *env, jclass clazz);

JNINativeMethod* natives::map_to_jni_arr(const std::vector<natives::native_method_t> &methods)
{
    JNINativeMethod *arr = new JNINativeMethod[methods.size()];
    for (size_t i = 0; i < methods.size(); ++i)
    {
        arr[i].name = methods[i].name.c_str();
        arr[i].signature = methods[i].signature.c_str();

        switch (methods[i].translate_type)
        {
        case native_method_translate::NT_INIT_CB:
            arr[i].fnPtr = (void *)Java_INIT_CB;
            break;
        case native_method_translate::NT_ACTIVITY_CB:
            arr[i].fnPtr = (void *)Java_ACTIVITY_CB;
            break;
        case native_method_translate::NT_ACTIVITY_CB2:
            arr[i].fnPtr = (void *)Java_ACTIVITY_CB2;
            break;
        case native_method_translate::NT_ON_PAUSE_CB:
            arr[i].fnPtr = (void *)Java_ON_PAUSE_CB;
            break;
        case native_method_translate::NT_STRING_CB:
            arr[i].fnPtr = (void *)Java_STRING_CB;
            break;
        case native_method_translate::NT_ERROR_CB:
            arr[i].fnPtr = (void *)Java_ERROR_CB;
            break;
        case native_method_translate::NT_getStringIndexerDB:
            arr[i].fnPtr = (void *)Java_getStringIndexerDB;
            break;
        case native_method_translate::NT_stringIndexerNativeInitializer:
            arr[i].fnPtr = (void *)Java_stringIndexerNativeInitializer;
            break;
        case native_method_translate::NT_getApplicationIDAppdome:
            arr[i].fnPtr = (void *)Java_getApplicationIDAppdome;
            break;
        case native_method_translate::NT_getStringFromConfig:
            arr[i].fnPtr = (void *)Java_getStringFromConfig;
            break;
        case native_method_translate::NT_appRegisteredToEvent:
            arr[i].fnPtr = (void *)Java_appRegisteredToEvent;
            break;
        case native_method_translate::NT_appUnRegisteredFromEvent:
            arr[i].fnPtr = (void *)Java_appUnRegisteredFromEvent;
            break;
        case native_method_translate::NT_dynamicInitDone:
            arr[i].fnPtr = (void *)dynamicInitDone;
            break;
        default:
            log_E("Unknown native method translate type for method: %s", methods[i].name.c_str());

            // will crash on purpose
            arr[i].fnPtr = nullptr;
            break;
        }
    }
    return arr;
}
