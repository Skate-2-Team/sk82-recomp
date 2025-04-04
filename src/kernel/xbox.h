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

enum XBOXPRIMITIVETYPE : __int32
{
    XD3DPT_POINTLIST = 0x1,
    XD3DPT_LINELIST = 0x2,
    XD3DPT_LINESTRIP = 0x3,
    XD3DPT_TRIANGLELIST = 0x4,
    XD3DPT_TRIANGLEFAN = 0x5,
    XD3DPT_TRIANGLESTRIP = 0x6,
    XD3DPT_RECTLIST = 0x8,
    XD3DPT_QUADLIST = 0xD,
    XD3DPT_FORCE_DWORD = 0x7FFFFFFF,
};

enum _D3DBLOCKTYPE : __int32
{
    D3DBLOCKTYPE_NONE = 0x0,
    D3DBLOCKTYPE_PRIMARY_OVERRUN = 0x1,
    D3DBLOCKTYPE_SECONDARY_OVERRUN = 0x2,
    D3DBLOCKTYPE_SWAP_THROTTLE = 0x3,
    D3DBLOCKTYPE_BLOCK_UNTIL_IDLE = 0x4,
    D3DBLOCKTYPE_BLOCK_UNTIL_NOT_BUSY = 0x5,
    D3DBLOCKTYPE_BLOCK_ON_FENCE = 0x6,
    D3DBLOCKTYPE_VERTEX_SHADER_RELEASE = 0x7,
    D3DBLOCKTYPE_PIXEL_SHADER_RELEASE = 0x8,
    D3DBLOCKTYPE_VERTEX_BUFFER_RELEASE = 0x9,
    D3DBLOCKTYPE_VERTEX_BUFFER_LOCK = 0xA,
    D3DBLOCKTYPE_INDEX_BUFFER_RELEASE = 0xB,
    D3DBLOCKTYPE_INDEX_BUFFER_LOCK = 0xC,
    D3DBLOCKTYPE_TEXTURE_RELEASE = 0xD,
    D3DBLOCKTYPE_TEXTURE_LOCK = 0xE,
    D3DBLOCKTYPE_COMMAND_BUFFER_RELEASE = 0xF,
    D3DBLOCKTYPE_COMMAND_BUFFER_LOCK = 0x10,
    D3DBLOCKTYPE_CONSTANT_BUFFER_RELEASE = 0x11,
    D3DBLOCKTYPE_CONSTANT_BUFFER_LOCK = 0x12,
    D3DBLOCKTYPE_MAX = 0x13,
};

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

typedef struct _XOVERLAPPED
{
    be<uint32_t> Internal;
    be<uint32_t> InternalHigh;
    be<uint32_t> Offset;
    be<uint32_t> OffsetHigh;
    be<uint32_t> hEvent;
} XOVERLAPPED;

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

// Content types
#define XCONTENTTYPE_SAVEDATA 1
#define XCONTENTTYPE_DLC 2
#define XCONTENTTYPE_RESERVED 3

#define XCONTENT_NEW 1
#define XCONTENT_EXISTING 2

#define XCONTENT_MAX_DISPLAYNAME 128
#define XCONTENT_MAX_FILENAME 42
#define XCONTENTDEVICE_MAX_NAME 27

#define MSG_AREA(msgid) (((msgid) >> 16) & 0xFFFF)

typedef struct _XXOVERLAPPED
{
    union
    {
        struct
        {
            be<uint32_t> Error;
            be<uint32_t> Length;
        };

        struct
        {
            uint32_t InternalLow;
            uint32_t InternalHigh;
        };
    };
    uint32_t InternalContext;
    be<uint32_t> hEvent;
    be<uint32_t> pCompletionRoutine;
    be<uint32_t> dwCompletionContext;
    be<uint32_t> dwExtendedError;
} XXOVERLAPPED, *PXXOVERLAPPED;

typedef struct _XCONTENT_DATA
{
    be<uint32_t> DeviceID;
    be<uint32_t> dwContentType;
    be<uint16_t> szDisplayName[XCONTENT_MAX_DISPLAYNAME];
    char szFileName[XCONTENT_MAX_FILENAME];
} XCONTENT_DATA, *PXCONTENT_DATA;

typedef struct _XUSER_SIGNIN_INFO
{
    be<uint64_t> xuid;
    be<uint32_t> dwField08;
    be<uint32_t> SigninState;
    be<uint32_t> dwField10;
    be<uint32_t> dwField14;
    char Name[16];
} XUSER_SIGNIN_INFO;

// Direct reflection of XInput structures

#define XAMINPUT_DEVTYPE_GAMEPAD 0x01
#define XAMINPUT_DEVSUBTYPE_GAMEPAD 0x01

#define XAMINPUT_GAMEPAD_DPAD_UP 0x0001
#define XAMINPUT_GAMEPAD_DPAD_DOWN 0x0002
#define XAMINPUT_GAMEPAD_DPAD_LEFT 0x0004
#define XAMINPUT_GAMEPAD_DPAD_RIGHT 0x0008
#define XAMINPUT_GAMEPAD_START 0x0010
#define XAMINPUT_GAMEPAD_BACK 0x0020
#define XAMINPUT_GAMEPAD_LEFT_THUMB 0x0040
#define XAMINPUT_GAMEPAD_RIGHT_THUMB 0x0080
#define XAMINPUT_GAMEPAD_LEFT_SHOULDER 0x0100
#define XAMINPUT_GAMEPAD_RIGHT_SHOULDER 0x0200
#define XAMINPUT_GAMEPAD_A 0x1000
#define XAMINPUT_GAMEPAD_B 0x2000
#define XAMINPUT_GAMEPAD_X 0x4000
#define XAMINPUT_GAMEPAD_Y 0x8000

typedef struct _XAMINPUT_GAMEPAD
{
    uint16_t wButtons;
    uint8_t bLeftTrigger;
    uint8_t bRightTrigger;
    int16_t sThumbLX;
    int16_t sThumbLY;
    int16_t sThumbRX;
    int16_t sThumbRY;
} XAMINPUT_GAMEPAD, *PXAMINPUT_GAMEPAD;

typedef struct _XAMINPUT_VIBRATION
{
    uint16_t wLeftMotorSpeed;
    uint16_t wRightMotorSpeed;
} XAMINPUT_VIBRATION, *PXAMINPUT_VIBRATION;

typedef struct _XAMINPUT_CAPABILITIES
{
    uint8_t Type;
    uint8_t SubType;
    uint16_t Flags;
    XAMINPUT_GAMEPAD Gamepad;
    XAMINPUT_VIBRATION Vibration;
} XAMINPUT_CAPABILITIES, *PXAMINPUT_CAPABILITIES;

typedef struct _XAMINPUT_STATE
{
    uint32_t dwPacketNumber;
    XAMINPUT_GAMEPAD Gamepad;
} XAMINPUT_STATE, *PXAMINPUT_STATE;
