/*
 * Copyright (c) 2021 XXXX.
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

#ifndef OHOS_MEMORY_MEMMGR_EVENT_OBSERVER_H
#define OHOS_MEMORY_MEMMGR_EVENT_OBSERVER_H

#include "common_event_subscriber.h"
#include "common_event_subscribe_info.h"
#include "mem_mgr_event_subscriber.h"

namespace OHOS {
namespace Memory {
struct MemMgrCaredEventCallback {
    std::function<void(const EventFwk::CommonEventData &data)> OnReceiveCaredEvent;
};

class MemMgrEventObserver {
public:
    MemMgrEventObserver(const MemMgrCaredEventCallback &callback);
    ~MemMgrEventObserver();
    /**
     * @brief System common event recei ver.
     * @param eventData Common event data.
     */
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData);
protected:
private:
    std::shared_ptr<MemMgrEventSubscriber> subscriber_;
    MemMgrCaredEventCallback callback_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_EVENT_OBSERVER_H
