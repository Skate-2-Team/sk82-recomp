#include "hooks.h"

namespace Hooks
{
    static std::string ResolvePath(std::string fileName)
    {
        // strip first 6 characters
        fileName.erase(fileName.begin(), fileName.begin() + 6);

        std::string fullPath = "game\\" + fileName;

        return fullPath;
    }

    BOOL Hooks_GetFileSizeEx(FileHandle *hFile, _LARGE_INTEGER *lpFileSize)
    {
        if (!GetFileSizeEx(hFile->m_handle, lpFileSize))
            return false;

        if (!lpFileSize)
            return false;

        Log::Info("GetFileSizeEx", "File size: ", lpFileSize->QuadPart);

        lpFileSize->QuadPart = ByteSwap(lpFileSize->QuadPart);

        return true;
    }

    DWORD Hooks_SetFilePointer(FileHandle *hFile, LONG lDistance, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
    {
        return SetFilePointer(hFile->m_handle, lDistance, lpDistanceToMoveHigh, dwMoveMethod);
    }

    FileHandle *Hooks_CreateFileA(const char *lpFileName,
                                  unsigned int dwDesiredAccess,
                                  unsigned int dwShareMode,
                                  void *lpSecurityAttributes,
                                  unsigned int dwCreationDisposition,
                                  unsigned int dwFlagsAndAttributes,
                                  void *hTemplateFile)
    {
        if (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED) // no async
        {
            Log::Error("CreateFileA", "Overlapped file access is not supported.");
            return GetInvalidKernelObject<FileHandle>();
        }

        std::string fileName(lpFileName);

        auto tempname = fileName.find("game:\\");

        if (tempname == std::string::npos)
        {
            tempname = fileName.find("GAME:\\");
        }

        if (tempname == std::string::npos)
        {
            Log::Error("CreateFileA", "Returning INVALID_HANDLE_VALUE for -> ", lpFileName);

            return GetInvalidKernelObject<FileHandle>();
        }

        std::string actualPath = ResolvePath(fileName);

        auto handle = CreateFileA(
            actualPath.c_str(), dwDesiredAccess,
            dwShareMode, nullptr, dwCreationDisposition, dwFlagsAndAttributes,
            nullptr);

        if (handle == INVALID_HANDLE_VALUE)
        {
            Log::Error("CreateFileA", "Failed to open file -> ", lpFileName);
            return GetInvalidKernelObject<FileHandle>();
        }

        FileHandle *newHandle = CreateKernelObject<FileHandle>(handle);
        newHandle->m_handle = handle;

        Log::Info("CreateFileA", "Opening file -> ", lpFileName, ", handle is -> ", handle);

        return newHandle;
    }

    BOOL Hooks_WriteFile(FileHandle *hFile,
                         const void *lpBuffer,
                         unsigned int nNumberOfBytesToWrite,
                         unsigned int *lpNumberOfBytesWritten,
                         _OVERLAPPED *lpOverlapped)
    {
        Log::Info("WriteFile", "Invoked");

        return WriteFile(hFile->m_handle, lpBuffer, nNumberOfBytesToWrite, reinterpret_cast<LPDWORD>(lpNumberOfBytesWritten), lpOverlapped);
    }

    BOOL Hooks_ReadFile(FileHandle *hFile,
                        void *lpBuffer,
                        unsigned int nNumberOfBytesToRead,
                        unsigned int *lpNumberOfBytesRead,
                        _OVERLAPPED *lpOverlapped)
    {
        std::vector<uint8_t> buffer(nNumberOfBytesToRead);

        // Log::Info("ReadFile", "Reading file with handle -> ", reinterpret_cast<HANDLE>(hFile), ", Num Bytes -> ", nNumberOfBytesToRead);

        DWORD bytesRead = 0;
        if (ReadFile(hFile->m_handle, buffer.data(), nNumberOfBytesToRead, &bytesRead, NULL))
        {
            memcpy(lpBuffer, buffer.data(), bytesRead);
            return true;
        }

        Log::Error("ReadFile", "Failed to read file. Error: ", GetLastError());
        return false;
    }

    // This func calls NtCreateFile directly, so we need to hook it so we dont crash
    BOOL Hooks_CreateDirectoryA(const char *lpPathName, void *lpSecurityAttributes)
    {
        Log::Info("CreateDirectoryA", "Creating directory -> ", lpPathName);

        std::string fileName(lpPathName);

        auto tempname = fileName.find("game:\\");

        if (tempname == std::string::npos)
        {
            tempname = fileName.find("GAME:\\");
        }

        if (tempname == std::string::npos) // not a game path
        {
            return false;
        }

        std::string actualPath = ResolvePath(fileName);

        return CreateDirectoryA(actualPath.c_str(), 0);
    }

    void Import_NtOpenFile()
    {
        Log::Stub("NtOpenFile", "Called.");
    }

    void Import_NtCreateFile()
    {
        Log::Stub("NtOpenFile", "Called.");
    }

    void Import_NtWriteFile()
    {
        Log::Stub("NtWriteFile", "Called.");
    }

    void Import_NtSetInformationFile()
    {
        Log::Stub("NtSetInformationFile", "Called.");
    }

    void Import_NtQueryInformationFile()
    {
        Log::Stub("NtQueryInformationFile", "Called.");
    }

    void Import_NtQueryVolumeInformationFile()
    {
        Log::Stub("NtQueryVolumeInformationFile", "Called.");
    }

    void Import_NtQueryDirectoryFile()
    {
        Log::Stub("NtQueryDirectoryFile", "Called.");
    }

    void Import_NtReadFileScatter()
    {
        Log::Stub("NtReadFileScatter", "Called.");
    }

    void Import_NtReadFile()
    {
        Log::Stub("NtReadFile", "Called.");
    }

    void Import_ObDeleteSymbolicLink()
    {
        Log::Stub("ObDeleteSymbolicLink", "Called.");
    }

    void Import_ObCreateSymbolicLink()
    {
        Log::Stub("ObCreateSymbolicLink", "Called.");
    }

    void Import_NtQueryFullAttributesFile()
    {
        Log::Stub("NtQueryFullAttributesFile", "Called.");
    }

    void Import_NtFlushBuffersFile()
    {
        Log::Stub("NtFlushBuffersFile", "Called.");
    }

    void Import_IoDismountVolume()
    {
        Log::Stub("IoDismountVolume", "Called.");
    }

    void Import_IoDismountVolumeByFileHandle()
    {
        Log::Stub("IoDismountVolumeByFileHandle", "Called.");
    }

}

GUEST_FUNCTION_HOOK(sub_82C74A90, Hooks::Hooks_WriteFile)
GUEST_FUNCTION_HOOK(sub_82C74908, Hooks::Hooks_ReadFile)
GUEST_FUNCTION_HOOK(sub_82C74F90, Hooks::Hooks_CreateFileA)
GUEST_FUNCTION_HOOK(sub_82C78A98, Hooks::Hooks_GetFileSizeEx)
GUEST_FUNCTION_HOOK(sub_82C74C48, Hooks::Hooks_SetFilePointer)
GUEST_FUNCTION_HOOK(sub_82C74BB8, Hooks::Hooks_CreateDirectoryA)

GUEST_FUNCTION_HOOK(__imp__ObCreateSymbolicLink, Hooks::Import_ObCreateSymbolicLink)
GUEST_FUNCTION_HOOK(__imp__ObDeleteSymbolicLink, Hooks::Import_ObDeleteSymbolicLink)

GUEST_FUNCTION_HOOK(__imp__IoDismountVolumeByFileHandle, Hooks::Import_IoDismountVolumeByFileHandle)
GUEST_FUNCTION_HOOK(__imp__IoDismountVolume, Hooks::Import_IoDismountVolume)

GUEST_FUNCTION_HOOK(__imp__NtFlushBuffersFile, Hooks::Import_NtFlushBuffersFile)
GUEST_FUNCTION_HOOK(__imp__NtQueryFullAttributesFile, Hooks::Import_NtQueryFullAttributesFile)
GUEST_FUNCTION_HOOK(__imp__NtOpenFile, Hooks::Import_NtOpenFile)
GUEST_FUNCTION_HOOK(__imp__NtCreateFile, Hooks::Import_NtCreateFile)
GUEST_FUNCTION_HOOK(__imp__NtWriteFile, Hooks::Import_NtWriteFile)
GUEST_FUNCTION_HOOK(__imp__NtSetInformationFile, Hooks::Import_NtSetInformationFile)
GUEST_FUNCTION_HOOK(__imp__NtQueryInformationFile, Hooks::Import_NtQueryInformationFile)
GUEST_FUNCTION_HOOK(__imp__NtQueryVolumeInformationFile, Hooks::Import_NtQueryVolumeInformationFile)
GUEST_FUNCTION_HOOK(__imp__NtQueryDirectoryFile, Hooks::Import_NtQueryDirectoryFile)
GUEST_FUNCTION_HOOK(__imp__NtReadFileScatter, Hooks::Import_NtReadFileScatter)
GUEST_FUNCTION_HOOK(__imp__NtReadFile, Hooks::Import_NtReadFile)
