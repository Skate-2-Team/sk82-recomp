#include "loader.h"

namespace Loader
{
    // From Unleashed Recompiled
    bool Init(const std::string &p_xexPath)
    {
        std::ifstream file(p_xexPath, std::ios::binary);
        if (!file.is_open())
        {
            Log::Error("Loader", "Failed to open XEX file");
            return false;
        }

        // load file into vector
        std::vector<uint8_t> loadResult((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        auto *header = reinterpret_cast<const Xex2Header *>(loadResult.data());
        auto *security = reinterpret_cast<const Xex2SecurityInfo *>(loadResult.data() + header->securityOffset);
        auto *fileFormatInfo = reinterpret_cast<const Xex2OptFileFormatInfo *>(getOptHeaderPtr(loadResult.data(), XEX_HEADER_FILE_FORMAT_INFO));

        auto entry = *reinterpret_cast<const uint32_t *>(getOptHeaderPtr(loadResult.data(), XEX_HEADER_ENTRY_POINT));
        ByteSwapInplace(entry);

        auto srcData = loadResult.data() + header->headerSize;
        auto destData = reinterpret_cast<uint8_t *>(Memory::Translate(security->loadAddress));

        Log::Info("Loader", "Load Address: 0x", reinterpret_cast<void *>(security->loadAddress.get()));
        Log::Info("Loader", "Entry Point: 0x", reinterpret_cast<void *>(entry));

        // Translate them and print them
        Log::Info("Loader", "Translated Load Address: 0x", Memory::Translate(entry));
        Log::Info("Loader", "Translated Entry Point: 0x", Memory::Translate(entry));

        // print compression type
        Log::Info("Loader", "Compression Type: ", GetCompressionString(fileFormatInfo->compressionType));

        if (fileFormatInfo->compressionType == XEX_COMPRESSION_NONE)
        {
            Log::Info("Loader", "Copying xex data raw.");

            memcpy(destData, srcData, security->imageSize);
        }
        else if (fileFormatInfo->compressionType == XEX_COMPRESSION_BASIC)
        {
            Log::Info("Loader", "Decompressing xex data.");

            // decompress

            auto *blocks = reinterpret_cast<const Xex2FileBasicCompressionBlock *>(fileFormatInfo + 1);
            const size_t numBlocks = (fileFormatInfo->infoSize / sizeof(Xex2FileBasicCompressionInfo)) - 1;

            for (size_t i = 0; i < numBlocks; i++)
            {
                memcpy(destData, srcData, blocks[i].dataSize);

                srcData += blocks[i].dataSize;
                destData += blocks[i].dataSize;

                memset(destData, 0, blocks[i].zeroSize);
                destData += blocks[i].zeroSize;
            }
        }

        Log::Info("Loader", "XEX loaded into memory.");

        m_startAddress = entry;

        return true;
    }
}
