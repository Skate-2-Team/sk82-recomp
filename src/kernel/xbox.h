#pragma once

#include "xex.h"

inline struct _STRING
{
    unsigned __int16 Length;
    unsigned __int16 MaximumLength;
    unsigned __int8 *Buffer;
} *PSTRING;

inline struct _XVIDEO_MODE
{
    be<unsigned int> dwDisplayWidth;
    be<unsigned int> dwDisplayHeight;
    be<int> fIsInterlaced;
    be<int> fIsWideScreen;
    be<int> fIsHiDef;
    be<float> RefreshRate;
    be<unsigned int> VideoStandard;
    be<unsigned int> Reserved[5];
} *PXVIDEO_MODE, XVIDEO_MODE;

typedef struct _XLIST_ENTRY
{
    be<uint32_t> Flink;
    be<uint32_t> Blink;
} XLIST_ENTRY;

typedef struct _XDISPATCHER_HEADER
{
    union
    {
        struct
        {
            uint8_t Type;
            union
            {
                uint8_t Abandoned;
                uint8_t Absolute;
                uint8_t NpxIrql;
                uint8_t Signalling;
            };
            union
            {
                uint8_t Size;
                uint8_t Hand;
            };
            union
            {
                uint8_t Inserted;
                uint8_t DebugActive;
                uint8_t DpcActive;
            };
        };
        be<uint32_t> Lock;
    };

    be<uint32_t> SignalState;
    XLIST_ENTRY WaitListHead;
} XDISPATCHER_HEADER, *XPDISPATCHER_HEADER;

// These variables are never accessed in guest code, we can safely use them in little endian
typedef struct _XRTL_CRITICAL_SECTION
{
    XDISPATCHER_HEADER Header;
    int32_t LockCount;
    int32_t RecursionCount;
    uint32_t OwningThread;
} XRTL_CRITICAL_SECTION;

struct _D3DSURFACE_PARAMETERS
{
    be<unsigned int> Base;
    be<unsigned int> HierarchicalZBase;
    be<int> ColorExpBias;
};
