#pragma once

#include <string>

namespace assets
{
    // Optional eager initialization; getters also initialize lazily. Failed
    // discovery is retried on the next call. Requires a mapped base.apk.
    bool init();
    std::string get_apk_path();
    // Best effort: inferred from the APK's Android install directory.
    std::string get_package_name();

    // name is relative to assets/ (an existing assets/ prefix is accepted).
    // Returns binary bytes. Missing/invalid and empty assets both return "".
    // Supports stored/deflated ZIP entries, excluding ZIP64 and encryption.
    std::string get_apk_asset(const std::string &name);
    std::string get_by_uuid(const std::string &uuid);
    std::string get_from_name(const std::string &name);
    std::string get_as_key(const std::string &name);

    void populate_hash_map(const std::string &blobs_config);
}
