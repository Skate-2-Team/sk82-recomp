#pragma once

#include <windows.h>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#include "kernel/memory.h"
#include "ppc/ppc_recomp_shared.h"
#include "xex.h"
#include "log.h"

namespace Loader
{
    inline uint32_t m_startAddress = 0;

    bool Init(const std::string &p_xexPath);
}