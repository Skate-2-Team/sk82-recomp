#pragma once

#include <cstdint>

struct TargaHeader
{
    uint8_t IDLength;
    uint8_t ColormapType;
    uint8_t ImageType;
    uint8_t ColormapSpecification[5];
    uint16_t XOrigin;
    uint16_t YOrigin;
    uint16_t ImageWidth;
    uint16_t ImageHeight;
    uint8_t PixelDepth;
    uint8_t ImageDescriptor;
};

static_assert(sizeof(TargaHeader) == 18);