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

template <typename TGuest>
struct HostObject
{
    typedef TGuest guest_type;
};

typedef struct _XKSEMAPHORE
{
    XDISPATCHER_HEADER Header;
    be<uint32_t> Limit;
} XKSEMAPHORE;

typedef XDISPATCHER_HEADER XKEVENT;

extern "C" void *MmGetHostAddress(uint32_t ptr);
template <typename T>
struct xpointer
{
    be<uint32_t> ptr;

    xpointer() : ptr(0)
    {
    }

    xpointer(T *p) : ptr(p != nullptr ? (reinterpret_cast<size_t>(p) - reinterpret_cast<size_t>(MmGetHostAddress(0))) : 0)
    {
    }

    T *get() const
    {
        if (!ptr.value)
        {
            return nullptr;
        }

        return reinterpret_cast<T *>(MmGetHostAddress(ptr));
    }

    operator T *() const
    {
        return get();
    }

    T *operator->() const
    {
        return get();
    }
};

typedef struct _XANSI_STRING
{
    be<uint16_t> Length;
    be<uint16_t> MaximumLength;
    xpointer<char> Buffer;
} XANSI_STRING;

typedef struct _XOBJECT_ATTRIBUTES
{
    be<uint32_t> RootDirectory;
    xpointer<XANSI_STRING> Name;
    xpointer<void> Attributes;
} XOBJECT_ATTRIBUTES;

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

struct _MM_STATISTICS
{
    be<unsigned int> Length;
    be<unsigned int> TotalPhysicalPages;
    be<unsigned int> KernelPages;
    be<unsigned int> TitleAvailablePages;
    be<unsigned int> TitleTotalVirtualMemoryBytes;
    be<unsigned int> TitleReservedVirtualMemoryBytes;
    be<unsigned int> TitlePhysicalPages;
    be<unsigned int> TitlePoolPages;
    be<unsigned int> TitleStackPages;
    be<unsigned int> TitleImagePages;
    be<unsigned int> TitleHeapPages;
    be<unsigned int> TitleVirtualPages;
    be<unsigned int> TitlePageTablePages;
    be<unsigned int> TitleCachePages;
    be<unsigned int> SystemAvailablePages;
    be<unsigned int> SystemTotalVirtualMemoryBytes;
    be<unsigned int> SystemReservedVirtualMemoryBytes;
    be<unsigned int> SystemPhysicalPages;
    be<unsigned int> SystemPoolPages;
    be<unsigned int> SystemStackPages;
    be<unsigned int> SystemImagePages;
    be<unsigned int> SystemHeapPages;
    be<unsigned int> SystemVirtualPages;
    be<unsigned int> SystemPageTablePages;
    be<unsigned int> SystemCachePages;
    be<unsigned int> HighestPhysicalPage;
};
