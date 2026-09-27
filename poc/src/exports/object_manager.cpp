#include <appdome.hpp>
#include <logcat.hpp>

#include <unordered_map>

#include <sys/mman.h>
#include <sys/sysconf.h>

struct mapped_object_t
{
    void* mapped = nullptr;
    size_t original_size = 0;
    size_t mapped_size = 0;
};

static std::unordered_map<uintptr_t /*addr*/, exports::object_metadata_t /*code*/> OBJECT_CODE_MAP;
static std::unordered_map<uintptr_t /*addr*/, mapped_object_t> OBJECT_MAP;
static uintptr_t JNI_ONLOAD_ADDY = 0;
static exports::bss_info_t BSS_INFO{};

void hex_dump(const uint8_t* data, size_t size)
{
    for (size_t i = 0; i < size; i += 16)
    {
        std::string line;
        for (size_t j = 0; j < 16 && i + j < size; ++j)
        {
            char byte_str[4];
            snprintf(byte_str, sizeof(byte_str), "%02x ", data[i + j]);
            line += byte_str;
        }
        log_D("%s", line.c_str());
    }
}

std::vector<uint8_t> get_setup_code(uintptr_t return_to)
{
#if defined(__arm__)
    const uint32_t return_to_u32 = static_cast<uint32_t>(return_to);

    // preentry shim:
    //   ldr.w r12, [pc, #4]
    //   b.n +4
    //   nop
    //   .word return_to
    return {
        0xDF, 0xF8, 0x04, 0xC0,
        0x02, 0xE0,
        0x00, 0xBF,
        static_cast<uint8_t>(return_to_u32 & 0xFF),
        static_cast<uint8_t>((return_to_u32 >> 8) & 0xFF),
        static_cast<uint8_t>((return_to_u32 >> 16) & 0xFF),
        static_cast<uint8_t>((return_to_u32 >> 24) & 0xFF)
    };
#else
    (void)return_to;
    return {};
#endif
}

std::vector<uint8_t> get_return_back_code()
{
#if defined(__aarch64__)
    // br x17
    return {0x20, 0x02, 0x1f, 0xd6};
#elif defined(__arm__)
    // thumb bx r12
    return {0x60, 0x47};
#elif defined(__x86_64__)
    // jmp r10
    return {0x41, 0xFF, 0xE2};
#else
    return {};
#endif
}

std::pair<void*, size_t> get_or_map_object(const uintptr_t address, uintptr_t return_to)
{
    auto it = OBJECT_MAP.find(address);
    if (it != OBJECT_MAP.end())
    {
        const auto& cached = it->second;
        return {cached.mapped, cached.original_size};
    }

    auto code_it = OBJECT_CODE_MAP.find(address);
    if (code_it == OBJECT_CODE_MAP.end())
    {
        log_E("no code found for: 0x%lx", address);
        return {nullptr, 0};
    }

    auto metadata = code_it->second;
    auto alloc = metadata.code;
    if (alloc.empty())
    {
        log_E("skipping empty object mapping at: 0x%lx", address);
        return {nullptr, 0};
    }

    const auto setup_code = get_setup_code(return_to);
    const auto return_back_code = get_return_back_code();

    alloc.insert(alloc.begin(), setup_code.begin(), setup_code.end());

    // we have to append the return back at the end of the code as it runs independently
    alloc.insert(alloc.end(), return_back_code.begin(), return_back_code.end());

    log_D("alloc hex");
    hex_dump(alloc.data(), alloc.size());

    void* mapped = mmap(nullptr, alloc.size(), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (mapped == MAP_FAILED)
    {
        log_E("failed to mmap memory for object: 0x%lx", address);
        return {nullptr, 0};
    }

    const size_t original_size = metadata.has_original_blob_size ? metadata.original_blob_size : alloc.size();

    log_D("final map sz %zu", alloc.size());
    log_D("original size: %zu", original_size);

    std::copy(alloc.begin(), alloc.end(), reinterpret_cast<uint8_t*>(mapped));
    __builtin___clear_cache(reinterpret_cast<char*>(mapped), reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(mapped) + alloc.size()));

    OBJECT_MAP[address] = {mapped, original_size, alloc.size()};
    return {mapped, original_size};
}

size_t get_object_original_size(const uintptr_t address)
{
    auto code_it = OBJECT_CODE_MAP.find(address);
    if (code_it == OBJECT_CODE_MAP.end())
    {
        log_E("no code found for: 0x%lx", address);
        return 0;
    }

    const auto& metadata = code_it->second;
    if (metadata.code.empty())
    {
        log_E("skipping empty object while fetching size at: 0x%lx", address);
        return 0;
    }

    return metadata.has_original_blob_size ? metadata.original_blob_size : metadata.code.size();
}

void patch_all_objects(uintptr_t base)
{
    for (const auto& [address, alloc] : OBJECT_CODE_MAP)
    {
        if (address == BSS_INFO.start || address == JNI_ONLOAD_ADDY)
        {
            continue;
        }
        exports::handle_object_patch(address, base, false, THUMB_BIT);
    }
    log_D("exiting");
}

uintptr_t get_jni_onload_address()
{
    return JNI_ONLOAD_ADDY;
}

exports::bss_info_t get_bss_info()
{
    return BSS_INFO;
}

void parse_object_data(const std::string& object_data, std::unordered_map<uintptr_t /*addr*/, exports::object_metadata_t /*code*/>& object_map, exports::bss_info_t& bss_info);
void setup_object_manager( )
{
    auto object_data = assets::get_from_name(_OBJECTS_ASSET_NAME);
    if ( object_data.empty( ) )
    {
        log_E( "Failed to load object data from asset: %s", _OBJECTS_ASSET_NAME );
        return;
    }

    exports::bss_info_t bss_info{};
    std::unordered_map<uintptr_t, exports::object_metadata_t> object_map;
    parse_object_data(object_data, object_map, bss_info);

    log_I("Parsed %zu objects", object_map.size());
    log_I("bss: 0x%lx - 0x%lx, trampoline_reloc: 0x%lx", bss_info.start, bss_info.end, bss_info.trampoline_reloc);

    BSS_INFO = bss_info;
    
    // biggest alloc is JNI_OnLoad
    size_t max_size = 0;
    for (auto& [address, metadata] : object_map)
    {
        OBJECT_CODE_MAP[ address ] = metadata;
        
        if (metadata.code.size() > max_size && address != bss_info.start) // bss can fuck us over. (43m wasted becasue of it.)
        {
            max_size = metadata.code.size();
            JNI_ONLOAD_ADDY = address;

            log_D( "jni onload address: 0x%lx", JNI_ONLOAD_ADDY );
            log_D( "max object size: %zu", max_size );
        }
    }
}


size_t exports::handle_object_patch(const uintptr_t _address, uintptr_t base, bool is_bss, uintptr_t thumb_bit)
{
    uintptr_t address = _address - thumb_bit; // for arm32 we need to sub 1 to get the actual start of the function

    auto it = OBJECT_CODE_MAP.find(_address); // mapped objs for arm32 have thumb bit
    if (it == OBJECT_CODE_MAP.end())
    {
        log_E("No object found for address: 0x%lx", _address);
        return 0;
    }

    const auto& alloc = it->second.code;
    if (alloc.empty())
    {
        log_E("Skipping empty object patch at address: 0x%lx", address);
        return 0;
    }

    log_I("Patching object at address: 0x%lx with size: %zu", address, alloc.size());
    hex_dump(alloc.data(), alloc.size());

    const auto rebased_address = address + base;
    const auto page_size_raw = sysconf(_SC_PAGESIZE);
    const auto page_size = static_cast<uintptr_t>(page_size_raw);
    const auto patch_start = rebased_address;
    const auto patch_end = rebased_address + alloc.size();
    const auto page_start = patch_start & ~(page_size - 1);
    const auto page_end = (patch_end + page_size - 1) & ~(page_size - 1);
    const auto protect_len = page_end - page_start;

    // log_D("ok unprotecting memory at address: 0x%lx", rebased_address);
    if (mprotect(reinterpret_cast<void*>(page_start), protect_len, PROT_READ | PROT_WRITE | PROT_EXEC) != 0)
    {
        log_E("Failed to unprotect memory at address: 0x%lx", rebased_address);
        return 0;
    }

    // log_D("ok patching code to address: 0x%lx", rebased_address);
    std::copy(alloc.begin(), alloc.end(), reinterpret_cast<uint8_t*>(rebased_address));
    __builtin___clear_cache(reinterpret_cast<char*>(patch_start), reinterpret_cast<char*>(patch_end));

    // log_D("ok reprotecting memory at address: 0x%lx", rebased_address);
    if ( !is_bss ) // we HAVE to leave bss as rwx
    {
        if ( mprotect( reinterpret_cast< void* >( page_start ), protect_len, PROT_READ | PROT_EXEC ) != 0 )
        {
            log_E( "Failed to reprotect memory at address: 0x%lx", rebased_address );
            return 0;
        }
    }

    // log_I("Successfully patched object at address: 0x%lx", address);
    return alloc.size();
}