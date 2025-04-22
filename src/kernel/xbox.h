#pragma once

#include "xex.h"

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

#define XCONTENTDEVICETYPE_HDD 1
#define XCONTENTDEVICETYPE_MU 2

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

        return reinterpret_cast<T *>(Memory::MmGetHostAddress(ptr));
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

typedef struct _XDEVICE_DATA
{
    be<uint32_t> DeviceID;
    be<uint32_t> DeviceType;
    be<uint64_t> ulDeviceBytes;
    be<uint64_t> ulDeviceFreeBytes;
    be<uint16_t> wszName[XCONTENTDEVICE_MAX_NAME];
} XDEVICE_DATA, *PXDEVICE_DATA;

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
