#include "hooks.h"

/*
99% of this is from Unleashed Recompiled, all credits to the original authors <3
https://github.com/hedge-dev/UnleashedRecomp/blob/main/UnleashedRecomp/kernel/io/file_system.cpp
*/

namespace Hooks
{

    FileHandle *Hooks_CreateFileA(
        const char *lpFileName,
        uint32_t dwDesiredAccess,
        uint32_t dwShareMode,
        void *lpSecurityAttributes,
        uint32_t dwCreationDisposition,
        uint32_t dwFlagsAndAttributes)
    {
        assert(((dwDesiredAccess & ~(GENERIC_READ | GENERIC_WRITE | FILE_READ_DATA)) == 0) && "Unknown desired access bits.");
        assert(((dwShareMode & ~(FILE_SHARE_READ | FILE_SHARE_WRITE)) == 0) && "Unknown share mode bits.");
        assert(((dwCreationDisposition & ~(CREATE_NEW | CREATE_ALWAYS)) == 0) && "Unknown creation disposition bits.");

        std::filesystem::path filePath = ResolvePath(lpFileName);

        Log::Info("CreateFileA", "Creating file -> ", filePath);

        std::fstream fileStream;
        std::ios::openmode fileOpenMode = std::ios::binary;
        if (dwDesiredAccess & (GENERIC_READ | FILE_READ_DATA))
        {
            fileOpenMode |= std::ios::in;
        }

        if (dwDesiredAccess & GENERIC_WRITE)
        {
            fileOpenMode |= std::ios::out;
        }

        fileStream.open(filePath, fileOpenMode);
        if (!fileStream.is_open())
        {
            Log::Error("CreateFileA", "Failed to open file -> ", filePath);

#ifdef _WIN32
            GuestThread::SetLastError(GetLastError());
#endif
            return GetInvalidKernelObject<FileHandle>();
        }

        if (filePath.extension() == ".vp6")
        {
            DebugBreak();
        }

        FileHandle *fileHandle = CreateKernelObject<FileHandle>();
        fileHandle->stream = std::move(fileStream);
        fileHandle->path = std::move(filePath);
        return fileHandle;
    }

    static uint32_t Hooks_GetFileSize(FileHandle *hFile, be<uint32_t> *lpFileSizeHigh)
    {
        std::error_code ec;
        auto fileSize = std::filesystem::file_size(hFile->path, ec);
        if (!ec)
        {
            if (lpFileSizeHigh != nullptr)
            {
                *lpFileSizeHigh = uint32_t(fileSize >> 32U);
            }

            return (uint32_t)(fileSize);
        }

        return INVALID_FILE_SIZE;
    }

    uint32_t Hooks_GetFileSizeEx(FileHandle *hFile, LARGE_INTEGER *lpFileSize)
    {
        std::error_code ec;
        auto fileSize = std::filesystem::file_size(hFile->path, ec);
        if (!ec)
        {
            if (lpFileSize != nullptr)
            {
                lpFileSize->QuadPart = ByteSwap(fileSize);
            }

            return TRUE;
        }

        return FALSE;
    }

    uint32_t Hooks_ReadFile(
        FileHandle *hFile,
        void *lpBuffer,
        uint32_t nNumberOfBytesToRead,
        be<uint32_t> *lpNumberOfBytesRead,
        XOVERLAPPED *lpOverlapped)
    {
        uint32_t result = FALSE;
        if (lpOverlapped != nullptr)
        {
            std::streamoff streamOffset = lpOverlapped->Offset + (std::streamoff(lpOverlapped->OffsetHigh.get()) << 32U);
            hFile->stream.clear();
            hFile->stream.seekg(streamOffset, std::ios::beg);

            if (hFile->stream.bad())
            {
                return FALSE;
            }
        }

        uint32_t numberOfBytesRead;
        hFile->stream.read((char *)(lpBuffer), nNumberOfBytesToRead);
        if (!hFile->stream.bad())
        {
            numberOfBytesRead = uint32_t(hFile->stream.gcount());
            result = TRUE;
        }

        if (result)
        {
            if (lpOverlapped != nullptr)
            {
                lpOverlapped->Internal = 0;
                lpOverlapped->InternalHigh = numberOfBytesRead;
            }
            else if (lpNumberOfBytesRead != nullptr)
            {
                *lpNumberOfBytesRead = numberOfBytesRead;
            }
        }

        Log::Info("ReadFile", "Read ", numberOfBytesRead, " bytes from file -> ", hFile->path, ".");

        return result;
    }

    uint32_t Hooks_SetFilePointer(FileHandle *hFile, int32_t lDistanceToMove, be<int32_t> *lpDistanceToMoveHigh, uint32_t dwMoveMethod)
    {
        int32_t distanceToMoveHigh = lpDistanceToMoveHigh ? lpDistanceToMoveHigh->get() : 0;
        std::streamoff streamOffset = lDistanceToMove + (std::streamoff(distanceToMoveHigh) << 32U);
        std::fstream::seekdir streamSeekDir = {};
        switch (dwMoveMethod)
        {
        case FILE_BEGIN:
            streamSeekDir = std::ios::beg;
            break;
        case FILE_CURRENT:
            streamSeekDir = std::ios::cur;
            break;
        case FILE_END:
            streamSeekDir = std::ios::end;
            break;
        default:
            assert(false && "Unknown move method.");
            break;
        }

        hFile->stream.clear();
        hFile->stream.seekg(streamOffset, streamSeekDir);
        if (hFile->stream.bad())
        {
            return INVALID_SET_FILE_POINTER;
        }

        std::streampos streamPos = hFile->stream.tellg();
        if (lpDistanceToMoveHigh != nullptr)
            *lpDistanceToMoveHigh = int32_t(streamPos >> 32U);

        return uint32_t(streamPos);
    }

    uint32_t Hooks_SetFilePointerEx(FileHandle *hFile, int32_t lDistanceToMove, LARGE_INTEGER *lpNewFilePointer, uint32_t dwMoveMethod)
    {
        std::fstream::seekdir streamSeekDir = {};
        switch (dwMoveMethod)
        {
        case FILE_BEGIN:
            streamSeekDir = std::ios::beg;
            break;
        case FILE_CURRENT:
            streamSeekDir = std::ios::cur;
            break;
        case FILE_END:
            streamSeekDir = std::ios::end;
            break;
        default:
            assert(false && "Unknown move method.");
            break;
        }

        hFile->stream.clear();
        hFile->stream.seekg(lDistanceToMove, streamSeekDir);
        if (hFile->stream.bad())
        {
            return FALSE;
        }

        if (lpNewFilePointer != nullptr)
        {
            lpNewFilePointer->QuadPart = ByteSwap(int64_t(hFile->stream.tellg()));
        }

        return TRUE;
    }

    uint32_t Hooks_WriteFile(FileHandle *hFile, const void *lpBuffer, uint32_t nNumberOfBytesToWrite, be<uint32_t> *lpNumberOfBytesWritten, void *lpOverlapped)
    {
        assert(lpOverlapped == nullptr && "Overlapped not implemented.");

        hFile->stream.write((const char *)(lpBuffer), nNumberOfBytesToWrite);
        if (hFile->stream.bad())
            return FALSE;

        if (lpNumberOfBytesWritten != nullptr)
            *lpNumberOfBytesWritten = uint32_t(hFile->stream.gcount());

        return TRUE;
    }

    BOOL Hooks_CreateDirectoryA(const char *lpPathName, void *lpSecurityAttributes)
    {
        Log::Info("CreateDirectoryA", "Creating directory -> ", lpPathName);

        std::string fileName(lpPathName);

        auto tempname = fileName.find("d:\\");

        if (tempname == std::string::npos)
        {
            tempname = fileName.find("d:\\");
        }

        if (tempname == std::string::npos) // not a game path
        {
            return false;
        }

        std::string actualPath = ResolvePath(fileName);

        return CreateDirectoryA(actualPath.c_str(), 0);
    }

    FindHandle *Import_FindFirstFileA(const char *lpFileName, WIN32_FIND_DATAA *lpFindFileData)
    {
        std::string_view path = lpFileName;

        if (path.find("\\*") == (path.size() - 2) || path.find("/*") == (path.size() - 2))
        {
            path.remove_suffix(1);
        }
        else if (path.find("\\*.*") == (path.size() - 4) || path.find("/*.*") == (path.size() - 4))
        {
            path.remove_suffix(3);
        }
        else
        {
            Log::Info("FindFirstFileA", "Searching for -> ", path);

            assert(!std::filesystem::path(path).has_extension() && "Unknown search pattern.");
        }

        FindHandle findHandle(path);

        if (findHandle.searchResult.empty())
            return GetInvalidKernelObject<FindHandle>();

        findHandle.fillFindData(lpFindFileData);

        return CreateKernelObject<FindHandle>(std::move(findHandle));
    }

    uint32_t Import_FindNextFileA(FindHandle *Handle, WIN32_FIND_DATAA *lpFindFileData)
    {
        Handle->iterator++;

        if (Handle->iterator == Handle->searchResult.end())
        {
            return FALSE;
        }
        else
        {
            Handle->fillFindData(lpFindFileData);
            return TRUE;
        }
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

GUEST_FUNCTION_HOOK(sub_82C74DB8, Hooks::Import_FindFirstFileA)
GUEST_FUNCTION_HOOK(sub_82C746A8, Hooks::Import_FindNextFileA)

GUEST_FUNCTION_HOOK(sub_82C74A90, Hooks::Hooks_WriteFile)
GUEST_FUNCTION_HOOK(sub_82C74908, Hooks::Hooks_ReadFile)
GUEST_FUNCTION_HOOK(sub_82C74F90, Hooks::Hooks_CreateFileA)

GUEST_FUNCTION_HOOK(sub_82C75230, Hooks::Hooks_GetFileSize)
GUEST_FUNCTION_HOOK(sub_82C78A98, Hooks::Hooks_GetFileSizeEx)

GUEST_FUNCTION_HOOK(sub_82C74C48, Hooks::Hooks_SetFilePointer)
GUEST_FUNCTION_HOOK(sub_82C78B60, Hooks::Hooks_SetFilePointerEx)

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
