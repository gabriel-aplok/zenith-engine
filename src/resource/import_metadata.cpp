#include "resource/import_metadata.hpp"

#include <cstdio>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

namespace Zenith
{
    namespace
    {
        template <typename Writer>
        void writeStringArray(Writer &writer, const std::vector<std::string> &values)
        {
            writer.StartArray();
            for (const auto &value : values)
            {
                writer.String(value.c_str(), static_cast<rapidjson::SizeType>(value.size()));
            }
            writer.EndArray();
        }
    } // namespace

    bool readImportMetadata(const std::filesystem::path &path, ImportMetadata &metadata)
    {
        std::FILE *file = nullptr;
#if defined(_WIN32)
        _wfopen_s(&file, path.wstring().c_str(), L"rb");
#else
        file = std::fopen(path.string().c_str(), "rb");
#endif
        if (!file)
        {
            return false;
        }

        char buffer[8192];
        rapidjson::FileReadStream stream(file, buffer, sizeof(buffer));
        rapidjson::Document document;
        document.ParseStream(stream);
        std::fclose(file);

        if (document.HasParseError() || !document.IsObject())
        {
            return false;
        }

        auto readString = [&document](const char *name) -> std::string
        {
            if (!document.HasMember(name) || !document[name].IsString())
            {
                return {};
            }
            return document[name].GetString();
        };

        metadata.virtualPath = readString("virtualPath");
        metadata.sourcePath = readString("sourcePath");
        metadata.bakedPath = readString("bakedPath");
        metadata.importerName = readString("importerName");
        metadata.settingsJson = readString("settingsJson");

        if (document.HasMember("importerVersion") && document["importerVersion"].IsUint())
        {
            metadata.importerVersion = document["importerVersion"].GetUint();
        }

        if (document.HasMember("sourceHash") && document["sourceHash"].IsUint64())
        {
            metadata.sourceHash = document["sourceHash"].GetUint64();
        }

        if (document.HasMember("importedAtUtc") && document["importedAtUtc"].IsInt64())
        {
            metadata.importedAtUtc = document["importedAtUtc"].GetInt64();
        }

        metadata.dependencies.clear();
        if (document.HasMember("dependencies") && document["dependencies"].IsArray())
        {
            for (const auto &entry : document["dependencies"].GetArray())
            {
                if (entry.IsString())
                {
                    metadata.dependencies.emplace_back(entry.GetString(), entry.GetStringLength());
                }
            }
        }

        return true;
    }

    bool writeImportMetadata(const std::filesystem::path &path, const ImportMetadata &metadata)
    {
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);

        std::FILE *file = nullptr;
#if defined(_WIN32)
        _wfopen_s(&file, path.wstring().c_str(), L"wb");
#else
        file = std::fopen(path.string().c_str(), "wb");
#endif
        if (!file)
        {
            return false;
        }

        char buffer[8192];
        rapidjson::FileWriteStream stream(file, buffer, sizeof(buffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(stream);
        writer.SetIndent(' ', 2);

        writer.StartObject();
        writer.Key("virtualPath");
        writer.String(metadata.virtualPath.c_str(), static_cast<rapidjson::SizeType>(metadata.virtualPath.size()));
        writer.Key("sourcePath");
        writer.String(metadata.sourcePath.c_str(), static_cast<rapidjson::SizeType>(metadata.sourcePath.size()));
        writer.Key("bakedPath");
        writer.String(metadata.bakedPath.c_str(), static_cast<rapidjson::SizeType>(metadata.bakedPath.size()));
        writer.Key("importerName");
        writer.String(metadata.importerName.c_str(), static_cast<rapidjson::SizeType>(metadata.importerName.size()));
        writer.Key("importerVersion");
        writer.Uint(metadata.importerVersion);
        writer.Key("sourceHash");
        writer.Uint64(metadata.sourceHash);
        writer.Key("settingsJson");
        writer.String(metadata.settingsJson.c_str(), static_cast<rapidjson::SizeType>(metadata.settingsJson.size()));
        writer.Key("dependencies");
        writeStringArray(writer, metadata.dependencies);
        writer.Key("importedAtUtc");
        writer.Int64(metadata.importedAtUtc);
        writer.EndObject();
        std::fclose(file);
        return true;
    }
} // namespace Zenith
