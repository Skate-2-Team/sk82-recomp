#include "hooks.h"

namespace Hooks
{

    void Hooks_RtlOutputDebugString(_STRING *p_outputString)
    {
        Log::Info("Imports->RtlOutputDebugString", "Address is -> ", (void *)p_outputString);

        OutputDebugStringA((char *)p_outputString->Buffer);

        Log::Info("Imports->OutDebugString", p_outputString->Buffer);

        if (strstr((char *)p_outputString->Buffer, "runtime error"))
            DebugBreak();
    }

    void Hooks_XapiInitProcess()
    {
        Log::Info("XapiInitProcess", "Invoked");

        // All this function does is RtlCreateHeap for malloc and free etc,
        // But we already hook those, so we just need to set the heap to non-null
        // so the crt doesnt complain

        int *XapiProcessHeap = Memory::Translate<int *>(0x82E758E4);

        *XapiProcessHeap = 1;
    }

    void Import_RtlInitAnsiString(_STRING *p_destination, char *p_source)
    {
        const uint16_t l_length = p_source ? (uint16_t)strlen((char *)p_source) : 0;

        p_destination->Length = ByteSwap(l_length);
        p_destination->MaximumLength = ByteSwap(l_length + 1);
        p_destination->Buffer = (unsigned char *)p_source;
    }

    uint64_t Import_KeQueryPerformanceFrequency()
    {
        return 49875000;
    }

    uint32_t Import_ObReferenceObjectByHandle(uint32_t handle, uint32_t objectType, be<uint32_t> *object)
    {
        *object = handle;
        return 0;
    }

    uint32_t Import_NtClose(uint32_t handle)
    {
        if (handle == GUEST_INVALID_HANDLE_VALUE)
            return 0xFFFFFFFF;

        if (IsKernelObject(handle))
        {
            DestroyKernelObject(handle);
            return 0;
        }
        else
        {
            // assert(false && "Unrecognized kernel object.");
            return 0xFFFFFFFF;
        }
    }

    void Import_RtlFreeAnsiString()
    {
        Log::Stub("RtlFreeAnsiString", "Called.");
    }

    void Import_RtlUnicodeStringToAnsiString()
    {
        Log::Stub("RtlUnicodeStringToAnsiString", "Called.");
    }

    void Import_RtlInitUnicodeString()
    {
        Log::Stub("RtlInitUnicodeString", "Called.");
    }

    void Import_RtlMultiByteToUnicodeN()
    {
        Log::Stub("RtlMultiByteToUnicodeN", "Called.");
    }

    void Import_ExRegisterTitleTerminateNotification()
    {
        Log::Stub("ExRegisterTitleTerminateNotification", "Called.");
    }

    void Import_XMsgInProcessCall()
    {
        Log::Stub("XMsgInProcessCall", "Called.");
    }

    void Import_NetDll_WSAStartup()
    {
        Log::Stub("NetDll_WSAStartup", "Called.");
    }

    void Import_KeBugCheck()
    {
        Log::Stub("KeBugCheck", "Called.");
    }

    void Import_HalReturnToFirmware()
    {
        Log::Stub("HalReturnToFirmware", "Called.");
    }

    void Import_XexCheckExecutablePrivilege()
    {
        Log::Stub("XexCheckExecutablePrivilege", "Called.");
    }

    void Import_RtlNtStatusToDosError()
    {
        Log::Stub("RtlNtStatusToDosError", "Called.");
    }

    void Import_RtlUnicodeToMultiByteN()
    {
        Log::Stub("RtlUnicodeToMultiByteN", "Called.");
    }

    void Import_RtlImageXexHeaderField()
    {
        Log::Stub("RtlImageXexHeaderField", "Called.");
    }

    void Import_KeBugCheckEx()
    {
        Log::Stub("KeBugCheckEx", "Called.");
    }

    void Import_KeGetCurrentProcessType()
    {
        Log::Stub("KeGetCurrentProcessType", "Called.");
    }

    void Import_RtlRaiseException()
    {
        Log::Stub("RtlRaiseException", "Called.");
    }

    void Import_ExGetXConfigSetting()
    {
        Log::Stub("ExGetXConfigSetting", "Called.");
    }

    void Import_XexGetProcedureAddress()
    {
        Log::Stub("XexGetProcedureAddress", "Called.");
    }

    void Import_XexGetModuleSection()
    {
        Log::Stub("XexGetModuleSection", "Called.");
    }

    void Import_XexGetModuleHandle()
    {
        Log::Stub("XexGetModuleHandle", "Called.");
    }

    void Import_ObDereferenceObject()
    {
        Log::Stub("ObDereferenceObject", "Called.");
    }

    void Import_RtlTimeToTimeFields()
    {
        Log::Stub("RtlTimeToTimeFields", "Called.");
    }

    void Import_KeQuerySystemTime()
    {
        Log::Stub("KeQuerySystemTime", "Called.");
    }

    void Import_KiApcNormalRoutineNop()
    {
        Log::Stub("KiApcNormalRoutineNop", "Called.");
    }

    void Import_DbgPrint(char *buffer)
    {
        Log::Stub("DbgPrint", buffer);
    }

    void Import_DbgBreakPoint()
    {
        Log::Stub("DbgBreakPoint", "Called.");
        DebugBreak();
    }

    void Import_RtlUnwind()
    {
        Log::Stub("RtlUnwind", "Called.");
    }

    void Import_KeEnableFpuExceptions()
    {
        Log::Stub("KeEnableFpuExceptions", "Called.");
    }

    void Import_XMAReleaseContext()
    {
        Log::Stub("XMAReleaseContext", "Called.");
    }

    void Import_XMACreateContext()
    {
        Log::Stub("XMACreateContext", "Called.");
    }

    void Import_RtlCaptureContext()
    {
        Log::Stub("RtlCaptureContext", "Called.");
    }

    void Import_XAudioRegisterRenderDriverClient()
    {
        Log::Stub("XAudioRegisterRenderDriverClient", "Called.");
    }

    void Import_XAudioUnregisterRenderDriverClient()
    {
        Log::Stub("XAudioUnregisterRenderDriverClient", "Called.");
    }

    void Import_XAudioSubmitRenderDriverFrame()
    {
        Log::Stub("XAudioSubmitRenderDriverFrame", "Called.");
    }

    void Import_XAudioGetVoiceCategoryVolume()
    {
        Log::Stub("XAudioGetVoiceCategoryVolume", "Called.");
    }

    void Import_XAudioGetVoiceCategoryVolumeChangeMask()
    {
        Log::Stub("XAudioGetVoiceCategoryVolumeChangeMask", "Called.");
    }

    void Import_KeRaiseIrqlToDpcLevel()
    {
        Log::Stub("KeRaiseIrqlToDpcLevel", "Called.");
    }

    void Import_KfLowerIrql()
    {
        Log::Stub("KfLowerIrql", "Called.");
    }

    void Import_SPrintf()
    {
        Log::Stub("SPrintf", "Called.");
    }

    void Import_VSNPrintf()
    {
        Log::Stub("VSNPrintf", "Called.");
    }

    void Import_XAudioGetSpeakerConfig()
    {
        Log::Stub("XAudioGetSpeakerConfig", "Called.");
    }

    void Import_XMADisableContext()
    {
        Log::Stub("XMADisableContext", "Called.");
    }

    void Import_XMAGetOutputBufferWriteOffset()
    {
        Log::Stub("XMAGetOutputBufferWriteOffset", "Called.");
    }

    void Import_XMAInitializeContext()
    {
        Log::Stub("XMAInitializeContext", "Called.");
    }

    void Import_XMAIsInputBuffer0Valid()
    {
        Log::Stub("XMAIsInputBuffer0Valid", "Called.");
    }

    void Import_XMASetInputBuffer1()
    {
        Log::Stub("XMASetInputBuffer1", "Called.");
    }

    void Import_XNotifyDelayUI()
    {
        Log::Stub("XNotifyDelayUI", "Called.");
    }

    void Import_StfsControlDevice()
    {
        Log::Stub("StfsControlDevice", "Called.");
    }

    void Import_StfsCreateDevice()
    {
        Log::Stub("StfsCreateDevice", "Called.");
    }

    void Import_XMAEnableContext()
    {
        Log::Stub("XMAEnableContext", "Called.");
    }

    void Import_XMAGetOutputBufferReadOffset()
    {
        Log::Stub("XMAGetOutputBufferReadOffset", "Called.");
    }

    void Import_XMAIsInputBuffer1Valid()
    {
        Log::Stub("XMAIsInputBuffer1Valid", "Called.");
    }

    void Import_XMAIsOutputBufferValid()
    {
        Log::Stub("XMAIsOutputBufferValid", "Called.");
    }

    void Import_XMASetInputBuffer0()
    {
        Log::Stub("XMASetInputBuffer0", "Called.");
    }

    void Import_XMASetInputBuffer0Valid()
    {
        Log::Stub("XMASetInputBuffer0Valid", "Called.");
    }

    void Import_XMASetInputBuffer1Valid()
    {
        Log::Stub("XMASetInputBuffer1Valid", "Called.");
    }

    void Import_XMASetInputBufferReadOffset()
    {
        Log::Stub("XMASetInputBufferReadOffset", "Called.");
    }

    void Import_XMASetOutputBufferReadOffset()
    {
        Log::Stub("XMASetOutputBufferReadOffset", "Called.");
    }

    void Import_XMASetOutputBufferValid()
    {
        Log::Stub("XMASetOutputBufferValid", "Called.");
    }

    void Import_XeCryptSha()
    {
        Log::Stub("XeCryptSha", "Called.");
    }

    void Import_XeKeysConsolePrivateKeySign()
    {
        Log::Stub("XeKeysConsolePrivateKeySign", "Called.");
    }

    void Import_XeKeysConsoleSignatureVerification()
    {
        Log::Stub("XeKeysConsoleSignatureVerification", "Called.");
    }

    void Import__snprintf()
    {
        Log::Stub("_snprintf", "Called.");
    }

    void Import_FscSetCacheElementCount()
    {
        Log::Stub("FscSetCacheElementCount", "Called.");
    }

    void Import_IoCheckShareAccess()
    {
        Log::Stub("IoCheckShareAccess", "Called.");
    }

    void Import_IoCompleteRequest()
    {
        Log::Stub("IoCompleteRequest", "Called.");
    }

    void Import_IoCreateDevice()
    {
        Log::Stub("IoCreateDevice", "Called.");
    }

    void Import_IoDeleteDevice()
    {
        Log::Stub("IoDeleteDevice", "Called.");
    }

    void Import_IoInvalidDeviceRequest()
    {
        Log::Stub("IoInvalidDeviceRequest", "Called.");
    }

    void Import_IoRemoveShareAccess()
    {
        Log::Stub("IoRemoveShareAccess", "Called.");
    }

    void Import_IoSetShareAccess()
    {
        Log::Stub("IoSetShareAccess", "Called.");
    }

    void Import_NtDeviceIoControlFile()
    {
        Log::Stub("NtDeviceIoControlFile", "Called.");
    }

    void Import_ObIsTitleObject()
    {
        Log::Stub("ObIsTitleObject", "Called.");
    }

    void Import_ObReferenceObject()
    {
        Log::Stub("ObReferenceObject", "Called.");
    }

    void Import_RtlCompareStringN()
    {
        Log::Stub("RtlCompareStringN", "Called.");
    }

    void Import_RtlTimeFieldsToTime()
    {
        Log::Stub("RtlTimeFieldsToTime", "Called.");
    }

    void Import_RtlUpcaseUnicodeChar()
    {
        Log::Stub("RtlUpcaseUnicodeChar", "Called.");
    }

    void Import_NetDll_XNetCleanup()
    {
        Log::Stub("NetDll_XNetCleanup", "Called.");
    }

    void Import_NetDll_XNetConnect()
    {
        Log::Stub("NetDll_XNetConnect", "Called.");
    }

    void Import_NetDll_XNetCreateKey()
    {
        Log::Stub("NetDll_XNetCreateKey", "Called.");
    }

    void Import_NetDll_XNetDnsLookup()
    {
        Log::Stub("NetDll_XNetDnsLookup", "Called.");
    }

    void Import_NetDll_XNetDnsRelease()
    {
        Log::Stub("NetDll_XNetDnsRelease", "Called.");
    }

    void Import_NetDll_XNetGetConnectStatus()
    {
        Log::Stub("NetDll_XNetGetConnectStatus", "Called.");
    }

    void Import_NetDll_XNetGetTitleXnAddr()
    {
        Log::Stub("NetDll_XNetGetTitleXnAddr", "Called.");
    }

    void Import_NetDll_XNetInAddrToXnAddr()
    {
        Log::Stub("NetDll_XNetInAddrToXnAddr", "Called.");
    }

    void Import_NetDll_XNetQosListen()
    {
        Log::Stub("NetDll_XNetQosListen", "Called.");
    }

    void Import_NetDll_XNetQosLookup()
    {
        Log::Stub("NetDll_XNetQosLookup", "Called.");
    }

    void Import_NetDll_XNetQosRelease()
    {
        Log::Stub("NetDll_XNetQosRelease", "Called.");
    }

    void Import_NetDll_XNetQosServiceLookup()
    {
        Log::Stub("NetDll_XNetQosServiceLookup", "Called.");
    }

    void Import_NetDll_XNetRandom()
    {
        Log::Stub("NetDll_XNetRandom", "Called.");
    }

    void Import_NetDll_XNetRegisterKey()
    {
        Log::Stub("NetDll_XNetRegisterKey", "Called.");
    }

    void Import_NetDll_XNetServerToInAddr()
    {
        Log::Stub("NetDll_XNetServerToInAddr", "Called.");
    }

    void Import_NetDll_XNetStartup()
    {
        Log::Stub("NetDll_XNetStartup", "Called.");
    }

    void Import_NetDll_XNetUnregisterKey()
    {
        Log::Stub("NetDll_XNetUnregisterKey", "Called.");
    }

    void Import_NetDll_XNetXnAddrToInAddr()
    {
        Log::Stub("NetDll_XNetXnAddrToInAddr", "Called.");
    }

    void Import_NetDll_WSACleanup()
    {
        Log::Stub("NetDll_WSACleanup", "Called.");
    }

    void Import_NetDll_WSAGetOverlappedResult()
    {
        Log::Stub("NetDll_WSAGetOverlappedResult", "Called.");
    }

    void Import_NetDll_WSARecv()
    {
        Log::Stub("NetDll_WSARecv", "Called.");
    }

    void Import_NetDll_WSARecvFrom()
    {
        Log::Stub("NetDll_WSARecvFrom", "Called.");
    }

    void Import_NetDll_XNetGetEthernetLinkStatus()
    {
        Log::Stub("NetDll_XNetGetEthernetLinkStatus", "Called.");
    }

    void Import_NetDll_accept()
    {
        Log::Stub("NetDll_accept", "Called.");
    }

    void Import_NetDll_bind()
    {
        Log::Stub("NetDll_bind", "Called.");
    }

    void Import_NetDll_closesocket()
    {
        Log::Stub("NetDll_closesocket", "Called.");
    }

    void Import_NetDll_connect()
    {
        Log::Stub("NetDll_connect", "Called.");
    }

    void Import_NetDll_getpeername()
    {
        Log::Stub("NetDll_getpeername", "Called.");
    }

    void Import_NetDll_getsockname()
    {
        Log::Stub("NetDll_getsockname", "Called.");
    }

    void Import_NetDll_getsockopt()
    {
        Log::Stub("NetDll_getsockopt", "Called.");
    }

    void Import_NetDll_ioctlsocket()
    {
        Log::Stub("NetDll_ioctlsocket", "Called.");
    }

    void Import_NetDll_listen()
    {
        Log::Stub("NetDll_listen", "Called.");
    }

    void Import_NetDll_recv()
    {
        Log::Stub("NetDll_recv", "Called.");
    }

    void Import_NetDll_recvfrom()
    {
        Log::Stub("NetDll_recvfrom", "Called.");
    }

    void Import_NetDll_select()
    {
        Log::Stub("NetDll_select", "Called.");
    }

    void Import_NetDll_setsockopt()
    {
        Log::Stub("NetDll_setsockopt", "Called.");
    }

    void Import_NetDll_shutdown()
    {
        Log::Stub("NetDll_shutdown", "Called.");
    }

    void Import_NetDll_socket()
    {
        Log::Stub("NetDll_socket", "Called.");
    }

    void Import_CurlOpenTitleBackingFile()
    {
        Log::Stub("CurlOpenTitleBackingFile", "Called.");
    }

    void Import_NetDll_WSACloseEvent()
    {
        Log::Stub("NetDll_WSACloseEvent", "Called.");
    }

    void Import_NetDll_WSACreateEvent()
    {
        Log::Stub("NetDll_WSACreateEvent", "Called.");
    }

    void Import_NetDll_WSAGetLastError()
    {
        Log::Stub("NetDll_WSAGetLastError", "Called.");
    }

    void Import_NetDll_WSAResetEvent()
    {
        Log::Stub("NetDll_WSAResetEvent", "Called.");
    }

    void Import_NetDll_WSASetEvent()
    {
        Log::Stub("NetDll_WSASetEvent", "Called.");
    }

    void Import_NetDll_WSAWaitForMultipleEvents()
    {
        Log::Stub("NetDll_WSAWaitForMultipleEvents", "Called.");
    }

    void Import_NetDll___WSAFDIsSet()
    {
        Log::Stub("NetDll___WSAFDIsSet", "Called.");
    }

    void Import_NetDll_inet_addr()
    {
        Log::Stub("NetDll_inet_addr", "Called.");
    }

    void Import_NetDll_send()
    {
        Log::Stub("NetDll_send", "Called.");
    }

    void Import_NetDll_sendto()
    {
        Log::Stub("NetDll_sendto", "Called.");
    }

    void Import_RtlComputeCrc32()
    {
        Log::Stub("RtlComputeCrc32", "Called.");
    }

    void Import_XMsgStartIORequestEx()
    {
        Log::Stub("XMsgStartIORequestEx", "Called.");
    }

    void Import_XNetLogonGetMachineID()
    {
        Log::Stub("XNetLogonGetMachineID", "Called.");
    }

    void Import_XNetLogonGetTitleID()
    {
        Log::Stub("XNetLogonGetTitleID", "Called.");
    }

    void Import_XeCryptMd5Final()
    {
        Log::Stub("XeCryptMd5Final", "Called.");
    }

    void Import_XeCryptMd5Init()
    {
        Log::Stub("XeCryptMd5Init", "Called.");
    }

    void Import_XeCryptMd5Update()
    {
        Log::Stub("XeCryptMd5Update", "Called.");
    }

    void Import_XeCryptShaFinal()
    {
        Log::Stub("XeCryptShaFinal", "Called.");
    }

    void Import_XeCryptShaInit()
    {
        Log::Stub("XeCryptShaInit", "Called.");
    }

    void Import_XeCryptShaUpdate()
    {
        Log::Stub("XeCryptShaUpdate", "Called.");
    }
}

GUEST_FUNCTION_STUB(sub_82C69308) // NetTick

GUEST_FUNCTION_HOOK(sub_82C72188, Hooks::Hooks_RtlOutputDebugString)
GUEST_FUNCTION_HOOK(sub_82C7A598, Hooks::Hooks_XapiInitProcess)

GUEST_FUNCTION_HOOK(__imp__XeCryptMd5Init, Hooks::Import_XeCryptMd5Init)
GUEST_FUNCTION_HOOK(__imp__XeCryptMd5Update, Hooks::Import_XeCryptMd5Update)
GUEST_FUNCTION_HOOK(__imp__XeCryptShaFinal, Hooks::Import_XeCryptShaFinal)
GUEST_FUNCTION_HOOK(__imp__XeCryptShaInit, Hooks::Import_XeCryptShaInit)
GUEST_FUNCTION_HOOK(__imp__XeCryptShaUpdate, Hooks::Import_XeCryptShaUpdate)
GUEST_FUNCTION_HOOK(__imp__XeCryptMd5Final, Hooks::Import_XeCryptMd5Final)

GUEST_FUNCTION_HOOK(__imp__CurlOpenTitleBackingFile, Hooks::Import_CurlOpenTitleBackingFile)
GUEST_FUNCTION_HOOK(__imp__NetDll_WSACloseEvent, Hooks::Import_NetDll_WSACloseEvent)
GUEST_FUNCTION_HOOK(__imp__NetDll_WSACreateEvent, Hooks::Import_NetDll_WSACreateEvent)
GUEST_FUNCTION_HOOK(__imp__NetDll_WSAGetLastError, Hooks::Import_NetDll_WSAGetLastError)
GUEST_FUNCTION_HOOK(__imp__NetDll_WSAResetEvent, Hooks::Import_NetDll_WSAResetEvent)
GUEST_FUNCTION_HOOK(__imp__NetDll_WSASetEvent, Hooks::Import_NetDll_WSASetEvent)
GUEST_FUNCTION_HOOK(__imp__NetDll_WSAWaitForMultipleEvents, Hooks::Import_NetDll_WSAWaitForMultipleEvents)
GUEST_FUNCTION_HOOK(__imp__NetDll___WSAFDIsSet, Hooks::Import_NetDll___WSAFDIsSet)
GUEST_FUNCTION_HOOK(__imp__NetDll_inet_addr, Hooks::Import_NetDll_inet_addr)
GUEST_FUNCTION_HOOK(__imp__NetDll_send, Hooks::Import_NetDll_send)
GUEST_FUNCTION_HOOK(__imp__NetDll_sendto, Hooks::Import_NetDll_sendto)
GUEST_FUNCTION_HOOK(__imp__RtlComputeCrc32, Hooks::Import_RtlComputeCrc32)
GUEST_FUNCTION_HOOK(__imp__XMsgStartIORequestEx, Hooks::Import_XMsgStartIORequestEx)
GUEST_FUNCTION_HOOK(__imp__XNetLogonGetMachineID, Hooks::Import_XNetLogonGetMachineID)
GUEST_FUNCTION_HOOK(__imp__XNetLogonGetTitleID, Hooks::Import_XNetLogonGetTitleID)

GUEST_FUNCTION_HOOK(__imp__NetDll_WSACleanup, Hooks::Import_NetDll_WSACleanup)
GUEST_FUNCTION_HOOK(__imp__NetDll_WSAGetOverlappedResult, Hooks::Import_NetDll_WSAGetOverlappedResult)
GUEST_FUNCTION_HOOK(__imp__NetDll_WSARecv, Hooks::Import_NetDll_WSARecv)
GUEST_FUNCTION_HOOK(__imp__NetDll_WSARecvFrom, Hooks::Import_NetDll_WSARecvFrom)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetGetEthernetLinkStatus, Hooks::Import_NetDll_XNetGetEthernetLinkStatus)
GUEST_FUNCTION_HOOK(__imp__NetDll_accept, Hooks::Import_NetDll_accept)
GUEST_FUNCTION_HOOK(__imp__NetDll_bind, Hooks::Import_NetDll_bind)
GUEST_FUNCTION_HOOK(__imp__NetDll_closesocket, Hooks::Import_NetDll_closesocket)
GUEST_FUNCTION_HOOK(__imp__NetDll_connect, Hooks::Import_NetDll_connect)
GUEST_FUNCTION_HOOK(__imp__NetDll_getpeername, Hooks::Import_NetDll_getpeername)
GUEST_FUNCTION_HOOK(__imp__NetDll_getsockname, Hooks::Import_NetDll_getsockname)
GUEST_FUNCTION_HOOK(__imp__NetDll_getsockopt, Hooks::Import_NetDll_getsockopt)
GUEST_FUNCTION_HOOK(__imp__NetDll_ioctlsocket, Hooks::Import_NetDll_ioctlsocket)
GUEST_FUNCTION_HOOK(__imp__NetDll_listen, Hooks::Import_NetDll_listen)
GUEST_FUNCTION_HOOK(__imp__NetDll_recv, Hooks::Import_NetDll_recv)
GUEST_FUNCTION_HOOK(__imp__NetDll_recvfrom, Hooks::Import_NetDll_recvfrom)
GUEST_FUNCTION_HOOK(__imp__NetDll_select, Hooks::Import_NetDll_select)
GUEST_FUNCTION_HOOK(__imp__NetDll_setsockopt, Hooks::Import_NetDll_setsockopt)
GUEST_FUNCTION_HOOK(__imp__NetDll_shutdown, Hooks::Import_NetDll_shutdown)
GUEST_FUNCTION_HOOK(__imp__NetDll_socket, Hooks::Import_NetDll_socket)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetCleanup, Hooks::Import_NetDll_XNetCleanup)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetConnect, Hooks::Import_NetDll_XNetConnect)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetCreateKey, Hooks::Import_NetDll_XNetCreateKey)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetDnsLookup, Hooks::Import_NetDll_XNetDnsLookup)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetDnsRelease, Hooks::Import_NetDll_XNetDnsRelease)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetGetConnectStatus, Hooks::Import_NetDll_XNetGetConnectStatus)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetGetTitleXnAddr, Hooks::Import_NetDll_XNetGetTitleXnAddr)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetInAddrToXnAddr, Hooks::Import_NetDll_XNetInAddrToXnAddr)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetQosListen, Hooks::Import_NetDll_XNetQosListen)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetQosLookup, Hooks::Import_NetDll_XNetQosLookup)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetQosRelease, Hooks::Import_NetDll_XNetQosRelease)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetQosServiceLookup, Hooks::Import_NetDll_XNetQosServiceLookup)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetRandom, Hooks::Import_NetDll_XNetRandom)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetRegisterKey, Hooks::Import_NetDll_XNetRegisterKey)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetServerToInAddr, Hooks::Import_NetDll_XNetServerToInAddr)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetStartup, Hooks::Import_NetDll_XNetStartup)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetUnregisterKey, Hooks::Import_NetDll_XNetUnregisterKey)
GUEST_FUNCTION_HOOK(__imp__NetDll_XNetXnAddrToInAddr, Hooks::Import_NetDll_XNetXnAddrToInAddr)

GUEST_FUNCTION_HOOK(__imp__ObIsTitleObject, Hooks::Import_ObIsTitleObject)
GUEST_FUNCTION_HOOK(__imp__ObReferenceObject, Hooks::Import_ObReferenceObject)
GUEST_FUNCTION_HOOK(__imp__RtlCompareStringN, Hooks::Import_RtlCompareStringN)
GUEST_FUNCTION_HOOK(__imp__RtlTimeFieldsToTime, Hooks::Import_RtlTimeFieldsToTime)
GUEST_FUNCTION_HOOK(__imp__RtlUpcaseUnicodeChar, Hooks::Import_RtlUpcaseUnicodeChar)

GUEST_FUNCTION_HOOK(__imp__IoCheckShareAccess, Hooks::Import_IoCheckShareAccess)
GUEST_FUNCTION_HOOK(__imp__IoCompleteRequest, Hooks::Import_IoCompleteRequest)
GUEST_FUNCTION_HOOK(__imp__IoCreateDevice, Hooks::Import_IoCreateDevice)
GUEST_FUNCTION_HOOK(__imp__IoDeleteDevice, Hooks::Import_IoDeleteDevice)
GUEST_FUNCTION_HOOK(__imp__IoInvalidDeviceRequest, Hooks::Import_IoInvalidDeviceRequest)
GUEST_FUNCTION_HOOK(__imp__IoRemoveShareAccess, Hooks::Import_IoRemoveShareAccess)
GUEST_FUNCTION_HOOK(__imp__IoSetShareAccess, Hooks::Import_IoSetShareAccess)

GUEST_FUNCTION_HOOK(__imp__FscSetCacheElementCount, Hooks::Import_FscSetCacheElementCount)
GUEST_FUNCTION_HOOK(__imp__StfsControlDevice, Hooks::Import_StfsControlDevice)
GUEST_FUNCTION_HOOK(__imp__StfsCreateDevice, Hooks::Import_StfsCreateDevice)
GUEST_FUNCTION_HOOK(__imp__XMAEnableContext, Hooks::Import_XMAEnableContext)
GUEST_FUNCTION_HOOK(__imp__XMAGetOutputBufferReadOffset, Hooks::Import_XMAGetOutputBufferReadOffset)
GUEST_FUNCTION_HOOK(__imp__XMAIsInputBuffer1Valid, Hooks::Import_XMAIsInputBuffer1Valid)
GUEST_FUNCTION_HOOK(__imp__XMAIsOutputBufferValid, Hooks::Import_XMAIsOutputBufferValid)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBuffer0, Hooks::Import_XMASetInputBuffer0)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBuffer0Valid, Hooks::Import_XMASetInputBuffer0Valid)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBuffer1Valid, Hooks::Import_XMASetInputBuffer1Valid)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBufferReadOffset, Hooks::Import_XMASetInputBufferReadOffset)
GUEST_FUNCTION_HOOK(__imp__XMASetOutputBufferReadOffset, Hooks::Import_XMASetOutputBufferReadOffset)
GUEST_FUNCTION_HOOK(__imp__XMASetOutputBufferValid, Hooks::Import_XMASetOutputBufferValid)
GUEST_FUNCTION_HOOK(__imp__XeCryptSha, Hooks::Import_XeCryptSha)
GUEST_FUNCTION_HOOK(__imp__XeKeysConsolePrivateKeySign, Hooks::Import_XeKeysConsolePrivateKeySign)
GUEST_FUNCTION_HOOK(__imp__XeKeysConsoleSignatureVerification, Hooks::Import_XeKeysConsoleSignatureVerification)
GUEST_FUNCTION_HOOK(__imp___snprintf, Hooks::Import__snprintf)

GUEST_FUNCTION_HOOK(__imp__NtDeviceIoControlFile, Hooks::Import_NtDeviceIoControlFile)
GUEST_FUNCTION_HOOK(__imp__NtClose, Hooks::Import_NtClose)

GUEST_FUNCTION_HOOK(__imp__KeBugCheck, Hooks::Import_KeBugCheck)
GUEST_FUNCTION_HOOK(__imp__KeBugCheckEx, Hooks::Import_KeBugCheckEx)
GUEST_FUNCTION_HOOK(__imp__KeQueryPerformanceFrequency, Hooks::Import_KeQueryPerformanceFrequency)
GUEST_FUNCTION_HOOK(__imp__KeGetCurrentProcessType, Hooks::Import_KeGetCurrentProcessType)
GUEST_FUNCTION_HOOK(__imp__KeRaiseIrqlToDpcLevel, Hooks::Import_KeRaiseIrqlToDpcLevel)
GUEST_FUNCTION_HOOK(__imp__KeQuerySystemTime, Hooks::Import_KeQuerySystemTime)
GUEST_FUNCTION_HOOK(__imp__KeEnableFpuExceptions, Hooks::Import_KeEnableFpuExceptions)
GUEST_FUNCTION_HOOK(__imp__KiApcNormalRoutineNop, Hooks::Import_KiApcNormalRoutineNop)
GUEST_FUNCTION_HOOK(__imp__KfLowerIrql, Hooks::Import_KfLowerIrql)

GUEST_FUNCTION_HOOK(__imp__XexCheckExecutablePrivilege, Hooks::Import_XexCheckExecutablePrivilege)
GUEST_FUNCTION_HOOK(__imp__XexGetProcedureAddress, Hooks::Import_XexGetProcedureAddress)
GUEST_FUNCTION_HOOK(__imp__XexGetModuleSection, Hooks::Import_XexGetModuleSection)
GUEST_FUNCTION_HOOK(__imp__XexGetModuleHandle, Hooks::Import_XexGetModuleHandle)

GUEST_FUNCTION_HOOK(__imp__XAudioGetSpeakerConfig, Hooks::Import_XAudioGetSpeakerConfig)
GUEST_FUNCTION_HOOK(__imp__XNotifyDelayUI, Hooks::Import_XNotifyDelayUI)
GUEST_FUNCTION_HOOK(__imp__XAudioRegisterRenderDriverClient, Hooks::Import_XAudioRegisterRenderDriverClient)
GUEST_FUNCTION_HOOK(__imp__XAudioUnregisterRenderDriverClient, Hooks::Import_XAudioUnregisterRenderDriverClient)
GUEST_FUNCTION_HOOK(__imp__XAudioSubmitRenderDriverFrame, Hooks::Import_XAudioSubmitRenderDriverFrame)
GUEST_FUNCTION_HOOK(__imp__XAudioGetVoiceCategoryVolume, Hooks::Import_XAudioGetVoiceCategoryVolume)
GUEST_FUNCTION_HOOK(__imp__XAudioGetVoiceCategoryVolumeChangeMask, Hooks::Import_XAudioGetVoiceCategoryVolumeChangeMask)

GUEST_FUNCTION_HOOK(__imp__RtlNtStatusToDosError, Hooks::Import_RtlNtStatusToDosError)
GUEST_FUNCTION_HOOK(__imp__RtlUnicodeToMultiByteN, Hooks::Import_RtlUnicodeToMultiByteN)
GUEST_FUNCTION_HOOK(__imp__RtlImageXexHeaderField, Hooks::Import_RtlImageXexHeaderField)
GUEST_FUNCTION_HOOK(__imp__RtlRaiseException, Hooks::Import_RtlRaiseException)
GUEST_FUNCTION_HOOK(__imp__RtlTimeToTimeFields, Hooks::Import_RtlTimeToTimeFields)
GUEST_FUNCTION_HOOK(__imp__RtlInitAnsiString, Hooks::Import_RtlInitAnsiString)
GUEST_FUNCTION_HOOK(__imp__RtlFreeAnsiString, Hooks::Import_RtlFreeAnsiString)
GUEST_FUNCTION_HOOK(__imp__RtlUnicodeStringToAnsiString, Hooks::Import_RtlUnicodeStringToAnsiString)
GUEST_FUNCTION_HOOK(__imp__RtlInitUnicodeString, Hooks::Import_RtlInitUnicodeString)
GUEST_FUNCTION_HOOK(__imp__RtlMultiByteToUnicodeN, Hooks::Import_RtlMultiByteToUnicodeN)
GUEST_FUNCTION_HOOK(__imp__RtlCaptureContext, Hooks::Import_RtlCaptureContext)
GUEST_FUNCTION_HOOK(__imp__RtlUnwind, Hooks::Import_RtlUnwind)

GUEST_FUNCTION_HOOK(__imp__XMADisableContext, Hooks::Import_XMADisableContext)
GUEST_FUNCTION_HOOK(__imp__XMAGetOutputBufferWriteOffset, Hooks::Import_XMAGetOutputBufferWriteOffset)
GUEST_FUNCTION_HOOK(__imp__XMAInitializeContext, Hooks::Import_XMAInitializeContext)
GUEST_FUNCTION_HOOK(__imp__XMAIsInputBuffer0Valid, Hooks::Import_XMAIsInputBuffer0Valid)
GUEST_FUNCTION_HOOK(__imp__XMASetInputBuffer1, Hooks::Import_XMASetInputBuffer1)
GUEST_FUNCTION_HOOK(__imp__XMAReleaseContext, Hooks::Import_XMAReleaseContext)
GUEST_FUNCTION_HOOK(__imp__XMACreateContext, Hooks::Import_XMACreateContext)

GUEST_FUNCTION_HOOK(__imp__ObDereferenceObject, Hooks::Import_ObDereferenceObject)
GUEST_FUNCTION_HOOK(__imp__ObReferenceObjectByHandle, Hooks::Import_ObReferenceObjectByHandle)

GUEST_FUNCTION_HOOK(__imp__XMsgInProcessCall, Hooks::Import_XMsgInProcessCall)
GUEST_FUNCTION_HOOK(__imp__HalReturnToFirmware, Hooks::Import_HalReturnToFirmware)

GUEST_FUNCTION_HOOK(__imp__ExRegisterTitleTerminateNotification, Hooks::Import_ExRegisterTitleTerminateNotification)
GUEST_FUNCTION_HOOK(__imp__ExGetXConfigSetting, Hooks::Import_ExGetXConfigSetting)

GUEST_FUNCTION_HOOK(__imp__NetDll_WSAStartup, Hooks::Import_NetDll_WSAStartup)

GUEST_FUNCTION_HOOK(__imp__DbgPrint, Hooks::Import_DbgPrint)
GUEST_FUNCTION_HOOK(__imp__DbgBreakPoint, Hooks::Import_DbgBreakPoint)
GUEST_FUNCTION_HOOK(__imp__sprintf, Hooks::Import_SPrintf)
GUEST_FUNCTION_HOOK(__imp___vsnprintf, Hooks::Import_VSNPrintf)

// this should be removed from recomp
GUEST_FUNCTION_STUB(__imp____C_specific_handler)
