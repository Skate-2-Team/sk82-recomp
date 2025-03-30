#include "xdm.h"

Mutex g_kernelLock;

void DestroyKernelObject(KernelObject *obj)
{
    obj->~KernelObject();
    g_heap->Free(obj);
}

uint32_t GetKernelHandle(KernelObject *obj)
{
    assert(obj != GetInvalidKernelObject());
    return Memory::MapVirtual(obj);
}

void DestroyKernelObject(uint32_t handle)
{
    DestroyKernelObject(GetKernelObject(handle));
}

bool IsKernelObject(uint32_t handle)
{
    return (handle & 0x80000000) != 0;
}

bool IsKernelObject(void *obj)
{
    return IsKernelObject(Memory::MapVirtual(obj));
}

bool IsInvalidKernelObject(void *obj)
{
    return obj == GetInvalidKernelObject();
}
