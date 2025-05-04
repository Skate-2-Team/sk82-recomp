#pragma once

#include <d3d9.h>
#include <fstream>
#include <map>

#include "log.h"
#include "dds_context.h"
#include "d3d_context.h"
#include "tga_context.h"
#include "xex.h"

/*
I wrote this class to convert the RX2 texture data straight to a D3D9 texture.
*/

namespace XTexHelperTypes
{
    constexpr const uint32_t RW4_MAGIC = 878137993;

    enum ObjectType
    {
        RWGOBJECTTYPE_TEXTURE = '\x00\x02\x00\xE8',
        RWOBJECTTYPE_BASERESOURCE_1 = '\x00\x01\x00\x31',
    };

    struct TextureData
    {
        uint32_t width;
        uint32_t height;
        uint32_t widthInBlocks;
        uint32_t heightInBlocks;
        uint32_t texelPitch = 16;
        uint32_t rowPitch;
        uint32_t slicePitch;
        uint8_t *untiledData;
    };

    struct ArenaFile
    {
        uint32_t magic; // 0x00

        PADDING(0x1C, 0);

        be<uint32_t> numFiles; // 0x20

        PADDING(0xC, 1);

        be<uint32_t> fileTableOffset; // 0x30
    };

#pragma pack(push, 1)
    struct ArenaTOC
    {
        be<uint32_t> offset; // 0
        be<uint64_t> size;   // 4
        PADDING(0x8, 0);
        be<uint32_t> type;
    };
#pragma pack(pop)

    static_assert(sizeof(ArenaTOC) == 24);
    static_assert(offsetof(ArenaTOC, size) == 4);

    static_assert(offsetof(ArenaFile, numFiles) == 0x20);
    static_assert(offsetof(ArenaFile, fileTableOffset) == 0x30);
};

class XTexHelper
{
public:
    XTexHelper() = default;
    ~XTexHelper() = default;

    void WriteToDDS(std::string inputFileName, std::string outputFilePath);
    void WriteToDDS(XTexHelperTypes::TextureData textureData, std::string outputFilePath);

    void WriteToTGA(std::string inputFileName, std::string outputFilePath);

    IDirect3DTexture9 *ConvertToDXTexture(LPDIRECT3DDEVICE9 device, XTexHelperTypes::TextureData textureData);

    XTexHelperTypes::TextureData GetTextureData(GuestBaseTexture *baseTexture, uint8_t *texData, bool byteSwap = true);

private:
    std::vector<uint8_t> ReadFileData(std::string path);
    void UntileSurface(LPPOINT point, uint32_t width, uint8_t *destination, uint32_t rowPitch, uint8_t *source, uint32_t height, LPRECT rect, uint32_t texelPitch);
    void SwapDXT5Data(uint8_t *data, uint32_t dataSize, size_t elementSize);
};