#include "xtexhelper.h"

using namespace XTexHelperTypes;

// Takes an RX2 Texture File
void XTexHelper::WriteToDDS(std::string rx2FilePath, std::string outputFilePath)
{
    if (rx2FilePath.empty())
    {
        Log::Error("XTexHelper", "File name is empty, cannot write to DDS.");
        return;
    }

    Log::Info("XTexHelper", "Writing to DDS file: ", rx2FilePath);

    auto data = ReadFileData(rx2FilePath);

    if (data.empty())
    {
        Log::Error("XTexHelper", "Failed to read file data for DDS conversion.");
        return;
    }

    // parse RX2 header.
    // we need to find the offset to the D3DBaseTexture, and the raw texel data.
    auto arenaFile = reinterpret_cast<ArenaFile *>(data.data());

    if (arenaFile->magic != RW4_MAGIC)
    {
        Log::Info("XTexHelper", "Magic is incorrect, not a valid file.");
        return;
    }

    Log::Info("XTexHelper", "ArenaFile numFiles: ", arenaFile->numFiles.get(), " fileTableOffset: ", arenaFile->fileTableOffset.get());

    uint32_t seek = 0;

    GuestBaseTexture *baseTexture = nullptr;
    uint8_t *rawTexelData = nullptr;
    uint32_t rawTexelDataSize = 0;

    for (uint32_t curFile = 0; curFile < arenaFile->numFiles; curFile++)
    {
        seek = arenaFile->fileTableOffset.get() + (curFile * sizeof(ArenaTOC));

        auto toc = reinterpret_cast<ArenaTOC *>(data.data() + seek);

        Log::Info("XTexHelper", "ArenaTOC offset: ", toc->offset, " size: ", toc->size, " type: ", toc->type);

        uint32_t dataOffset = toc->offset;

        if (toc->offset == 0)
        {
            // get first resource descriptor
            be<uint32_t> firstResourceOffset = *reinterpret_cast<be<uint32_t> *>(data.data() + 0x44);
            dataOffset += firstResourceOffset.get();
        }

        if (toc->type == RWGOBJECTTYPE_TEXTURE && !baseTexture)
        {
            Log::Info("XTexHelper", "Found base texture, offset: ", dataOffset, " size: ", toc->size);
            baseTexture = reinterpret_cast<GuestBaseTexture *>(data.data() + dataOffset);
        }
        else if (toc->type == RWOBJECTTYPE_BASERESOURCE_1 && !rawTexelData)
        {
            Log::Info("XTexHelper", "Found raw texel data, offset: ", dataOffset, " size: ", toc->size);
            rawTexelData = (data.data() + dataOffset);
            rawTexelDataSize = toc->size;
        }
    }

    WriteToDDS(GetTextureData(baseTexture, rawTexelData), outputFilePath);
}

void XTexHelper::WriteToDDS(XTexHelperTypes::TextureData textureData, std::string outputFilePath)
{
    // make the dds header
    DDSHeader ddsHeader = {0};
    ddsHeader.size = sizeof(DDSHeader);
    ddsHeader.flags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_LINEARSIZE;

    ddsHeader.height = textureData.height;
    ddsHeader.width = textureData.width;
    ddsHeader.pitchOrLinearSize = textureData.slicePitch;
    ddsHeader.depth = 1;
    ddsHeader.mipMapCount = 1;

    ddsHeader.ddspf.size = sizeof(DDSPixelFormat);
    ddsHeader.ddspf.flags = DDPF_FOURCC;                     // DDPF_FOURCC
    ddsHeader.ddspf.fourCC = MAKEFOURCC('D', 'X', 'T', '5'); // DXT5 format
    ddsHeader.caps = DDS_SURFACE_FLAGS_TEXTURE;

    std::ofstream outFile(outputFilePath, std::ios::binary);
    if (!outFile.is_open())
    {
        Log::Error("XTexHelper", "Failed to open output file for writing: ", outputFilePath);
        return;
    }

    const uint32_t DDS_SIGNATURE = MAKEFOURCC('D', 'D', 'S', ' ');
    outFile.write(reinterpret_cast<const char *>(&DDS_SIGNATURE), sizeof(DDS_SIGNATURE));
    outFile.write(reinterpret_cast<const char *>(&ddsHeader), sizeof(ddsHeader));
    outFile.write(reinterpret_cast<const char *>(textureData.untiledData), textureData.slicePitch);

    outFile.close();

    Log::Info("XTexHelper", "DDS file written successfully: ", outputFilePath);
}

TextureData XTexHelper::GetTextureData(GuestBaseTexture *baseTexture, uint8_t *rawTexelData, bool byteSwap)
{
    TextureData texData = {};

    auto constants = baseTexture->GetBitField();

    // print tex type
    uint32_t texWidth = constants.Size.TwoD.Width + 1;
    uint32_t texHeight = constants.Size.TwoD.Height + 1;
    uint32_t texMipLevels = baseTexture->GetLevelCount();

    // block width & height
    uint32_t blockWidth = (texWidth + 3) / 4;
    uint32_t blockHeight = (texHeight + 3) / 4;
    uint32_t rowPitch = blockWidth * texData.texelPitch;
    uint32_t slicePitch = rowPitch * blockHeight;

    texData.width = texWidth;
    texData.height = texHeight;
    texData.widthInBlocks = blockWidth;
    texData.heightInBlocks = blockHeight;
    texData.rowPitch = rowPitch;
    texData.slicePitch = slicePitch;

    Log::Info("XTexHelper", "Texture width: ", texWidth,
              " height: ", texHeight,
              " mipLevels: ", texMipLevels,
              " blockWidth: ", blockWidth,
              " blockHeight: ", blockHeight,
              " rowPitch: ", rowPitch,
              " slicePitch: ", slicePitch);

    // byte swap color fields

    if (byteSwap)
        SwapDXT5Data(rawTexelData, slicePitch, 2);

    POINT point = {0};
    RECT rect = {0, 0, static_cast<LONG>(blockWidth), static_cast<LONG>(blockHeight)};

    uint8_t *pUntiledBits = new uint8_t[slicePitch];

    // untile
    UntileSurface(&point, blockWidth, pUntiledBits, rowPitch, rawTexelData, blockHeight, &rect, texData.texelPitch);

    texData.untiledData = pUntiledBits;

    Log::Info("XTexHelper", "Untiled data size: ", slicePitch);

    return texData;
}

IDirect3DTexture9 *XTexHelper::ConvertToDXTexture(LPDIRECT3DDEVICE9 device, TextureData textureData)
{
    IDirect3DTexture9 *texture = nullptr;

    HRESULT hr = device->CreateTexture(
        textureData.width,
        textureData.height,
        1,
        0,
        D3DFMT_DXT5,
        D3DPOOL_MANAGED,
        &texture,
        nullptr);

    if (FAILED(hr))
    {
        Log::Error("ConvertToDXTexure", "Failed to create texture: ", hr);
        DebugBreak();
    }

    D3DLOCKED_RECT lockedRect;
    hr = texture->LockRect(0, &lockedRect, nullptr, 0);

    if (FAILED(hr))
    {
        Log::Error("ConvertToDXTexure", "Failed to lock texture: ", hr);
        DebugBreak();
    }

    uint8_t *dst = reinterpret_cast<uint8_t *>(lockedRect.pBits);

    for (uint32_t by = 0; by < textureData.heightInBlocks; ++by)
    {
        // Copy each row, respecting the destination pitch
        std::memcpy(dst + by * lockedRect.Pitch, textureData.untiledData + by * textureData.rowPitch, textureData.rowPitch);
    }

    texture->UnlockRect(0);

    return texture;
}

void XTexHelper::WriteToTGA(std::string inputFileName, std::string outputFilePath)
{
}

std::vector<uint8_t> XTexHelper::ReadFileData(std::string path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
        return {};

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    file.read(reinterpret_cast<char *>(data.data()), size);
    file.close();

    return data;
}

// only the heavens and the higher almighty knows what this does
void XTexHelper::UntileSurface(LPPOINT point, uint32_t width, uint8_t *destination, uint32_t rowPitch, uint8_t *source, uint32_t height, LPRECT rect, uint32_t texelPitch)
{
    tagRECT l_localRect;
    tagPOINT l_localPoint;
    LPRECT l_rect = rect;

    if (!rect)
    {
        l_localRect.left = 0;
        l_localRect.top = 0;
        l_localRect.right = width;
        l_localRect.bottom = height;
        l_rect = &l_localRect;
    }

    uint32_t l_rectWidth = l_rect->right - l_rect->left;
    uint32_t l_rectHeight = l_rect->bottom - l_rect->top;

    if (!point)
    {
        l_localPoint.x = 0;
        l_localPoint.y = 0;
        point = &l_localPoint;
    }

    // Calculate aligned width (round up to nearest multiple of 32)
    uint32_t l_alignedWidth = (width + 31) & 0xFFFFFFE0;

    void *l_sourceData;
    if (source == destination)
    {
        uint32_t l_alignSize = (texelPitch * l_alignedWidth * ((height + 31) & 0xFFFFFFE0) + 4095) & 0xFFFFF000;
        l_sourceData = _aligned_malloc(l_alignSize, 16u);
        memcpy((uint8_t *)l_sourceData, source, l_alignSize);
    }
    else
        l_sourceData = source;

    // Calculate block size and shift based on texel pitch
    uint32_t l_blockSize = 1 << ((texelPitch >> 4) - (texelPitch >> 2) + 3);
    uint32_t l_blockShift = (texelPitch >> 2) + (texelPitch >> 1 >> (texelPitch >> 2));

    // Calculate block alignment values
    int l_rectLeft = l_rect->left;
    uint32_t l_blockStartOffset = (~(l_blockSize - 1) & (l_rect->left + l_blockSize)) - l_rect->left;
    int l_rightEdgeOffset = (~(l_blockSize - 1) & (l_rectLeft + l_rectWidth)) - l_rectLeft;

    // Limit initial block width if it exceeds total width
    uint32_t l_leftBlockWidth = l_blockStartOffset;
    if (l_leftBlockWidth > l_rectWidth)
        l_leftBlockWidth = l_rectWidth;

    uint32_t l_bytesPerRow = l_leftBlockWidth << l_blockShift;

    // Process each row of the rectangle
    if (l_rectHeight)
    {
        uint32_t l_blockCountX = l_alignedWidth >> 5;

        for (uint32_t l_yOffset = 0; l_yOffset < l_rectHeight; l_yOffset++)
        {
            uint32_t l_y = l_yOffset + l_rect->top;
            uint32_t l_blockRowOffset = l_blockCountX * (l_y >> 5);
            uint32_t l_yBit4 = (l_y >> 4) & 1;
            uint32_t l_yBit3 = (l_y >> 3) & 1;
            uint32_t l_yBit0Offset = 16 * (l_y & 1);
            uint32_t l_yBits1_2Offset = 4 * (l_y & 6);
            uint32_t l_yBit3Offset = 2 * l_yBit3;
            uint32_t l_xPos = l_rect->left;
            uint32_t l_xOffsetBits = l_yBits1_2Offset + (l_rect->left & 7);
            uint32_t l_yBit3ShiftedOffset = l_yBit3 << (l_blockShift + 6);

            // Calculate destination row offset
            uint32_t l_rowOffset = rowPitch * (l_yOffset + point->y);

            // Calculate swizzled address components for this block
            uint32_t l_swizzledOffset = l_yBit0Offset + l_yBit3ShiftedOffset +
                                        ((l_xOffsetBits << (l_blockShift + 6) >> 6) & 0xF) +
                                        2 * (((l_xOffsetBits << (l_blockShift + 6) >> 6) & 0xFFFFFFF0) +
                                             (((l_blockRowOffset + (l_xPos >> 5)) << (l_blockShift + 6)) & 0x1FFFFFFF));

            // Copy first block (may be partial)
            memcpy(&destination[(point->x << l_blockShift) + l_rowOffset],
                   (uint8_t *)l_sourceData +
                       2048 * (((uint8_t)l_yBit4 + 2 * (((uint8_t)l_yBit3Offset + (uint8_t)(l_xPos >> 3)) & 3)) & 1) +
                       256 * ((l_swizzledOffset >> 6) & 7) +
                       32 * ((l_yBit4 + 2 * (((uint8_t)l_yBit3Offset + (uint8_t)(l_xPos >> 3)) & 3)) & 0xFFFFFFFE) +
                       8 * (l_swizzledOffset & 0xFFFFFE00) +
                       (l_swizzledOffset & 0x3F),
                   l_bytesPerRow);

            // Process full blocks in the middle
            uint32_t l_currentXOffset = l_blockStartOffset;
            if ((int)l_blockStartOffset < l_rightEdgeOffset)
            {
                uint32_t l_blockBytes = l_blockSize << l_blockShift;

                do
                {
                    uint32_t l_middleXPos = l_currentXOffset + l_rect->left;
                    uint32_t l_middleXOffsetBits = (l_yBits1_2Offset + (l_middleXPos & 7)) << (l_blockShift + 6);

                    uint32_t l_middleSwizzledOffset = l_yBit0Offset + l_yBit3ShiftedOffset +
                                                      ((l_middleXOffsetBits >> 6) & 0xF) +
                                                      2 * (((l_middleXOffsetBits >> 6) & 0xFFFFFFF0) +
                                                           (((l_blockRowOffset + (l_middleXPos >> 5)) << (l_blockShift + 6)) & 0x1FFFFFFF));

                    uint32_t l_middleYOffset = l_yBit4 + 2 * (((uint8_t)l_yBit3Offset + (uint8_t)(l_middleXPos >> 3)) & 3);

                    memcpy(&destination[((l_currentXOffset + point->x) << l_blockShift) + l_rowOffset],
                           (uint8_t *)l_sourceData +
                               256 * (((l_middleSwizzledOffset >> 6) & 7) + 8 * (((uint8_t)l_yBit4 + 2 * (((uint8_t)l_yBit3Offset + (uint8_t)(l_middleXPos >> 3)) & 3)) & 1)) +
                               32 * (l_middleYOffset & 0xFFFFFFFE) +
                               8 * (l_middleSwizzledOffset & 0xFFFFFE00) +
                               (l_middleSwizzledOffset & 0x3F),
                           l_blockBytes);

                    l_currentXOffset += l_blockSize;
                } while (l_currentXOffset < l_rightEdgeOffset);
            }

            // Process the rightmost partial block if needed
            if (l_currentXOffset < l_rectWidth)
            {
                uint32_t l_rightXPos = l_currentXOffset + l_rect->left;
                uint32_t l_rightYOffset = l_yBit4 + 2 * (((uint8_t)l_yBit3Offset + (uint8_t)(l_rightXPos >> 3)) & 3);
                uint32_t l_rightXOffsetBits = (l_yBits1_2Offset + (l_rightXPos & 7)) << (l_blockShift + 6);

                uint32_t l_rightSwizzledOffset = l_yBit0Offset + l_yBit3ShiftedOffset +
                                                 ((l_rightXOffsetBits >> 6) & 0xF) +
                                                 2 * (((l_rightXOffsetBits >> 6) & 0xFFFFFFF0) +
                                                      (((l_blockRowOffset + (l_rightXPos >> 5)) << (l_blockShift + 6)) & 0x1FFFFFFF));

                memcpy(&destination[((l_currentXOffset + point->x) << l_blockShift) + l_rowOffset],
                       (uint8_t *)l_sourceData +
                           2048 * (l_rightYOffset & 1) +
                           256 * ((l_rightSwizzledOffset >> 6) & 7) +
                           32 * (l_rightYOffset & 0xFFFFFFFE) +
                           8 * (l_rightSwizzledOffset & 0xFFFFFE00) +
                           (l_rightSwizzledOffset & 0x3F),
                       (l_rectWidth - l_currentXOffset) << l_blockShift);
            }
        }
    }

    // Free temporary buffer if created
    if (source == destination)
        _aligned_free(l_sourceData);
}

void XTexHelper::SwapDXT5Data(uint8_t *data, uint32_t dataSize, size_t elementSize)
{
    if (dataSize % elementSize != 0)
        return;

    size_t numElements = dataSize / elementSize;
    unsigned char *pData = data;
    for (size_t i = 0; i < numElements; i++)
    {
        unsigned char *pElement = pData + i * elementSize;
        for (size_t j = 0; j < elementSize / 2; j++)
        {
            std::swap(pElement[j], pElement[elementSize - 1 - j]);
        }
    }
}