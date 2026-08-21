#include "archive.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <format>
#include <functional>
#include <unordered_map>

namespace c2p2::modules {

    struct FormatConfig {
        std::function<int(struct archive*)> set_format;
        std::function<int(struct archive*)> add_filter = nullptr;
    };

    static const std::unordered_map<std::string, FormatConfig>& get_format_map() {
    static const std::unordered_map<std::string, FormatConfig> map = {
        {"zip", {archive_write_set_format_zip}},
        {"7z", {archive_write_set_format_7zip}},
        {"tar", {archive_write_set_format_pax}},
        {"tar.ustar", {archive_write_set_format_ustar}},
        {"tar.gnu", {archive_write_set_format_gnutar}},
        {"cpio", {archive_write_set_format_cpio}},
        {"cpio.odc", {archive_write_set_format_cpio_odc}},
        {"iso", {archive_write_set_format_iso9660}},
        {"iso9660", {archive_write_set_format_iso9660}},
        {"xar", {archive_write_set_format_xar}},
        {"ar", {archive_write_set_format_ar_bsd}},
        {"ar.svr4", {archive_write_set_format_ar_svr4}},
        {"shar", {archive_write_set_format_shar}},
        {"shar.dump", {archive_write_set_format_shar_dump}},

        {"gz", {archive_write_set_format_raw, archive_write_add_filter_gzip}},
        {"gzip", {archive_write_set_format_raw, archive_write_add_filter_gzip}},
        {"bz2", {archive_write_set_format_raw, archive_write_add_filter_bzip2}},
        {"bzip2", {archive_write_set_format_raw, archive_write_add_filter_bzip2}},
        {"xz", {archive_write_set_format_raw, archive_write_add_filter_xz}},
        {"zstd", {archive_write_set_format_raw, archive_write_add_filter_zstd}},
        {"lz4", {archive_write_set_format_raw, archive_write_add_filter_lz4}},
        {"lzip", {archive_write_set_format_raw, archive_write_add_filter_lzip}},
        {"lzma", {archive_write_set_format_raw, archive_write_add_filter_lzma}},
        {"lzo", {archive_write_set_format_raw, archive_write_add_filter_lzop}},
        {"z", {archive_write_set_format_raw, archive_write_add_filter_compress}},
        {"uu", {archive_write_set_format_raw, archive_write_add_filter_uuencode}},
        {"uuencode", {archive_write_set_format_raw, archive_write_add_filter_uuencode}},
        {"grz", {archive_write_set_format_raw, archive_write_add_filter_grzip}},

        {"tar.gz", {archive_write_set_format_pax, archive_write_add_filter_gzip}},
        {"tgz", {archive_write_set_format_pax, archive_write_add_filter_gzip}},
        {"tar.bz2", {archive_write_set_format_pax, archive_write_add_filter_bzip2}},
        {"tbz2", {archive_write_set_format_pax, archive_write_add_filter_bzip2}},
        {"tar.xz", {archive_write_set_format_pax, archive_write_add_filter_xz}},
        {"txz", {archive_write_set_format_pax, archive_write_add_filter_xz}},
        {"tar.zst", {archive_write_set_format_pax, archive_write_add_filter_zstd}},
        {"tzst", {archive_write_set_format_pax, archive_write_add_filter_zstd}},
        {"tar.lz4", {archive_write_set_format_pax, archive_write_add_filter_lz4}},
        {"tlz4", {archive_write_set_format_pax, archive_write_add_filter_lz4}},
        {"tar.lz", {archive_write_set_format_pax, archive_write_add_filter_lzip}},
        {"tar.lzma", {archive_write_set_format_pax, archive_write_add_filter_lzma}},
        {"tlz", {archive_write_set_format_pax, archive_write_add_filter_lzma}},
        {"tar.lzo", {archive_write_set_format_pax, archive_write_add_filter_lzop}},
        {"tar.Z", {archive_write_set_format_pax, archive_write_add_filter_compress}},

        {"cpio.gz", {archive_write_set_format_cpio, archive_write_add_filter_gzip}},
        {"cpio.xz", {archive_write_set_format_cpio, archive_write_add_filter_xz}},
        {"cpio.zst", {archive_write_set_format_cpio, archive_write_add_filter_zstd}}
    };
    return map;
}

    static std::expected<DataBuffer, ModuleError> compress_buffer(
        const DataBuffer& input,
        const std::string& format,
        const std::string& filename
    ) {
        const auto& map = get_format_map();
        auto it = map.find(format);

        if (it == map.end()) {
            return std::unexpected(ModuleError{.message = "Unsupported format: " + format});
        }

        DataBuffer output;
        output.resize(input.size() + 1024);
        size_t used_size = 0;

        struct archive* a = archive_write_new();

        const auto& config = it->second;
        if (config.set_format) config.set_format(a);
        if (config.add_filter) config.add_filter(a);

        if (archive_write_open_memory(a, output.data(), output.size(), &used_size) != ARCHIVE_OK) {
            archive_write_free(a);
            return std::unexpected(ModuleError{.message = "Failed to initialize compression buffer"});
        }

        struct archive_entry* entry = archive_entry_new();
        archive_entry_set_pathname(entry, filename.c_str());
        archive_entry_set_size(entry, input.size());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);

        archive_write_header(a, entry);
        archive_write_data(a, input.data(), input.size());

        archive_entry_free(entry);
        archive_write_close(a);
        archive_write_free(a);

        output.resize(used_size);
        return output;
    }

    static std::expected<DataBuffer, ModuleError> decompress_buffer(const DataBuffer& input) {
        DataBuffer output;
        struct archive* a = archive_read_new();

        archive_read_support_format_all(a);
        archive_read_support_filter_all(a);
        archive_read_support_format_raw(a);

        if (archive_read_open_memory(a, input.data(), input.size()) != ARCHIVE_OK) {
            archive_read_free(a);
            return std::unexpected(ModuleError{.message = "Failed to open compressed archive"});
        }

        struct archive_entry* entry;
        if (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            size_t size = archive_entry_size(entry);

            if (size == 0) {
                char buffer[8192];
                la_ssize_t bytes_read;
                while ((bytes_read = archive_read_data(a, buffer, sizeof(buffer))) > 0) {
                    output.insert(output.end(),
                                  reinterpret_cast<std::byte*>(buffer),
                                  reinterpret_cast<std::byte*>(buffer + bytes_read));
                }
            } else {
                output.resize(size);
                archive_read_data(a, output.data(), size);
            }
        }

        archive_read_free(a);
        return output;
    }

    std::expected<DataBuffer, ModuleError> Archive::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        std::string format = "zip";
        if (const auto it = params.find("--format"); it != params.end()) {
            format = it->second;
        }

        std::string filename = "data.bin";
        if (const auto it = params.find("--filename"); it != params.end()) {
            filename = it->second;
        }

        if (action == "compress") {
            return compress_buffer(input, format, filename);
        } else if (action == "decompress") {
            return decompress_buffer(input);
        }

        return std::unexpected(ModuleError{
            .message = std::format("Unsupported action '{}' for Archive module", action)
        });
    }

}