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

#include <string>

#include "mem_mgr_event_center.h"
#include "memmgr_log.h"

#include "mem_mgr_event_observer.h"
#include "reclaim_priority_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrEventCenter";
}

IMPLEMENT_SINGLE_INSTANCE(MemMgrEventCenter);

MemMgrEventCenter::MemMgrEventCenter() : appStateCallback_(std::make_shared<AppStateCallbackMemHost>())
{
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
        handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
        if (handler_ == nullptr) {
            HILOGI("failed to create event handler");
            return false;
        }
    }
    return true;
}

bool MemMgrEventCenter::RegisterEventListener()
{
    HILOGI("called");
    RegisterMemPressMonitor();

    RegisterAppStateCallback();

    RegisterAccountObserver();

    RegisterSystemEventObserver();
    return true;
}

void MemMgrEventCenter::RegisterAppStateCallback()
{
    HILOGI("called");
    retryTimes_++;
    if (!appStateCallback_->ConnectAppMgrService() && retryTimes_ < 10) { // 10 : max retry times
        HILOGE("failed to ConnectAppMgrService, try again after 3s!, retryTimes=%{public}d/10", retryTimes_);
        handler_->PostTask(registerEventListenerFunc_, 3000, AppExecFwk::EventQueue::Priority::HIGH); // 3000 means 3s
        return;
    }
    if (appStateCallback_->Connected()) {
        HILOGE("success to ConnectAppMgrService");
        if (!appStateCallback_->Register()) {
            HILOGE("failed to RegisterAppStateCallback");
            return;
        }
        HILOGI("success to RegisterAppStateCallback");
    }
}

void MemMgrEventCenter::RegisterSystemEventObserver()
{
    MemMgrCaredEventCallback callback = {
        std::bind(&MemMgrEventCenter::OnReceiveCaredEvent, this, std::placeholders::_1),
    };
    sysEvtOberserver_ = std::make_unique<MemMgrEventObserver>(callback);
    
    HILOGI("success to register cared event callback");
}

void MemMgrEventCenter::RegisterAccountObserver()
{
    AccountCallback callback = {
        std::bind(&MemMgrEventCenter::OnOsAccountsChanged, this, std::placeholders::_1),
    };
    accountOberserver_ = std::make_unique<AccountObserver>(callback);
    HILOGI("success to register account callback");
}

void MemMgrEventCenter::RegisterMemPressMonitor()
{
    HILOGI("called");
    MemPressCallback callback = {
        std::bind(&MemMgrEventCenter::OnMemPressLevelUploaded, this, std::placeholders::_1),
    };
    psiMonitor_ = std::make_unique<MemoryPressureMonitor>(callback);
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
    ReclaimPriorityManager::GetInstance().OsAccountChanged(id);
}

void MemMgrEventCenter::OnMemPressLevelUploaded(const int &level)
{
    HILOGI("memory pressure level=<%{public}d>", level);
    // notify kill strategy manager
}
} // namespace Memory
} // namespace OHOS
