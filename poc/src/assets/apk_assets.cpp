#include "assets.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <sstream>
#include <vector>
#include <zlib.h>

namespace
{
    std::string APK_PATH;
    std::string PACKAGE_NAME;
    std::mutex APK_INIT_MUTEX;

    struct apk_info
    {
        std::string path;
        std::string package_name;
    };

    std::string package_from_path(const std::string &path)
    {
        // /data/app/[~~install-token/]<package>-<install-token>/base.apk
        const auto end = path.find_last_of('/');
        if (end == std::string::npos || end == 0)
            return {};

        const auto start = path.find_last_of('/', end - 1);
        if (start == std::string::npos)
            return {};

        std::string package = path.substr(start + 1, end - start - 1);
        package = package.substr(0, package.find('-'));

        bool segment_start = true;
        for (unsigned char c : package)
        {
            if (c == '.' && !segment_start)
            {
                segment_start = true;
                continue;
            }
            const bool letter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
            if (!letter && (segment_start || (c != '_' && (c < '0' || c > '9'))))
                return {};
            segment_start = false;
        }

        if (segment_start || (package.find('.') == std::string::npos && package != "android"))
            return {};

        return package;
    }

    apk_info find_apk(std::istream &maps, const std::string &process_name)
    {
        const std::string process_package = process_name.substr(0, process_name.find(':'));
        std::vector<apk_info> candidates;
        std::string line;

        while (std::getline(maps, line))
        {
            std::istringstream fields(line);
            std::string field, path;
            if (!(fields >> field >> field >> field >> field >> field))
                continue;

            std::getline(fields >> std::ws, path);
            if (path.size() < 9 || path.front() != '/' ||
                path.compare(path.size() - 9, 9, "/base.apk") != 0)
                continue;

            apk_info candidate{path, package_from_path(path)};
            if (!candidate.package_name.empty() && candidate.package_name == process_package)
                return candidate;

            if (std::none_of(candidates.begin(), candidates.end(),
                             [&](const apk_info &info) { return info.path == path; }))
                candidates.push_back(std::move(candidate));
        }

        return candidates.size() == 1 ? candidates.front() : apk_info{};
    }

    std::uint16_t u16(const unsigned char *p)
    {
        return static_cast<std::uint16_t>(p[0] | (p[1] << 8));
    }

    std::uint32_t u32(const unsigned char *p)
    {
        return u16(p) | (static_cast<std::uint32_t>(u16(p + 2)) << 16);
    }

    bool read_at(std::ifstream &file, std::uint64_t limit, std::uint64_t offset,
                 void *data, std::size_t size)
    {
        if (offset > limit || size > limit - offset)
            return false;
        file.seekg(static_cast<std::streamoff>(offset));
        return static_cast<bool>(file.read(static_cast<char *>(data), size));
    }

    std::string read_zip_asset(const std::string &path, const std::string &name)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
            return {};
        
            const auto end = file.tellg();
        if (end < std::streamoff(22))
            return {};

        const auto file_size = static_cast<std::uint64_t>(end);
        
        // end record is followed by at most 65535 comment bytes.
        const auto tail_size = static_cast<std::size_t>(std::min<std::uint64_t>(file_size, 22 + 65535));

        std::vector<unsigned char> tail(tail_size);
        if (!read_at(file, file_size, file_size - tail_size, tail.data(), tail.size()))
            return {};

        std::size_t eocd = tail.size() - 22;
        while (u32(tail.data() + eocd) != 0x06054b50 ||
               eocd + 22 + u16(tail.data() + eocd + 20) != tail.size() ||
               std::uint64_t(u32(tail.data() + eocd + 12)) + u32(tail.data() + eocd + 16) !=
                   file_size - tail_size + eocd)
        {
            if (eocd == 0)
                return {};
            --eocd;
        }
        const unsigned char *record = tail.data() + eocd;
        const auto entries = u16(record + 10);
        const std::uint64_t directory_size = u32(record + 12);
        const std::uint64_t directory_start = u32(record + 16);
        const std::uint64_t directory_end = directory_start + directory_size;
        
        // zip64?
        if (u16(record + 4) != 0 || u16(record + 6) != 0 ||
            u16(record + 8) != entries || entries == 0xffff ||
            directory_size == 0xffffffff || directory_start == 0xffffffff ||
            directory_end != file_size - tail_size + eocd)
            return {};

        std::uint64_t cursor = directory_start;
        for (std::uint32_t i = 0; i < entries; ++i)
        {
            std::array<unsigned char, 46> header{};
            if (!read_at(file, directory_end, cursor, header.data(), header.size()) ||
                u32(header.data()) != 0x02014b50)
                return {};

            const auto flags = u16(header.data() + 8);
            const auto method = u16(header.data() + 10);
            const auto expected_crc = u32(header.data() + 16);
            const auto compressed_size = u32(header.data() + 20);
            const auto size = u32(header.data() + 24);
            const auto name_size = u16(header.data() + 28);
            const auto extra_size = u16(header.data() + 30);
            const auto comment_size = u16(header.data() + 32);
            const auto local_offset = u32(header.data() + 42);
            const std::uint64_t next = cursor + header.size() + name_size + extra_size + comment_size;
            if (next > directory_end)
                return {};

            std::string entry_name(name_size, '\0');
            if (!read_at(file, directory_end, cursor + header.size(), entry_name.data(), name_size))
                return {};

            cursor = next;
            if (entry_name != name)
                continue;

            if ((flags & 0x0041) != 0 || (method != 0 && method != 8) ||
                u16(header.data() + 34) != 0 || compressed_size == 0xffffffff ||
                size == 0xffffffff || local_offset == 0xffffffff)
                return {};

            std::array<unsigned char, 30> local{};
            if (!read_at(file, directory_start, local_offset, local.data(), local.size()) ||
                u32(local.data()) != 0x04034b50 || u16(local.data() + 6) != flags ||
                u16(local.data() + 8) != method || u16(local.data() + 26) != name_size)
                return {};

            std::string local_name(name_size, '\0');
            if (!read_at(file, directory_start, std::uint64_t(local_offset) + local.size(),
                         local_name.data(), name_size) || local_name != entry_name)
                return {};

            const std::uint64_t data_offset = std::uint64_t(local_offset) + local.size() +
                                              name_size + u16(local.data() + 28);
            if (data_offset > directory_start || compressed_size > directory_start - data_offset)
                return {};

            std::string result;
            if (method == 0)
            {
                if (compressed_size != size)
                    return {};
                result.resize(size);
                if (!read_at(file, directory_start, data_offset, result.data(), result.size()))
                    return {};
            }
            else
            {
                z_stream stream{};
                if (inflateInit2(&stream, -MAX_WBITS) != Z_OK)
                    return {};

                struct inflate_cleanup
                {
                    z_stream *stream;
                    ~inflate_cleanup() { inflateEnd(stream); }
                } cleanup{&stream};
                std::array<unsigned char, 32768> input{}, output{};
                std::uint64_t consumed = 0;
                int status = Z_OK;
                while (status != Z_STREAM_END)
                {
                    if (stream.avail_in == 0 && consumed < compressed_size)
                    {
                        const auto count = static_cast<std::size_t>(
                            std::min<std::uint64_t>(input.size(), compressed_size - consumed));
                        if (!read_at(file, directory_start, data_offset + consumed, input.data(), count))
                            return {};

                        consumed += count;
                        stream.next_in = input.data();
                        stream.avail_in = static_cast<uInt>(count);
                    }
                    stream.next_out = output.data();
                    stream.avail_out = static_cast<uInt>(output.size());
                    status = inflate(&stream, Z_NO_FLUSH);
                    const auto count = output.size() - stream.avail_out;
                    if ((status != Z_OK && status != Z_STREAM_END) || count > size - result.size())
                        return {};

                    result.append(reinterpret_cast<const char *>(output.data()), count);
                }
                if (stream.total_in != compressed_size || result.size() != size)
                    return {};
            }
            if (crc32(0, reinterpret_cast<const Bytef *>(result.data()),
                      static_cast<uInt>(result.size())) != expected_crc)
                return {};

            return result;
        }
        return {};
    }
}

bool assets::init()
{
    std::lock_guard<std::mutex> lock(APK_INIT_MUTEX);
    if (!APK_PATH.empty())
        return true;

    std::ifstream cmdline("/proc/self/cmdline", std::ios::binary);
    std::string process_name;
    std::getline(cmdline, process_name, '\0');
    std::ifstream maps("/proc/self/maps");
    auto info = find_apk(maps, process_name);
    if (info.path.empty())
        return false;
    PACKAGE_NAME = std::move(info.package_name);
    APK_PATH = std::move(info.path);
    return true;
}

std::string assets::get_apk_path()
{
    return init() ? APK_PATH : std::string{};
}

std::string assets::get_package_name()
{
    return init() ? PACKAGE_NAME : std::string{};
}

std::string assets::get_apk_asset(const std::string &name)
{
    if (name.empty() || !init())
        return {};
    const std::string entry = name.compare(0, 7, "assets/") == 0 ? name : "assets/" + name;
    return read_zip_asset(APK_PATH, entry);
}
