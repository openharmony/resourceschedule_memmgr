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

#ifndef OHOS_MEMORY_MEMMGR_EVENT_CENTER_H
#define OHOS_MEMORY_MEMMGR_EVENT_CENTER_H

#include "single_instance.h"
#include "event_handler.h"
#include "app_state_callback_mem_host.h"
#include "mem_mgr_event_observer.h"
#include "account_observer.h"

namespace OHOS {
namespace Memory {
class MemMgrEventCenter {
    DECLARE_SINGLE_INSTANCE_BASE(MemMgrEventCenter);

public:
    bool Init();
    ~MemMgrEventCenter();

protected:
    /**
     * @brief System common event receiver.
     * @param eventData Common event data.
     */
    void OnReceiveCaredEvent(const EventFwk::CommonEventData &eventData);

    void OnOsAccountsChanged(const int &id);

    void OnMemPsiUploaded(const int &level);
private:
    MemMgrEventCenter();
    bool GetEventHandler();
    bool RegisterEventListener();
    void RegisterAppStateCallback();
    void RegisterSystemEventObserver();
    void RegisterAccountObserver();
    void RegisterMemPsiMonitor();

    std::function<void()> registerEventListenerFunc_;
    int retryTimes_ = 0;
    std::shared_ptr<AppStateCallbackMemHost> appStateCallback_;
    std::unique_ptr<MemMgrEventObserver> sysEvtOberserver_;
    std::unique_ptr<AccountObserver> accountOberserver_;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_EVENT_CENTER_H
