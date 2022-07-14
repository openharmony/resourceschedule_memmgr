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

#include "mem_mgr_client.h"
#include "memmgr_log.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrClient";
}

IMPLEMENT_SINGLE_INSTANCE(MemMgrClient);

int32_t MemMgrClient::GetBundlePriorityList(BundlePriorityList &bundlePrioList)
{
    HILOGE("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->GetBundlePriorityList(bundlePrioList);
}

int32_t MemMgrClient::NotifyDistDevStatus(int32_t pid, int32_t uid, const std::string &name, bool connected)
{
    HILOGI("called, pid=%{public}d, uid=%{public}d, name=%{public}s, connected=%{public}d", pid, uid, name.c_str(),
        connected);
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->NotifyDistDevStatus(pid, uid, name, connected);
}


sptr<IMemMgr> MemMgrClient::GetMemMgrService()
{
    HILOGI("called");
    std::lock_guard<std::mutex> lock(mutex_);
    if (dpProxy_ != nullptr) {
        return dpProxy_;
    }

    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        HILOGE("get samgr failed");
        return nullptr;
    }
    auto object = samgrProxy->GetSystemAbility(MEMORY_MANAGER_SA_ID);
    if (object == nullptr) {
        HILOGE("get service failed");
        return nullptr;
    }
    HILOGI("get service succeed");
    dpProxy_ = iface_cast<IMemMgr>(object);
    return dpProxy_;
}
} // namespace Memory
} // namespace OHOS
