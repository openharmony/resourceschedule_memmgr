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
#include "extension_connection_observer.h"
#include "common_event_observer.h"
#include "account_observer.h"
#include "memory_pressure_observer.h"
#ifdef CONFIG_BGTASK_MGR
#include "bg_task_observer.h"
#endif
#include "system_ability_definition.h"
#include "app_mgr_interface.h"
#include "iservice_registry.h"
#include "app_process_data.h"
#include "app_mgr_client.h"
#include "app_state_observer.h"

namespace OHOS {
namespace Memory {
class MemMgrEventCenter {
    DECLARE_SINGLE_INSTANCE_BASE(MemMgrEventCenter);

public:
    ~MemMgrEventCenter();
    bool Init();
    void RetryRegisterEventObserver(int32_t systemAbilityId);
    void Dump(int fd);
    void RemoveEventObserver(int32_t systemAbilityId);
private:
    MemMgrEventCenter();
    bool CreateRegisterHandler();
    void UnregisterEventObserver();
    bool RegisterEventObserver();
    void RegisterBgTaskObserver();
    void RegisterAccountObserver();
    void RegisterExtConnObserver();
    void RegisterAppStateObserver();
    void RegisterCommonEventObserver();
    void RegisterMemoryPressureObserver();
    int regAccountObsRetry_ = 0;
    int regAppStatusObsRetry_ = 0;
    std::unique_ptr<AppExecFwk::AppMgrClient> appMgrClient_;
    std::shared_ptr<AppExecFwk::EventHandler> regObsHandler_;
    std::shared_ptr<ExtensionConnectionObserver> extConnObserver_;
    std::shared_ptr<AccountObserver> accountObserver_;
    std::shared_ptr<CommonEventObserver> commonEventObserver_;
#ifdef CONFIG_BGTASK_MGR
    std::shared_ptr<BgTaskObserver> bgTaskObserver_;
#endif
    std::shared_ptr<MemoryPressureObserver> memoryPressureObserver_;
    sptr<AppStateObserver> appStateObserver_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_EVENT_CENTER_H
