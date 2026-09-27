#include <appdome.hpp>
#include <logcat.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <unordered_map>

int64_t decode_address(int64_t encoded_address)
{
    return (encoded_address >> 16);
}

bool is_in_range(uintptr_t value, uintptr_t lower, uintptr_t upper)
{
    return value >= lower && value <= upper;
}

template <typename T>
T read_value(const std::vector<uint8_t> &buffer, size_t &offset)
{
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        value |= static_cast<T>(buffer[offset + i]) << (i * 8);
    }
    offset += sizeof(T);
    return value;
}

// hex bytes have to be read in little endian order, so we need to reverse the byte order when reading them
template <typename T>
T read_int64(const std::vector<uint8_t> &buffer, size_t &offset)
{
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
    {
        value |= static_cast<T>(buffer[offset + sizeof(T) - 1 - i]) << (i * 8);
    }
    offset += sizeof(T);
    return value;
}

std::string read_string(const std::vector<uint8_t> &buffer, size_t &offset)
{
    std::string result;
    while (offset < buffer.size() && buffer[offset] != 0)
    {
        result += static_cast<char>(buffer[offset]);
        ++offset;
    }
    ++offset; // skip the null terminator
    return result;
}

enum OPCODES
{
    AP_INVALID = 0x00,

    AP_OBJECTADDRESS = 0x66,
    AP_OBJECTSIZE = 0x59,
    AP_SKIP4 = 0x7E,
    AP_SKIPN = 0x1C
};

uint8_t get_opcode(uint16_t opcode)
{
    // every opcode is encoded with flags
    // ( 0x66 << 8 ) | flags
    //

    // uint8_t marker = opcode & 0xFF;

    uint8_t actual_opcode = (opcode >> 8) & 0xFF;
    return actual_opcode;
}

// bool save_object_data(const std::vector<uint8_t> &object_data, const std::string &filename)
// {
//     std::string save_dir = "extracted_objects";
//     std::string full_path = save_dir + "/" + filename;

//     std::filesystem::create_directories(save_dir);

//     std::ofstream outputFile(full_path, std::ios::binary);
//     if (!outputFile)
//     {
//         return false;
//     }
//     outputFile.write(reinterpret_cast<const char *>(object_data.data()), object_data.size());
//     return true;
// }


bool parse_objectsize_block(const std::vector<uint8_t> &buffer, size_t block_offset, uint32_t &size_out, size_t &data_offset_out)
{
    if (block_offset + 7 >= buffer.size())
    {
        return false;
    }

    uint32_t size_from_plus3 = static_cast<uint32_t>(buffer[block_offset + 3]) |
                               (static_cast<uint32_t>(buffer[block_offset + 4]) << 8) |
                               (static_cast<uint32_t>(buffer[block_offset + 5]) << 16) |
                               (static_cast<uint32_t>(buffer[block_offset + 6]) << 24);

    uint32_t size_from_plus4 = static_cast<uint32_t>(buffer[block_offset + 4]) |
                               (static_cast<uint32_t>(buffer[block_offset + 5]) << 8) |
                               (static_cast<uint32_t>(buffer[block_offset + 6]) << 16) |
                               (static_cast<uint32_t>(buffer[block_offset + 7]) << 24);

    // payload starts after 8byte AP_OBJECTSIZE block in both known layouts.
    data_offset_out = block_offset + 8;

    bool plus3_valid = (size_from_plus3 != 0) && (data_offset_out + size_from_plus3 <= buffer.size());
    bool plus4_valid = (size_from_plus4 != 0) && (data_offset_out + size_from_plus4 <= buffer.size());

    if (plus3_valid && plus4_valid)
    {
        size_out = (buffer[block_offset + 3] == 0) ? size_from_plus4 : size_from_plus3;
        return true;
    }

    if (plus3_valid)
    {
        size_out = size_from_plus3;
        return true;
    }

    if (plus4_valid)
    {
        size_out = size_from_plus4;
        return true;
    }

    return false;
}

void parse_object_data(const std::string& object_data, std::unordered_map<uintptr_t /*addr*/, exports::object_metadata_t /*code*/>& object_map, exports::bss_info_t& bss_info)
{
    using namespace exports;
    
#if defined(__arm__)
    constexpr bool is_arm32 = true;
#else
    constexpr bool is_arm32 = false;
#endif

    uintptr_t lower_bound_arm64 = 0x2000000;
    uintptr_t upper_bound_arm64 = 0x3000000;
    uintptr_t lower_bound_arm32 = 0x1000000;
    uintptr_t upper_bound_arm32 = 0x3000000;

    uintptr_t lower_bound = is_arm32 ? lower_bound_arm32 : lower_bound_arm64;
    uintptr_t upper_bound = is_arm32 ? upper_bound_arm32 : upper_bound_arm64;
    
    bool has_trampoline_reloc = false;
    bool has_bss_start = false;
    bool has_bss_end = false;
    bool bss_payload_found = false;

    uint32_t bss_size_from_tail = 0;
    size_t bss_data_start_offset = 0;
    std::vector<uint8_t> bss_tail_data;

    std::vector<uint8_t> buffer(object_data.begin(), object_data.end());

    // state
    size_t offset = 0;

    offset += 5; // +5 = programname
    auto program_name = read_string(buffer, offset);
    log_D("Program name: %s", program_name.data());

    log_D("buffer size: %d", buffer.size());

    constexpr int _TIMEOUT = 5;
    int timeout_counter = 0;
    
    while (offset < buffer.size())
    {
        uint8_t opcode = read_value<uint8_t>(buffer, offset);

        if (opcode == AP_INVALID)
        {
            continue;
        }

        if (opcode == AP_OBJECTADDRESS)
        {
            // 66 31 00 00 | opcode flags pad pad
            int64_t encoded_address = read_value<int64_t>(buffer, offset);
            log_D("Encoded address: 0x%llx", encoded_address);
            
            uint8_t flags = encoded_address & 0xFF;
            if (flags < 0x30)
            {
                // offset back to 7
                offset -= 7; // we dont want to accidentally skip over a real one
                continue;
            }

            int64_t decoded_address = decode_address(encoded_address);
            uintptr_t decoded_u = static_cast<uintptr_t>(decoded_address); // keep unsigned
            bool in_code_range = is_in_range(decoded_u, lower_bound, upper_bound);

            uint32_t original_blob_size = 0;
            bool has_original_blob_size = false;
            uintptr_t onload_address_candidate = 0;
            bool has_onload_address_candidate = false;

            uint64_t primary_decoded_value = static_cast<uint64_t>(decode_address(encoded_address));
            if (flags == 0x31)
            {
                original_blob_size = static_cast<uint32_t>(primary_decoded_value & 0xFFFFFFFFULL);
                has_original_blob_size = true;
            }
            if (flags == 0x33)
            {
                bss_info.trampoline_reloc = static_cast<uintptr_t>(primary_decoded_value);
                has_trampoline_reloc = true;
            }
            if (flags == 0x34)
            {
                bss_info.start = static_cast<uintptr_t>(primary_decoded_value);
                has_bss_start = true;
            }
            if (flags == 0x37)
            {
                bss_info.end = static_cast<uintptr_t>(primary_decoded_value);
                has_bss_end = true;
            }
            if (flags == 0x38)
            {
                onload_address_candidate = static_cast<uintptr_t>(primary_decoded_value);
                has_onload_address_candidate = true;
            }

            if (!in_code_range)
            {
                if (has_bss_start && has_bss_end && bss_info.end >= bss_info.start && !bss_payload_found)
                {
                    uintptr_t bss_span = bss_info.end - bss_info.start;
                    if (bss_span != 0)
                    {
                        size_t probe_offset = offset;
                        while (probe_offset < buffer.size() && buffer[probe_offset] != AP_OBJECTSIZE)
                        {
                            ++probe_offset;
                        }

                        uint32_t probe_size = 0;
                        size_t probe_data_offset = 0;
                        if (probe_offset < buffer.size() && parse_objectsize_block(buffer, probe_offset, probe_size, probe_data_offset))
                        {
                            if (static_cast<uintptr_t>(probe_size) == bss_span)
                            {
                                bss_size_from_tail = probe_size;
                                bss_data_start_offset = probe_data_offset;

                                if ((bss_data_start_offset + bss_size_from_tail) < buffer.size())
                                {
                                    bool looks_like_padding = (buffer[bss_data_start_offset] == 0x00) && (buffer[bss_data_start_offset + 1] != 0x00);
                                    if (looks_like_padding)
                                    {
                                        ++bss_data_start_offset;
                                        log_W("bss payload realigned by +1 byte, new data start offset: 0x%llx", bss_data_start_offset);
                                    }
                                }

                                bss_tail_data = std::vector<uint8_t>(
                                    buffer.begin() + bss_data_start_offset,
                                    buffer.begin() + bss_data_start_offset + bss_size_from_tail
                                );
                                bss_payload_found = true;
                                log_I("bss payload matched by span, size: 0x%llx, data start offset: 0x%llx", bss_size_from_tail, bss_data_start_offset);
                            }
                        }
                    }
                }
                continue;
            }

            size_t header_cursor = offset;
            size_t objectsize_block_offset = buffer.size();
            while (header_cursor < buffer.size())
            {
                uint8_t header_opcode = buffer[header_cursor++];
                if (header_opcode == AP_OBJECTSIZE)
                {
                    objectsize_block_offset = header_cursor - 1;
                    break;
                }

                if (header_opcode == AP_OBJECTADDRESS)
                {
                    if (header_cursor + sizeof(int64_t) > buffer.size())
                    {
                        break;
                    }

                    size_t metadata_offset = header_cursor;
                    int64_t metadata_encoded = read_value<int64_t>(buffer, metadata_offset);
                    header_cursor = metadata_offset;

                    uint8_t metadata_flags = metadata_encoded & 0xFF;
                    uint64_t metadata_value = static_cast<uint64_t>(decode_address(metadata_encoded));
                    if (!has_original_blob_size && metadata_flags == 0x31)
                    {
                        original_blob_size = static_cast<uint32_t>(metadata_value & 0xFFFFFFFFULL);
                        has_original_blob_size = true;
                    }

                    if (!has_trampoline_reloc && metadata_flags == 0x33)
                    {
                        bss_info.trampoline_reloc = static_cast<uintptr_t>(metadata_value);
                        has_trampoline_reloc = true;
                    }

                    if (!has_bss_start && metadata_flags == 0x34)
                    {
                        bss_info.start = static_cast<uintptr_t>(metadata_value);
                        has_bss_start = true;
                    }

                    if (!has_bss_end && metadata_flags == 0x37)
                    {
                        bss_info.end = static_cast<uintptr_t>(metadata_value);
                        has_bss_end = true;
                    }

                    if (metadata_flags == 0x38)
                    {
                        onload_address_candidate = static_cast<uintptr_t>(metadata_value);
                        has_onload_address_candidate = true;
                    }
                }
            }

            if (objectsize_block_offset == buffer.size())
            {
                log_E("failed to locate AP_OBJECTSIZE for decoded address: 0x%llx", decoded_address);
                break;
            }

            uint32_t object_size = 0;
            size_t data_offset = 0;
            if (!parse_objectsize_block(buffer, objectsize_block_offset, object_size, data_offset))
            {
                log_E("failed to parse AP_OBJECTSIZE block at offset: 0x%llx", objectsize_block_offset);
                break;
            }

            if (has_bss_start && has_bss_end && bss_info.end >= bss_info.start)
            {
                uintptr_t bss_span = bss_info.end - bss_info.start;
                if (!bss_payload_found && bss_span != 0 && static_cast<uintptr_t>(object_size) == bss_span)
                {
                    bss_size_from_tail = object_size;
                    bss_data_start_offset = data_offset;

                    if ((bss_data_start_offset + bss_size_from_tail) < buffer.size())
                    {
                        bool looks_like_padding = (buffer[bss_data_start_offset] == 0x00) && (buffer[bss_data_start_offset + 1] != 0x00);
                        if (looks_like_padding)
                        {
                            ++bss_data_start_offset;
                            log_W("bss payload realigned by +1 byte, new data start offset: 0x%llx", bss_data_start_offset);
                        }
                    }

                    bss_tail_data = std::vector<uint8_t>(
                        buffer.begin() + bss_data_start_offset,
                        buffer.begin() + bss_data_start_offset + bss_size_from_tail
                    );
                    bss_payload_found = true;
                    log_I("bss payload matched by span, size: 0x%llx, data start offset: 0x%llx", bss_size_from_tail, bss_data_start_offset);
                }
            }

            // next N bytes are the object data
            std::vector<uint8_t> object_data(buffer.begin() + data_offset, buffer.begin() + data_offset + object_size);
            offset = data_offset + object_size;

            uintptr_t effective_address = decoded_u;
            
            // Only arm32 needs the 0x38 override
            if (is_arm32 && has_onload_address_candidate && is_in_range(onload_address_candidate, lower_bound, upper_bound))
            {
                effective_address = onload_address_candidate;
            }

            if (!is_in_range(effective_address, lower_bound, upper_bound))
            {
                continue;
            }

            log_D("offset: 0x%llx, decoded address: 0x%llx, object size: 0x%x, original blob size: 0x%x", offset, effective_address, object_size, original_blob_size);

            // file: 0xaddy_arm64.bin
            // std::stringstream ss;
            // ss << "0x" << std::hex << decoded_address << "_" << input_arch << ".bin";
            // std::string filename = ss.str();

            object_metadata_t metadata;
            metadata.decoded_address = effective_address;
            metadata.object_size = object_size;
            metadata.original_blob_size = original_blob_size;
            metadata.data_offset = data_offset;
            metadata.has_original_blob_size = has_original_blob_size;
            metadata.code = object_data;

            object_map[effective_address] = metadata;

            // we did get something, were fine.
            timeout_counter -= 1;
        }
        
        // if we keep reading but dont get any objects, we might be in a bad state, so we timeout after a while
        // mainly because i cba to make a proper exit for android, it was working fine in windows
        timeout_counter += 1;
    }


    // bss append
    object_metadata_t bss_metadata;
    bss_metadata.decoded_address = bss_info.start;
    bss_metadata.object_size = bss_size_from_tail;
    bss_metadata.original_blob_size = bss_size_from_tail;
    bss_metadata.data_offset = bss_data_start_offset;
    bss_metadata.has_original_blob_size = true;
    bss_metadata.code = bss_tail_data;
    object_map[bss_info.start] = bss_metadata;

    if (!bss_payload_found)
    {
        log_W("bss payload not matched by span; start=0x%llx end=0x%llx", bss_info.start, bss_info.end);
    }

    log_I("return");
    return;
}