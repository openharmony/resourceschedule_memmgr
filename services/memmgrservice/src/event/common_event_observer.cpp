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

#include "common_event_observer.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"

#include "common_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "CommonEventObserver";
}

CommonEventObserver::CommonEventObserver(const EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : EventFwk::CommonEventSubscriber(subscriberInfo)
{
}

CommonEventObserver::~CommonEventObserver()
{
}

void CommonEventObserver::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    auto want = eventData.GetWant();
    std::string action = want.GetAction();
    HILOGI("action=<%{public}s>", action.c_str());
}
} // namespace Memory
} // namespace OHOS
