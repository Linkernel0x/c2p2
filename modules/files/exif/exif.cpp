#include "exif.hpp"
#include <exiv2/exiv2.hpp>
#include <exiv2/easyaccess.hpp>
#include <exiv2/basicio.hpp>
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include <nlohmann/json.hpp>
#include <format>

namespace c2p2::modules {

    static DataBuffer string_to_buffer(const std::string& str) {
        DataBuffer buf;
        buf.reserve(str.size());
        for (char c : str) {
            buf.push_back(static_cast<std::byte>(c));
        }
        return buf;
    }

    static std::expected<DataBuffer, ModuleError> get_metadata(const DataBuffer& input) {
        try {
            const auto image = Exiv2::ImageFactory::open(
                reinterpret_cast<const Exiv2::byte*>(input.data()),
                input.size()
            );

            if (!image) {
                return std::unexpected(ModuleError{.message = "Image format not supported"});
            }

            image->readMetadata();
            Exiv2::ExifData& exifData = image->exifData();

            nlohmann::json json_out = nlohmann::json::object();

            for (const auto & md : exifData) {
                json_out[md.key()] = md.value().toString();
            }

            return string_to_buffer(json_out.dump(4));

        } catch (const std::exception& e) {
            return std::unexpected(ModuleError{.message = std::format("Error reading EXIF: {}", e.what())});
        }
    }

    static std::expected<DataBuffer, ModuleError> remove_metadata(const DataBuffer& input) {
        try {
            const auto image = Exiv2::ImageFactory::open(
                reinterpret_cast<const Exiv2::byte*>(input.data()),
                input.size()
            );

            if (!image) {
                return std::unexpected(ModuleError{.message = "Image format not supported"});
            }

            image->clearExifData();
            image->clearIptcData();
            image->clearXmpData();
            image->writeMetadata();

            // read the image back into databuffer
            Exiv2::BasicIo& io = image->io();
            DataBuffer output(io.size());
            io.seek(0, Exiv2::BasicIo::beg);
            io.read(reinterpret_cast<Exiv2::byte*>(output.data()), io.size());

            return output;

        } catch (const std::exception& e) {
            return std::unexpected(ModuleError{.message = std::format("Error removing EXIF: {}", e.what())});
        }
    }

    static std::expected<DataBuffer, ModuleError> set_metadata(
        const DataBuffer& input,
        const std::string& key,
        const std::string& value
    ) {
        if (key.empty()) {
            return std::unexpected(ModuleError{.message = "Missing --key parameter for modify action"});
        }

        try {
            const auto image = Exiv2::ImageFactory::open(
                reinterpret_cast<const Exiv2::byte*>(input.data()),
                input.size()
            );

            if (!image) {
                return std::unexpected(ModuleError{.message = "Image format not supported"});
            }

            image->readMetadata();
            Exiv2::ExifData& exifData = image->exifData();

            exifData[key] = value;
            image->setExifData(exifData);
            image->writeMetadata();

            Exiv2::BasicIo& io = image->io();
            DataBuffer output(io.size());
            io.seek(0, Exiv2::BasicIo::beg);
            io.read(reinterpret_cast<Exiv2::byte*>(output.data()), io.size());

            return output;

        } catch (const std::exception& e) {
            return std::unexpected(ModuleError{.message = std::format("Error modifying EXIF: {}", e.what())});
        }
    }

    std::expected<DataBuffer, ModuleError> Exif::execute(
        const std::string& action,
        const DataBuffer& input,
        const ParamsMap& params
    ) const {
        if (action == "get") {
            return get_metadata(input);
        }
        if (action == "remove") {
            return remove_metadata(input);
        }
        if (action == "set") {
            const std::string key = params.contains("--key") ? params.at("--key") : "";
            const std::string value = params.contains("--value") ? params.at("--value") : "";
            return set_metadata(input, key, value);
        }

        return std::unexpected(ModuleError{
            .message = std::format("Unknown action '{}' for the Exif module", action)
        });
    }

}