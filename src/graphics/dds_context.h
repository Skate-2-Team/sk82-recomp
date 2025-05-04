#pragma once

#include <cstdint>

#define DDS_HEADER_FLAGS_TEXTURE 0x00001007
#define DDS_HEADER_FLAGS_MIPMAP 0x00020000
#define DDS_HEADER_FLAGS_VOLUME 0x00800000
#define DDS_HEADER_FLAGS_PITCH 0x00000008
#define DDS_HEADER_FLAGS_LINEARSIZE 0x00080000

#define DDS_HEIGHT 0x00000002
#define DDS_WIDTH 0x00000004

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000
#define DDS_SURFACE_FLAGS_MIPMAP 0x00400008
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008

#define DDPF_FOURCC 0x00000004

struct DDSPixelFormat
{
    uint32_t size;
    uint32_t flags;
    uint32_t fourCC;
    uint32_t rGBBitCount;
    uint32_t rBitMask;
    uint32_t gBitMask;
    uint32_t bBitMask;
    uint32_t aBitMask;
};

typedef struct
{
    uint32_t size;
    uint32_t flags;
    uint32_t height;
    uint32_t width;
    uint32_t pitchOrLinearSize;
    uint32_t depth;
    uint32_t mipMapCount;
    uint32_t reserved1[11];
    DDSPixelFormat ddspf;
    uint32_t caps;
    uint32_t caps2;
    uint32_t caps3;
    uint32_t caps4;
    uint32_t reserved2;
} DDSHeader;

static_assert(sizeof(DDSHeader) == 124);
static_assert(sizeof(DDSPixelFormat) == 32);