#pragma once

#include <memory>
#include <cstdint>
#include <cassert>

template <typename T>
inline T ByteSwap(T value)
{
    if constexpr (sizeof(T) == 1)
        return value;
    else if constexpr (sizeof(T) == 2)
        return static_cast<T>(__builtin_bswap16(static_cast<uint16_t>(value)));
    else if constexpr (sizeof(T) == 4)
        return static_cast<T>(__builtin_bswap32(static_cast<uint32_t>(value)));
    else if constexpr (sizeof(T) == 8)
        return static_cast<T>(__builtin_bswap64(static_cast<uint64_t>(value)));

    assert(false && "Unexpected byte size.");
    return value;
}

template <typename T>
inline void ByteSwapInplace(T &value)
{
    value = ByteSwap(value);
}

template <typename T>
struct be
{
    T value;

    be() : value(0)
    {
    }

    be(const T v)
    {
        set(v);
    }

    static T byteswap(T value)
    {
        if constexpr (std::is_same_v<T, double>)
        {
            const uint64_t swapped = ByteSwap(*reinterpret_cast<uint64_t *>(&value));
            return *reinterpret_cast<const T *>(&swapped);
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            const uint32_t swapped = ByteSwap(*reinterpret_cast<uint32_t *>(&value));
            return *reinterpret_cast<const T *>(&swapped);
        }
        else if constexpr (std::is_enum_v<T>)
        {
            const std::underlying_type_t<T> swapped = ByteSwap(*reinterpret_cast<std::underlying_type_t<T> *>(&value));
            return *reinterpret_cast<const T *>(&swapped);
        }
        else
        {
            return ByteSwap(value);
        }
    }

    void set(const T v)
    {
        value = byteswap(v);
    }

    T get() const
    {
        return byteswap(value);
    }

    be &operator|(T value)
    {
        set(get() | value);
        return *this;
    }

    be &operator&(T value)
    {
        set(get() & value);
        return *this;
    }

    operator T() const
    {
        return get();
    }

    be &operator=(T v)
    {
        set(v);
        return *this;
    }
};

enum Xex2CompressionType
{
    XEX_COMPRESSION_NONE = 0,
    XEX_COMPRESSION_BASIC = 1,
    XEX_COMPRESSION_NORMAL = 2,
    XEX_COMPRESSION_DELTA = 3,
};

enum Xex2HeaderKeys
{
    XEX_HEADER_RESOURCE_INFO = 0x000002FF,
    XEX_HEADER_FILE_FORMAT_INFO = 0x000003FF,
    XEX_HEADER_DELTA_PATCH_DESCRIPTOR = 0x000005FF,
    XEX_HEADER_BASE_REFERENCE = 0x00000405,
    XEX_HEADER_BOUNDING_PATH = 0x000080FF,
    XEX_HEADER_DEVICE_ID = 0x00008105,
    XEX_HEADER_ORIGINAL_BASE_ADDRESS = 0x00010001,
    XEX_HEADER_ENTRY_POINT = 0x00010100,
    XEX_HEADER_IMAGE_BASE_ADDRESS = 0x00010201,
    XEX_HEADER_IMPORT_LIBRARIES = 0x000103FF,
    XEX_HEADER_CHECKSUM_TIMESTAMP = 0x00018002,
    XEX_HEADER_ENABLED_FOR_CALLCAP = 0x00018102,
    XEX_HEADER_ENABLED_FOR_FASTCAP = 0x00018200,
    XEX_HEADER_ORIGINAL_PE_NAME = 0x000183FF,
    XEX_HEADER_STATIC_LIBRARIES = 0x000200FF,
    XEX_HEADER_TLS_INFO = 0x00020104,
    XEX_HEADER_DEFAULT_STACK_SIZE = 0x00020200,
    XEX_HEADER_DEFAULT_FILESYSTEM_CACHE_SIZE = 0x00020301,
    XEX_HEADER_DEFAULT_HEAP_SIZE = 0x00020401,
    XEX_HEADER_PAGE_HEAP_SIZE_AND_FLAGS = 0x00028002,
    XEX_HEADER_SYSTEM_FLAGS = 0x00030000,
    XEX_HEADER_EXECUTION_INFO = 0x00040006,
    XEX_HEADER_TITLE_WORKSPACE_SIZE = 0x00040201,
    XEX_HEADER_GAME_RATINGS = 0x00040310,
    XEX_HEADER_LAN_KEY = 0x00040404,
    XEX_HEADER_XBOX360_LOGO = 0x000405FF,
    XEX_HEADER_MULTIDISC_MEDIA_IDS = 0x000406FF,
    XEX_HEADER_ALTERNATE_TITLE_IDS = 0x000407FF,
    XEX_HEADER_ADDITIONAL_TITLE_MEMORY = 0x00040801,
    XEX_HEADER_EXPORTS_BY_NAME = 0x00E10402,
};

struct Xex2FileBasicCompressionBlock
{
    be<uint32_t> dataSize;
    be<uint32_t> zeroSize;
};

struct Xex2FileBasicCompressionInfo
{
    Xex2FileBasicCompressionBlock firstBlock;
};

struct Xex2OptFileFormatInfo
{
    be<uint32_t> infoSize;
    be<uint16_t> encryptionType;
    be<uint16_t> compressionType;
};

struct Xex2OptHeader
{
    be<uint32_t> key;

    union
    {
        be<uint32_t> value;
        be<uint32_t> offset;
    };
};

struct Xex2SecurityInfo
{
    be<uint32_t> headerSize;
    be<uint32_t> imageSize;
    char rsaSignature[0x100];
    be<uint32_t> unknown;
    be<uint32_t> imageFlags;
    be<uint32_t> loadAddress;
    char sectionDigest[0x14];
    be<uint32_t> importTableCount;
    char importTableDigest[0x14];
    char xgd2MediaId[0x10];
    char aesKey[0x10];
    be<uint32_t> exportTable;
    char headerDigest[0x14];
    be<uint32_t> region;
    be<uint32_t> allowedMediaTypes;
    be<uint32_t> pageDescriptorCount;
};

struct Xex2Header
{
    be<uint32_t> magic;
    be<uint32_t> moduleFlags;
    be<uint32_t> headerSize;
    be<uint32_t> reserved;
    be<uint32_t> securityOffset;
    be<uint32_t> headerCount;
};

inline const char *GetCompressionString(uint16_t compressionTypeNum)
{
    const char *compressionType = "Unknown";

    switch (compressionTypeNum)
    {
    case XEX_COMPRESSION_NONE:
        compressionType = "None";
        break;
    case XEX_COMPRESSION_BASIC:
        compressionType = "Basic";
        break;
    case XEX_COMPRESSION_NORMAL:
        compressionType = "Normal";
        break;
    case XEX_COMPRESSION_DELTA:
        compressionType = "Delta";
        break;
    }

    return compressionType;
}

inline const void *getOptHeaderPtr(const uint8_t *moduleBytes, uint32_t headerKey)
{
    const Xex2Header *xex2Header = (const Xex2Header *)(moduleBytes);
    for (uint32_t i = 0; i < xex2Header->headerCount; i++)
    {
        const Xex2OptHeader &optHeader = ((const Xex2OptHeader *)(xex2Header + 1))[i];
        if (optHeader.key == headerKey)
        {
            if ((headerKey & 0xFF) == 0)
            {
                return &optHeader.value;
            }
            else
            {
                return &moduleBytes[optHeader.offset];
            }
        }
    }

    return nullptr;
}