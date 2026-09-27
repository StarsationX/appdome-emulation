#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace crypt
{

    struct config
    {
        std::size_t payload_offset_off = 8;
        std::size_t counter_size_off = 28;
        std::size_t counter_off = 32;
    };

    std::vector<std::uint8_t> decrypt_ctr_payload(
        const std::vector<std::uint8_t> &data,
        const std::array<std::uint8_t, 16> &counter);

    std::vector<std::uint8_t> decrypt_blob_data(
        const std::vector<std::uint8_t> &enc,
        const config &cfg = config{},
        std::size_t max_len = static_cast<std::size_t>(-1));

    std::uint32_t read_u32_le(const std::uint8_t *p);
    std::uint32_t read_u32_be(const std::uint8_t *p);
    std::uint32_t read_u32_le(const std::vector<std::uint8_t> &b, std::size_t off);
    std::uint32_t read_u32_be(const std::vector<std::uint8_t> &b, std::size_t off);

} // namespace crypt
