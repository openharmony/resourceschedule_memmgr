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

#ifndef OHOS_MEMORY_MEMMGR_EVENT_MEM_HOST_H
#define OHOS_MEMORY_MEMMGR_EVENT_MEM_HOST_H

#include "app_process_data.h"
#include "app_mgr_client.h"
#include "app_state_observer.h"

#include <string>
#include "single_instance.h"
#include "memmgr_log.h"

namespace OHOS {
namespace Memory {
class AppStateCallbackMemHost {
public:
    AppStateCallbackMemHost();
    ~AppStateCallbackMemHost();
    bool ConnectAppMgrService();
    bool Connected();
    bool Register();
private:
    bool connected_ = false;
    std::unique_ptr<AppExecFwk::AppMgrClient> appMgrClient_;
    sptr<AppStateObserver> appStateObserver_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_EVENT_MEM_HOST_H
