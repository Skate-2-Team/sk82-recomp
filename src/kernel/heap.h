#pragma once
#include <mutex>
#include "memory.h"
#include "o1heap/o1heap.h"

constexpr size_t RESERVED_BEGIN = 0x7FEA0000;
constexpr size_t RESERVED_END = 0xA0000000;

class Heap
{
    O1HeapInstance *m_heap;
    O1HeapInstance *m_physicalHeap;
    std::mutex m_heapMutex;
    std::mutex m_physicalHeapMutex;

public:
    void *Alloc(std::size_t const size)
    {
        std::lock_guard lockGuard(m_heapMutex);
        return o1heapAllocate(m_heap, std::max<size_t>(1, size));
    }

    void *AllocPhysical(std::size_t size, std::size_t alignment)
    {
        size = std::max<size_t>(1, size);
        alignment = alignment == 0 ? 0x1000 : std::max<size_t>(16, alignment);

        std::lock_guard lockGuard(m_physicalHeapMutex);

        void *ptr = o1heapAllocate(m_physicalHeap, size + alignment);

        // PrintPhysicalHeapPercent();

        if (o1heapGetDiagnostics(m_physicalHeap).oom_count > 0)
        {
            Log::Error("Heap", "Out of memory in physical heap, allocated: ", o1heapGetDiagnostics(m_physicalHeap).allocated, " capacity -> ", o1heapGetDiagnostics(m_physicalHeap).capacity);
            DebugBreak();
        }

        size_t aligned = (reinterpret_cast<size_t>(ptr) + alignment) & ~(alignment - 1);

        *(reinterpret_cast<void **>(aligned) - 1) = ptr;
        *(reinterpret_cast<size_t *>(aligned) - 2) = size + O1HEAP_ALIGNMENT;

        return reinterpret_cast<void *>(aligned);
    }

    void Free(void *ptr)
    {
        if (ptr >= m_physicalHeap)
        {
            std::lock_guard lock(m_physicalHeapMutex);
            o1heapFree(m_physicalHeap, *(static_cast<void **>(ptr) - 1));
        }
        else
        {
            std::lock_guard lock(m_heapMutex);
            o1heapFree(m_heap, ptr);
        }
    }

    static size_t Size(void *ptr)
    {
        if (ptr)
            return *(static_cast<size_t *>(ptr) - 2) - O1HEAP_ALIGNMENT; // relies on fragment header in o1heap.c

        return 0;
    }

    void PrintPhysicalHeapPercent()
    {
        O1HeapDiagnostics diagnostics = o1heapGetDiagnostics(m_physicalHeap);

        Log::Info("Heap", diagnostics.allocated, " / ", diagnostics.capacity);

        if (diagnostics.allocated > 623426240)
        {
            DebugBreak();
        }
    }

    Heap()
    {
        Log::Info("Heap", "Initializing heap");

        m_heap = o1heapInit(Memory::Translate(0x20000), RESERVED_BEGIN - 0x20000);
        m_physicalHeap = o1heapInit(Memory::Translate(RESERVED_END), 0x100000000 - RESERVED_END);

        Log::Info("Heap", "Address of m_physicalHeap: ", (void *)m_physicalHeap);
        Log::Info("Heap", "Address of m_heap: ", (void *)m_heap);
    }

    template <typename T, typename... Args>
    T *Alloc(Args &&...args)
    {
        T *obj = (T *)Alloc(sizeof(T));
        new (obj) T(std::forward<Args>(args)...);
        return obj;
    }

    template <typename T, typename... Args>
    T *AllocPhysical(Args &&...args)
    {
        T *obj = (T *)AllocPhysical(sizeof(T), alignof(T));
        new (obj) T(std::forward<Args>(args)...);
        return obj;
    }
};

inline std::shared_ptr<Heap> g_heap = nullptr;
