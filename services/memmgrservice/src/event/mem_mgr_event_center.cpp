/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mem_mgr_event_center.h"
#include <string>
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "common_event_observer.h"
#include "reclaim_priority_manager.h"
#ifdef CONFIG_BGTASK_MGR
#include "background_task_mgr_helper.h"
#endif
#include "connection_observer_client.h"
#include "common_event_support.h"
#include "common_event_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrEventCenter";
const std::string MEMMGR_CENTER_HANDLER = "MemMgrEventCenterHandler";
const int ACCOUNT_MAX_RETRY_TIMES = 10;
const int ACCOUNT_RETRY_DELAY = 3000;
const int EXTCONN_RETRY_TIME = 1000;
}

IMPLEMENT_SINGLE_INSTANCE(MemMgrEventCenter);

MemMgrEventCenter::MemMgrEventCenter()
{}

bool MemMgrEventCenter::Init()
{
    HILOGI("called");
    if (CreateRegisterHandler()) {
        HandlerRegisterEvent(RegisterEvent::REG_ALLOBS_EVENT);
        return true;
    }
    return false;
}

bool MemMgrEventCenter::CreateRegisterHandler()
{
    if (!regObsHandler_) {
        MAKE_POINTER(regObsHandler_, shared, AppExecFwk::EventHandler, "failed to create register handler",
        return false, AppExecFwk::EventRunner::Create(MEMMGR_CENTER_HANDLER));
    }
    return true;
}

void MemMgrEventCenter::RemoveEventObserver(int32_t systemAbilityId)
{
    HILOGI("called");

    if (systemAbilityId == ABILITY_MGR_SERVICE_ID || systemAbilityId == APP_MGR_SERVICE_ID) {
        appStateObserver_ = nullptr;
        extConnObserver_ = nullptr;
        ReclaimPriorityManager::GetInstance().Reset();
    }
    if (systemAbilityId == SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN) {
        accountObserver_ = nullptr;
    }

    if (systemAbilityId == COMMON_EVENT_SERVICE_ID || systemAbilityId == COMMON_EVENT_SERVICE_ABILITY_ID) {
        commonEventObserver_ = nullptr;
    }    
}

bool MemMgrEventCenter::RegisterEventObserver()
{
    HILOGI("called");
    if (!memoryPressureObserver_) {
        RegisterMemoryPressureObserver();
    }

    if (!appStateObserver_) {
        RegisterAppStateObserver();
    }

    if (!extConnObserver_) {
        RegisterExtConnObserver();
    }

    if (!accountObserver_) {
        RegisterAccountObserver();
    }

    if (!commonEventObserver_) {
        RegisterCommonEventObserver();
    }

#ifdef CONFIG_BGTASK_MGR
    if (!bgTaskObserver_) {
        RegisterBgTaskObserver();
    }
#endif
    return true;
}

void MemMgrEventCenter::HandlerRegisterEvent(int64_t registerEventId)
{
    switch (registerEventId)
    {
        case RegisterEvent::REG_ALLOBS_EVENT:
            {
                std::function<void()> RegisterEventObserverFunc =
                                std::bind(&MemMgrEventCenter::RegisterEventObserver, this);
                regObsHandler_->PostImmediateTask(RegisterEventObserverFunc);
            }
            break;
        case RegisterEvent::REG_MEMPRESSOBS_EVENT:
            {
                std::function<void()> RegisterMemoryPressureObserverFunc =
                                    std::bind(&MemMgrEventCenter::RegisterMemoryPressureObserver, this);
                regObsHandler_->PostImmediateTask(RegisterMemoryPressureObserverFunc);
            }
            break;
        case RegisterEvent::REG_APPOBS_EVENT:
            {
                std::function<void()> RegisterAppStateObserverFunc =
                                    std::bind(&MemMgrEventCenter::RegisterAppStateObserver, this);
                regObsHandler_->PostImmediateTask(RegisterAppStateObserverFunc);
            }
            break;
        case RegisterEvent::REG_EXTOBS_EVENT:
            {
                std::function<void()> RegisterExtConnObserverFunc =
                                    std::bind(&MemMgrEventCenter::RegisterExtConnObserver, this);
                regObsHandler_->PostImmediateTask(RegisterExtConnObserverFunc);
            }
            break;
        case RegisterEvent::REG_ACCOUNTOBS_EVENT:
            {
                std::function<void()> RegisterAccountObserverFunc =
                                    std::bind(&MemMgrEventCenter::RegisterAccountObserver, this);
                regObsHandler_->PostImmediateTask(RegisterAccountObserverFunc);
            }
            break;
        case RegisterEvent::REG_COMMONOBS_EVENT:
            {
                std::function<void()> RegisterCommonEventObserverFunc =
                                    std::bind(&MemMgrEventCenter::RegisterCommonEventObserver, this);
                regObsHandler_->PostImmediateTask(RegisterCommonEventObserverFunc);
            }
            break;
        case RegisterEvent::REG_BGTASKOBS_EVENT:
#ifdef CONFIG_BGTASK_MGR
            {
                std::function<void()> RegisterBgTaskObserverFunc =
                                    std::bind(&MemMgrEventCenter::RegisterBgTaskObserver, this);
                regObsHandler_->PostImmediateTask(RegisterBgTaskObserverFunc);
            }
            break;
#endif
    }
    return;
}

void MemMgrEventCenter::RegisterAppStateObserver()
{
    HILOGI("called");
    MAKE_POINTER(appMgrClient_, unique, AppExecFwk::AppMgrClient, "make appMgrClient failed",
         return, /* no param */);
    appStateObserver_ = new (std::nothrow) AppStateObserver();
    while (appMgrClient_->ConnectAppMgrService() != AppExecFwk::AppMgrResultCode::RESULT_OK) {
        HILOGE("ConnectAppMgrService fail, try again! retryTimes=%{public}d", ++regAppStatusObsRetry_);
    }
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<AppExecFwk::IAppMgr> appObject =
        iface_cast<AppExecFwk::IAppMgr>(systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID));
    if (appObject) {
        int ret = appObject->RegisterApplicationStateObserver(appStateObserver_);
        if (ret == ERR_OK) {
            HILOGI("register success");
            return;
        }
        HILOGE("register fail, ret = %{public}d", ret);
        return;
    }
    HILOGE("get SystemAbilityManager fail");
}

void MemMgrEventCenter::RegisterExtConnObserver()
{
    HILOGI("called");
    MAKE_POINTER(extConnObserver_, shared, ExtensionConnectionObserver, "make ExtensionConnectionObserver failed",
        /* no return */, /* no param */);
    if (extConnObserver_ != nullptr) {
        int32_t ret = AbilityRuntime::ConnectionObserverClient::GetInstance().RegisterObserver(extConnObserver_);
        if (ret == ERR_OK) {
            HILOGI("register success");
            return;
        }
        HILOGE("register fail, ret = %{public}d", ret);
    }
    std::function<void()> RegisterExtConnObserverFunc = std::bind(&MemMgrEventCenter::RegisterExtConnObserver, this);
    regObsHandler_->PostTask(RegisterExtConnObserverFunc, EXTCONN_RETRY_TIME, AppExecFwk::EventQueue::Priority::LOW);
}

void MemMgrEventCenter::RegisterBgTaskObserver()
{
    HILOGI("called");
#ifdef CONFIG_BGTASK_MGR
    if (bgTaskObserver_) {
        return;
    }
    MAKE_POINTER(bgTaskObserver_, shared, BgTaskObserver, "make BgTaskObserver failed",
            return, /* no param */);
    ErrCode ret = BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(*bgTaskObserver_);
    if (ret == ERR_OK) {
        HILOGI("register success");
        return;
    }
    HILOGE("register fail, ret = %{public}d", ret);
#else
    HILOGI("BackgroundTaskMgr is not enable.");
#endif
}

void MemMgrEventCenter::RegisterCommonEventObserver()
{
    HILOGI("called");
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventSubscribeInfo commonEventSubscribeInfo(matchingSkills);
    MAKE_POINTER(commonEventObserver_, shared, CommonEventObserver, "make unique failed",
        return, commonEventSubscribeInfo);
    if (EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventObserver_)) {
        HILOGI("register success");
        return;
    }
    HILOGI("register fail");
}

void MemMgrEventCenter::RegisterAccountObserver()
{
    HILOGI("called");
    regAccountObsRetry_++;
    AccountSA::OsAccountSubscribeInfo osAccountSubscribeInfo;
    osAccountSubscribeInfo.SetOsAccountSubscribeType(AccountSA::OS_ACCOUNT_SUBSCRIBE_TYPE::ACTIVED);
    osAccountSubscribeInfo.SetName("MemMgrAccountActivedSubscriber");
    MAKE_POINTER(accountObserver_, shared, AccountObserver, "make unique failed", return, osAccountSubscribeInfo);
    ErrCode errCode = AccountSA::OsAccountManager::SubscribeOsAccount(accountObserver_);
    if (errCode == ERR_OK) {
        HILOGI("register success");
        return;
    }

    if (regAccountObsRetry_ < ACCOUNT_MAX_RETRY_TIMES) {
        std::function<void()> RegisterAccountObserverFunc =
            std::bind(&MemMgrEventCenter::RegisterAccountObserver, this);
        HILOGE("register fail, retCode = %{public}d, try again after 3s!, retryTimes=%{public}d/10",
            errCode, regAccountObsRetry_);
        regObsHandler_->PostTask(RegisterAccountObserverFunc, ACCOUNT_RETRY_DELAY,
            AppExecFwk::EventQueue::Priority::LOW); // 3000 means 3s
    }
}

void MemMgrEventCenter::RegisterMemoryPressureObserver()
{
    HILOGI("called");
    MAKE_POINTER(memoryPressureObserver_, shared, MemoryPressureObserver, "make MemoryPressureObserver failed",
        /* no return */, /* no param */);
    std::function<void()> initFunc = std::bind(&MemoryPressureObserver::Init, memoryPressureObserver_);
    regObsHandler_->PostTask(initFunc, 10000, AppExecFwk::EventQueue::Priority::HIGH); // 10000 means 10s
}

MemMgrEventCenter::~MemMgrEventCenter()
{
    UnregisterEventObserver();
}

void MemMgrEventCenter::UnregisterEventObserver()
{
#ifdef CONFIG_BGTASK_MGR
    if (bgTaskObserver_) {
        BackgroundTaskMgr::BackgroundTaskMgrHelper::UnsubscribeBackgroundTask(*bgTaskObserver_);
    }
    bgTaskObserver_ = nullptr;
#endif
    if (accountObserver_) {
        AccountSA::OsAccountManager::UnsubscribeOsAccount(accountObserver_);
    }
    if (commonEventObserver_) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(commonEventObserver_);
    }
    if (appStateObserver_) {
        delete appStateObserver_;
        appStateObserver_ = nullptr;
    }
    appMgrClient_ = nullptr;
    regObsHandler_ = nullptr;
    extConnObserver_ = nullptr;
    accountObserver_ = nullptr;
}

void MemMgrEventCenter::Dump(int fd)
{
    dprintf(fd, "state list of all observer\n");
    dprintf(fd, "                 name               state \n");
    dprintf(fd, "%30s %8s\n", "MemoryPressureObserver", memoryPressureObserver_ == nullptr ? "N" : "Y");
    dprintf(fd, "-----------------------------------------------------------------\n");
    dprintf(fd, "%30s %8s\n", "AppStateObserver", appStateObserver_ == nullptr ? "N" : "Y");
    dprintf(fd, "-----------------------------------------------------------------\n");
    dprintf(fd, "%30s %8s\n", "ExtConnObserver", extConnObserver_ == nullptr ? "N" : "Y");
    dprintf(fd, "-----------------------------------------------------------------\n");
#ifdef CONFIG_BGTASK_MGR
    dprintf(fd, "%30s %8s\n", "BgTaskObserver", bgTaskObserver_ == nullptr ? "N" : "Y");
    dprintf(fd, "-----------------------------------------------------------------\n");
#endif
    dprintf(fd, "%30s %8s\n", "AccountObserver", accountObserver_ == nullptr ? "N" : "Y");
    dprintf(fd, "-----------------------------------------------------------------\n");
    dprintf(fd, "%30s %8s\n", "CommonEventObserver", commonEventObserver_ == nullptr ? "N" : "Y");
    dprintf(fd, "-----------------------------------------------------------------\n");
}

void MemMgrEventCenter::RetryRegisterEventObserver(int32_t systemAbilityId)
{
#ifdef CONFIG_BGTASK_MGR
    if (systemAbilityId == BACKGROUND_TASK_MANAGER_SERVICE_ID) {
        HandlerRegisterEvent(RegisterEvent::REG_BGTASKOBS_EVENT);
    }
#endif
    if (systemAbilityId == ABILITY_MGR_SERVICE_ID || systemAbilityId == APP_MGR_SERVICE_ID) {
        HandlerRegisterEvent(RegisterEvent::REG_APPOBS_EVENT);
    }

    if (systemAbilityId == SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN) {
        HandlerRegisterEvent(RegisterEvent::REG_ACCOUNTOBS_EVENT);
    }

    if (systemAbilityId == COMMON_EVENT_SERVICE_ID || systemAbilityId == COMMON_EVENT_SERVICE_ABILITY_ID) {
        HandlerRegisterEvent(RegisterEvent::REG_COMMONOBS_EVENT);
    }
}
} // namespace Memory
} // namespace OHOS
