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

#include "mem_mgr_event_observer.h"
#include "memmgr_log.h"

#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrEventObserver";
}

MemMgrEventObserver::MemMgrEventObserver(const MemMgrCaredEventCallback &callback) : callback_(callback)
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_CONNECTED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_POWER_DISCONNECTED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    EventFwk::CommonEventSubscribeInfo commonEventSubscribeInfo(matchingSkills);

    subscriber_ = std::make_shared<MemMgrEventSubscriber>(commonEventSubscribeInfo,
                     std::bind(&MemMgrEventObserver::OnReceiveEvent, this, std::placeholders::_1));
    EventFwk::CommonEventManager::SubscribeCommonEvent(subscriber_);
}

MemMgrEventObserver::~MemMgrEventObserver()
{
    if (subscriber_) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(subscriber_);
    }
}

void MemMgrEventObserver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    HILOGI("called");
    if (callback_.OnReceiveCaredEvent != nullptr) {
        callback_.OnReceiveCaredEvent(eventData);
    }
}
} // namespace Memory
} // namespace OHOS
