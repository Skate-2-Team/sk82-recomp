#include <windows.h>
#include <iostream>
#include <memory>

#include "ppc/ppc_recomp_shared.h"

#include "kernel/heap.h"
#include "kernel/memory.h"
#include "kernel/function.h"
#include "kernel/hooks/hooks.h"
#include "kernel/ppc_local.h"
#include "cpu/guest_thread.h"

#include "graphics/video.h"

#include "loader.h"
#include "log.h"

int main()
{
    Log::m_printStubs = false;

    Log::Info("Main", "Skate 2 recomp launching...");

    g_heap = std::make_shared<Heap>();
    g_video = std::make_shared<Video>();

    Memory::Init();

    if (Loader::Init("game/default.xex"))
    {
        SetConsoleTitleA("Skate 2 - Debug Console");

        GuestThread::Start({Loader::m_startAddress, 0, 0});
    }

    std::cin.get();

    return 0;
}