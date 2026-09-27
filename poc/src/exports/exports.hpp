#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace exports
{
    extern uintptr_t g_app_base;
    extern uintptr_t g_appdome_base;
    
    struct bss_info_t
    {
        uintptr_t start;
        uintptr_t end;
        uintptr_t trampoline_reloc;
    };

    struct object_metadata_t
    {
        uintptr_t decoded_address = 0;
        uint32_t object_size = 0;
        uint32_t original_blob_size = 0;
        size_t data_offset = 0;
        bool has_original_blob_size = false;

        std::vector<uint8_t> code;
    };

    uintptr_t get_app_base();
    uintptr_t get_appdome_base();

    void setup_handlers();
    size_t handle_object_patch(const uintptr_t address, uintptr_t base, bool is_bss = false, uintptr_t thumb_bit = 0);
}