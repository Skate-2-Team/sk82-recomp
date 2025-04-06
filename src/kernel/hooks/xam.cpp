#include "hooks.h"

namespace Hooks
{
    static std::unordered_set<XamListener *> gListeners{};
    static uint64_t gXUID = 0xB13EBABEBABEBABE;

    XamListener::XamListener()
    {
        Log::Info("XamListener", "Created listener: ", this);
        gListeners.insert(this);
    }

    XamListener::~XamListener()
    {
        gListeners.erase(this);
    }

    void Import_XamContentCreateEx()
    {
        Log::Stub("XamContentCreateEx", "Called.");
    }

    void Import_XamContentClose()
    {
        Log::Stub("XamContentClose", "Called.");
    }

    void Import_XamContentGetLicenseMask()
    {
        Log::Stub("XamContentGetLicenseMask", "Called.");
    }

    void Import_XamContentCreateEnumerator()
    {
        Log::Stub("XamContentCreateEnumerator", "Called.");
    }

    uint32_t Import_XamContentGetDeviceData(uint32_t DeviceID, XDEVICE_DATA *pDeviceData)
    {
        pDeviceData->DeviceID = DeviceID;
        pDeviceData->DeviceType = XCONTENTDEVICETYPE_HDD;
        pDeviceData->ulDeviceBytes = 0x10000000;
        pDeviceData->ulDeviceFreeBytes = 0x10000000;
        pDeviceData->wszName[0] = 'S';
        pDeviceData->wszName[1] = 'k';
        pDeviceData->wszName[2] = 'a';
        pDeviceData->wszName[3] = 't';
        pDeviceData->wszName[4] = 'e';
        pDeviceData->wszName[5] = '\0';

        return 0;
    }

    void Import_XamEnumerate()
    {
        Log::Stub("XamEnumerate", "Called.");
    }

    void Import_XamShowSigninUI()
    {
        Log::Info("XamShowSigninUI", "Called.");
    }

    void Import_XamShowGamerCardUIForXUID()
    {
        Log::Stub("XamShowGamerCardUIForXUID", "Called.");
    }

    void Import_XamShowAchievementsUI()
    {
        Log::Stub("XamShowAchievementsUI", "Called.");
    }

    void Import_XamShowMarketplaceUI()
    {
        Log::Stub("XamShowMarketplaceUI", "Called.");
    }

    void Import_XamShowDeviceSelectorUI()
    {
        Log::Stub("XamShowDeviceSelectorUI", "Called.");
    }

    void Import_XamShowDirtyDiscErrorUI()
    {
        Log::Stub("XamShowDirtyDiscErrorUI", "Called.");
    }

    void Import_XamUserGetName()
    {
        Log::Stub("XamUserGetName", "Called.");
    }

    uint32_t Import_XamUserGetSigninState(uint32_t userIndex)
    {
        return true;
    }

    void Import_XamUserCheckPrivilege()
    {
        Log::Stub("XamUserCheckPrivilege", "Called.");
    }

    uint32_t Import_XamGetSystemVersion()
    {
        return 0;
    }

    void Import_XamUserCreateStatsEnumerator()
    {
        Log::Stub("XamUserCreateStatsEnumerator", "Called.");
    }

    DWORD Import_XamUserGetXUID(DWORD dwUserIndex, uint64_t *pXuid)
    {
        /*
        if (pXuid != nullptr)
        {
            *pXuid = ByteSwap(gXUID);
        }*/

        return S_OK;
    }

    void Import_XamEnableInactivityProcessing()
    {
        Log::Stub("XamEnableInactivityProcessing", "Called.");
    }

    void Import_XamResetInactivity()
    {
        Log::Stub("XamResetInactivity", "Called.");
    }

    void Import_XamLoaderLaunchTitle()
    {
        Log::Stub("XamLoaderLaunchTitle", "Called.");
    }

    void Import_XamShowMessageBoxUIEx()
    {
        Log::Stub("XamShowMessageBoxUIEx", "Called.");
    }

    void Import_XamLoaderTerminateTitle()
    {
        Log::Stub("XamLoaderTerminateTitle", "Called.");
    }

    void Import_XamGetExecutionId()
    {
        Log::Stub("XamGetExecutionId", "Called.");
    }

    void Import_XamUserReadProfileSettings(
        uint32_t titleId,
        uint32_t userIndex,
        uint32_t xuidCount,
        uint64_t *xuids,
        uint32_t settingCount,
        uint32_t *settingIds,
        be<uint32_t> *bufferSize,
        void *buffer,
        void *overlapped)
    {
        if (buffer != nullptr)
        {
            memset(buffer, 0, *bufferSize);
        }
        else
        {
            *bufferSize = 4;
        }
    }

    void Import_XamUserWriteProfileSettings()
    {
        Log::Stub("XamUserWriteProfileSettings", "Called.");
    }

    void Import_XamFree()
    {
        Log::Stub("XamFree", "Called.");
    }

    void Import_XamAlloc()
    {
        Log::Stub("XamAlloc", "Called.");
    }

    void Import_XamSessionRefObjByHandle()
    {
        Log::Stub("XamSessionRefObjByHandle", "Called.");
    }

    void Import_XamSessionCreateHandle()
    {
        Log::Stub("XamSessionCreateHandle", "Called.");
    }

    void Import_XNotifyPositionUI()
    {
        Log::Stub("XNotifyPositionUI", "Called.");
    }

    uint32_t Import_XamNotifyCreateListener(uint64_t qwAreas)
    {
        auto *listener = CreateKernelObject<XamListener>();

        listener->areas = qwAreas;

        return GetKernelHandle(listener);
    }

    void XamNotifyEnqueueEvent(uint32_t dwId, uint32_t dwParam)
    {
        for (const auto &listener : gListeners)
        {
            if (((1 << MSG_AREA(dwId)) & listener->areas) == 0)
                continue;

            listener->notifications.emplace_back(dwId, dwParam);
        }
    }

    bool Import_XNotifyGetNext(uint32_t hNotification, uint32_t dwMsgFilter, be<uint32_t> *pdwId, be<uint32_t> *pParam)
    {
        auto &listener = *GetKernelObject<XamListener>(hNotification);

        if (dwMsgFilter)
        {
            for (size_t i = 0; i < listener.notifications.size(); i++)
            {
                if (std::get<0>(listener.notifications[i]) == dwMsgFilter)
                {
                    if (pdwId)
                        *pdwId = std::get<0>(listener.notifications[i]);

                    if (pParam)
                        *pParam = std::get<1>(listener.notifications[i]);

                    listener.notifications.erase(listener.notifications.begin() + i);

                    return true;
                }
            }
        }
        else
        {
            if (listener.notifications.empty())
                return false;

            if (pdwId)
                *pdwId = std::get<0>(listener.notifications[0]);

            if (pParam)
                *pParam = std::get<1>(listener.notifications[0]);

            listener.notifications.erase(listener.notifications.begin());

            return true;
        }

        return false;
    }

    void Import_XMsgStartIORequest()
    {
        Log::Stub("XMsgStartIORequest", "Called.");
    }

    uint32_t Import_XGetGameRegion()
    {
        return 0x03FF;
    }

    void Import_XMsgCancelIORequest()
    {
        Log::Stub("XMsgCancelIORequest", "Called.");
    }

    uint32_t Import_XGetLanguage()
    {
        return 1;
    }

    uint32_t Import_XGetAVPack()
    {
        return 0;
    }

    void Import_XamShowKeyboardUI()
    {
        Log::Stub("XamShowKeyboardUI", "Called.");
    }

    void Import_XamShowPlayerReviewUI()
    {
        Log::Stub("XamShowPlayerReviewUI", "Called.");
    }

    void Import_XamShowGameInviteUI()
    {
        Log::Stub("XamShowGameInviteUI", "Called.");
    }

    void Import_XamShowMessageComposeUI()
    {
        Log::Stub("XamShowMessageComposeUI", "Called.");
    }

    void Import_XamShowFriendRequestUI()
    {
        Log::Stub("XamShowFriendRequestUI", "Called.");
    }

    void Import_XamTaskShouldExit()
    {
        Log::Stub("XamTaskShouldExit", "Called.");
    }

    void Import_XamTaskCloseHandle()
    {
        Log::Stub("XamTaskCloseHandle", "Called.");
    }

    void Import_XamContentFlush()
    {
        Log::Stub("XamContentFlush", "Called.");
    }

    uint32_t Import_XamContentGetCreator(uint32_t userIndex, const XCONTENT_DATA *contentData, be<uint32_t> *isCreator, be<uint64_t> *xuid, XXOVERLAPPED *overlapped)
    {
        if (isCreator)
            *isCreator = true;

        if (xuid)
            *xuid = gXUID;

        return 0;
    }

    uint32_t Import_XamContentGetDeviceState()
    {
        return 0;
    }

    uint32_t Import_XamUserGetSigninInfo(uint32_t userIndex, uint32_t flags, XUSER_SIGNIN_INFO *info)
    {
        if (userIndex == 0)
        {
            memset(info, 0, sizeof(*info));
            info->xuid = gXUID;
            info->SigninState = 1;
            strcpy(info->Name, "SK8");
            return 0;
        }

        return 0x00000525; // ERROR_NO_SUCH_USER
    }

    void Import_XamContentSetThumbnail()
    {
        Log::Stub("XamContentSetThumbnail", "Called.");
    }

    void Import_XamLoaderGetLaunchData()
    {
        Log::Stub("XamLoaderGetLaunchData", "Called.");
    }

    void Import_XamLoaderGetLaunchDataSize()
    {
        Log::Stub("XamLoaderGetLaunchDataSize", "Called.");
    }

    void Import_XamLoaderSetLaunchData()
    {
        Log::Stub("XamLoaderSetLaunchData", "Called.");
    }

    void Import_XamTaskSchedule()
    {
        Log::Stub("XamTaskSchedule", "Called.");
    }

    void Import_XamUserAreUsersFriends()
    {
        Log::Stub("XamUserAreUsersFriends", "Called.");
    }

    void Import_XamUserCreateAchievementEnumerator()
    {
        Log::Stub("XamUserCreateAchievementEnumerator", "Called.");
    }

    void Import_XamContentDelete()
    {
        Log::Stub("XamContentDelete", "Called.");
    }

    void Import_XamCreateEnumeratorHandle()
    {
        Log::Stub("XamCreateEnumeratorHandle", "Called.");
    }

    void Import_XamGetPrivateEnumStructureFromHandle()
    {
        Log::Stub("XamGetPrivateEnumStructureFromHandle", "Called.");
    }

    void Import_XamUserGetMembershipTierFromXUID()
    {
        Log::Stub("XamUserGetMembershipTierFromXUID", "Called.");
    }

    void Import_XamUserGetOnlineCountryFromXUID()
    {
        Log::Stub("XamUserGetOnlineCountryFromXUID", "Called.");
    }

    void Import_XamVoiceClose()
    {
        Log::Stub("XamVoiceClose", "Called.");
    }

    void Import_XamVoiceCreate()
    {
        Log::Stub("XamVoiceCreate", "Called.");
    }

    void Import_XamVoiceHeadsetPresent()
    {
        Log::Stub("XamVoiceHeadsetPresent", "Called.");
    }

    void Import_XamVoiceSubmitPacket()
    {
        Log::Stub("XamVoiceSubmitPacket", "Called.");
    }
}

GUEST_FUNCTION_HOOK(__imp__XamVoiceClose, Hooks::Import_XamVoiceClose)
GUEST_FUNCTION_HOOK(__imp__XamVoiceCreate, Hooks::Import_XamVoiceCreate)
GUEST_FUNCTION_HOOK(__imp__XamVoiceHeadsetPresent, Hooks::Import_XamVoiceHeadsetPresent)
GUEST_FUNCTION_HOOK(__imp__XamVoiceSubmitPacket, Hooks::Import_XamVoiceSubmitPacket)
GUEST_FUNCTION_HOOK(__imp__XamCreateEnumeratorHandle, Hooks::Import_XamCreateEnumeratorHandle)
GUEST_FUNCTION_HOOK(__imp__XamGetPrivateEnumStructureFromHandle, Hooks::Import_XamGetPrivateEnumStructureFromHandle)
GUEST_FUNCTION_HOOK(__imp__XamUserGetMembershipTierFromXUID, Hooks::Import_XamUserGetMembershipTierFromXUID)
GUEST_FUNCTION_HOOK(__imp__XamUserGetOnlineCountryFromXUID, Hooks::Import_XamUserGetOnlineCountryFromXUID)
GUEST_FUNCTION_HOOK(__imp__XamContentDelete, Hooks::Import_XamContentDelete)

GUEST_FUNCTION_HOOK(__imp__XNotifyPositionUI, Hooks::Import_XNotifyPositionUI)
GUEST_FUNCTION_HOOK(__imp__XNotifyGetNext, Hooks::Import_XNotifyGetNext)
GUEST_FUNCTION_HOOK(__imp__XMsgStartIORequest, Hooks::Import_XMsgStartIORequest)
GUEST_FUNCTION_HOOK(__imp__XGetGameRegion, Hooks::Import_XGetGameRegion)
GUEST_FUNCTION_HOOK(__imp__XMsgCancelIORequest, Hooks::Import_XMsgCancelIORequest)
GUEST_FUNCTION_HOOK(__imp__XGetLanguage, Hooks::Import_XGetLanguage)
GUEST_FUNCTION_HOOK(__imp__XGetAVPack, Hooks::Import_XGetAVPack)

GUEST_FUNCTION_HOOK(__imp__XamContentFlush, Hooks::Import_XamContentFlush)
GUEST_FUNCTION_HOOK(__imp__XamContentGetDeviceState, Hooks::Import_XamContentGetDeviceState)
GUEST_FUNCTION_HOOK(__imp__XamContentSetThumbnail, Hooks::Import_XamContentSetThumbnail)
GUEST_FUNCTION_HOOK(__imp__XamLoaderGetLaunchData, Hooks::Import_XamLoaderGetLaunchData)
GUEST_FUNCTION_HOOK(__imp__XamLoaderGetLaunchDataSize, Hooks::Import_XamLoaderGetLaunchDataSize)
GUEST_FUNCTION_HOOK(__imp__XamLoaderSetLaunchData, Hooks::Import_XamLoaderSetLaunchData)
GUEST_FUNCTION_HOOK(__imp__XamTaskSchedule, Hooks::Import_XamTaskSchedule)
GUEST_FUNCTION_HOOK(__imp__XamUserAreUsersFriends, Hooks::Import_XamUserAreUsersFriends)
GUEST_FUNCTION_HOOK(__imp__XamUserCreateAchievementEnumerator, Hooks::Import_XamUserCreateAchievementEnumerator)
GUEST_FUNCTION_HOOK(__imp__XamContentGetCreator, Hooks::Import_XamContentGetCreator)
GUEST_FUNCTION_HOOK(__imp__XamTaskCloseHandle, Hooks::Import_XamTaskCloseHandle)
GUEST_FUNCTION_HOOK(__imp__XamTaskShouldExit, Hooks::Import_XamTaskShouldExit)
GUEST_FUNCTION_HOOK(__imp__XamUserGetSigninInfo, Hooks::Import_XamUserGetSigninInfo)
GUEST_FUNCTION_HOOK(__imp__XamShowFriendRequestUI, Hooks::Import_XamShowFriendRequestUI)
GUEST_FUNCTION_HOOK(__imp__XamShowGameInviteUI, Hooks::Import_XamShowGameInviteUI)
GUEST_FUNCTION_HOOK(__imp__XamShowMessageComposeUI, Hooks::Import_XamShowMessageComposeUI)
GUEST_FUNCTION_HOOK(__imp__XamShowPlayerReviewUI, Hooks::Import_XamShowPlayerReviewUI)
GUEST_FUNCTION_HOOK(__imp__XamShowKeyboardUI, Hooks::Import_XamShowKeyboardUI)
GUEST_FUNCTION_HOOK(__imp__XamContentCreateEx, Hooks::Import_XamContentCreateEx)
GUEST_FUNCTION_HOOK(__imp__XamContentClose, Hooks::Import_XamContentClose)
GUEST_FUNCTION_HOOK(__imp__XamContentGetLicenseMask, Hooks::Import_XamContentGetLicenseMask)
GUEST_FUNCTION_HOOK(__imp__XamContentCreateEnumerator, Hooks::Import_XamContentCreateEnumerator)
GUEST_FUNCTION_HOOK(__imp__XamContentGetDeviceData, Hooks::Import_XamContentGetDeviceData)
GUEST_FUNCTION_HOOK(__imp__XamEnumerate, Hooks::Import_XamEnumerate)
GUEST_FUNCTION_HOOK(__imp__XamShowSigninUI, Hooks::Import_XamShowSigninUI)
GUEST_FUNCTION_HOOK(__imp__XamShowGamerCardUIForXUID, Hooks::Import_XamShowGamerCardUIForXUID)
GUEST_FUNCTION_HOOK(__imp__XamShowAchievementsUI, Hooks::Import_XamShowAchievementsUI)
GUEST_FUNCTION_HOOK(__imp__XamShowMarketplaceUI, Hooks::Import_XamShowMarketplaceUI)
GUEST_FUNCTION_HOOK(__imp__XamShowDeviceSelectorUI, Hooks::Import_XamShowDeviceSelectorUI)
GUEST_FUNCTION_HOOK(__imp__XamShowDirtyDiscErrorUI, Hooks::Import_XamShowDirtyDiscErrorUI)
GUEST_FUNCTION_HOOK(__imp__XamUserGetName, Hooks::Import_XamUserGetName)
GUEST_FUNCTION_HOOK(__imp__XamUserGetSigninState, Hooks::Import_XamUserGetSigninState)
GUEST_FUNCTION_HOOK(__imp__XamUserCheckPrivilege, Hooks::Import_XamUserCheckPrivilege)
GUEST_FUNCTION_HOOK(__imp__XamGetSystemVersion, Hooks::Import_XamGetSystemVersion)
GUEST_FUNCTION_HOOK(__imp__XamUserCreateStatsEnumerator, Hooks::Import_XamUserCreateStatsEnumerator)
GUEST_FUNCTION_HOOK(__imp__XamUserGetXUID, Hooks::Import_XamUserGetXUID)
GUEST_FUNCTION_HOOK(__imp__XamEnableInactivityProcessing, Hooks::Import_XamEnableInactivityProcessing)
GUEST_FUNCTION_HOOK(__imp__XamResetInactivity, Hooks::Import_XamResetInactivity)
GUEST_FUNCTION_HOOK(__imp__XamLoaderLaunchTitle, Hooks::Import_XamLoaderLaunchTitle)
GUEST_FUNCTION_HOOK(__imp__XamNotifyCreateListener, Hooks::Import_XamNotifyCreateListener)
GUEST_FUNCTION_HOOK(__imp__XamShowMessageBoxUIEx, Hooks::Import_XamShowMessageBoxUIEx)
GUEST_FUNCTION_HOOK(__imp__XamLoaderTerminateTitle, Hooks::Import_XamLoaderTerminateTitle)
GUEST_FUNCTION_HOOK(__imp__XamGetExecutionId, Hooks::Import_XamGetExecutionId)
GUEST_FUNCTION_HOOK(__imp__XamUserReadProfileSettings, Hooks::Import_XamUserReadProfileSettings)
GUEST_FUNCTION_HOOK(__imp__XamUserWriteProfileSettings, Hooks::Import_XamUserWriteProfileSettings)
GUEST_FUNCTION_HOOK(__imp__XamFree, Hooks::Import_XamFree)
GUEST_FUNCTION_HOOK(__imp__XamAlloc, Hooks::Import_XamAlloc)
GUEST_FUNCTION_HOOK(__imp__XamSessionRefObjByHandle, Hooks::Import_XamSessionRefObjByHandle)
GUEST_FUNCTION_HOOK(__imp__XamSessionCreateHandle, Hooks::Import_XamSessionCreateHandle)
