/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "mem_mgr_event_observer.h"
#include "reclaim_priority_manager.h"
#include "background_task_mgr_helper.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrEventCenter";
}

IMPLEMENT_SINGLE_INSTANCE(MemMgrEventCenter);

MemMgrEventCenter::MemMgrEventCenter()
{
    MAKE_POINTER(appStateCallback_, shared, AppStateCallbackMemHost, "make AppStateCallbackMemHost failed",
        /* no return */, /* no param */);
    MAKE_POINTER(subscriber_, shared, MemMgrBgTaskSubscriber, "make MemMgrBgTaskSubscriber failed", /* no return */,
        /* no param */);
    registerEventListenerFunc_ = std::bind(&MemMgrEventCenter::RegisterAppStateCallback, this);
}

MemMgrEventCenter::~MemMgrEventCenter()
{
}

bool MemMgrEventCenter::Init()
{
    HILOGI("called");
    if (GetEventHandler()) {
        return RegisterEventListener();
    }
    return false;
}

bool MemMgrEventCenter::GetEventHandler()
{
    if (!handler_) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

bool MemMgrEventCenter::RegisterEventListener()
{
    HILOGI("called");
    RegisterMemPressMonitor();

    RegisterAppStateCallback();

    RegisterBgTaskObserver();

    RegisterAccountObserver();

    RegisterSystemEventObserver();
    return true;
}

void MemMgrEventCenter::RegisterAppStateCallback()
{
    HILOGI("called");
    while (!appStateCallback_->ConnectAppMgrService()) {
        HILOGE("failed to ConnectAppMgrService, try again! retryTimes=%{public}d", ++retryTimes_);
    }
    if (appStateCallback_->Connected()) {
        HILOGE("success to ConnectAppMgrService, retryTimes=%{public}d", retryTimes_);
        if (!appStateCallback_->Register()) {
            HILOGE("failed to RegisterAppStateCallback");
            return;
        }
        HILOGI("success to RegisterAppStateCallback");
    }
}

void MemMgrEventCenter::RegisterBgTaskObserver()
{
    HILOGI("called");
    ErrCode ret = BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(*subscriber_);
    HILOGI("ret = %{public}d", ret);
}

void MemMgrEventCenter::RegisterSystemEventObserver()
{
    MemMgrCaredEventCallback callback = {
        std::bind(&MemMgrEventCenter::OnReceiveCaredEvent, this, std::placeholders::_1),
    };
    MAKE_POINTER(sysEvtOberserver_, unique, MemMgrEventObserver, "make unique failed", return, callback);
    HILOGI("success to register cared event callback");
}

void MemMgrEventCenter::RegisterAccountObserver()
{
    AccountCallback callback = {
        std::bind(&MemMgrEventCenter::OnOsAccountsChanged, this, std::placeholders::_1),
    };
    MAKE_POINTER(accountOberserver_, unique, AccountObserver, "make unique failed", return, callback);
    HILOGI("success to register account callback");
}

void MemMgrEventCenter::RegisterMemPressMonitor()
{
    HILOGI("called");
    MemPressCallback callback = {
        std::bind(&MemMgrEventCenter::OnMemPressLevelUploaded, this, std::placeholders::_1),
    };
    MAKE_POINTER(psiMonitor_, unique, MemoryPressureMonitor, "make unique failed", return, callback);
    HILOGI("success to register memory pressure callback");
}

// callback list below

void MemMgrEventCenter::OnReceiveCaredEvent(const EventFwk::CommonEventData &eventData)
{
    auto want = eventData.GetWant();
    std::string action = want.GetAction();
    HILOGI("action=<%{public}s>", action.c_str());
}

void MemMgrEventCenter::OnOsAccountsChanged(const int &id)
{
    HILOGI("account changed=<%{public}d>", id);
    // notify reclaim priority manager
    AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod = AccountSA::OsAccountManager::GetOsAccountSwitchMod();
    ReclaimPriorityManager::GetInstance().OsAccountChanged(id, switchMod);
}

void MemMgrEventCenter::OnMemPressLevelUploaded(const int &level)
{
    HILOGI("memory pressure level=<%{public}d>", level);
    // notify kill strategy manager
}
} // namespace Memory
} // namespace OHOS
