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

extern "C" {
    int32_t notify_process_status(int32_t pid, int32_t type, int32_t status, int saId)
    {
        return OHOS::Memory::MemMgrClient::GetInstance().NotifyProcessStatus(pid, type, status, saId);
    }

    int32_t set_critical(int32_t pid, bool critical, int32_t saId)
    {
        return OHOS::Memory::MemMgrClient::GetInstance().SetCritical(pid, critical, saId);
    }
}

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

int32_t MemMgrClient::GetKillLevelOfLmkd(int32_t &killLevel)
{
    HILOGE("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->GetKillLevelOfLmkd(killLevel);
}

#ifdef USE_PURGEABLE_MEMORY
int32_t MemMgrClient::RegisterActiveApps(int32_t pid, int32_t uid)
{
    HILOGI("called, pid=%{public}d, uid=%{public}d", pid, uid);
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->RegisterActiveApps(pid, uid);
}

int32_t MemMgrClient::DeregisterActiveApps(int32_t pid, int32_t uid)
{
    HILOGI("called, pid=%{public}d, uid=%{public}d", pid, uid);
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->DeregisterActiveApps(pid, uid);
}

int32_t MemMgrClient::SubscribeAppState(const AppStateSubscriber &subscriber)
{
    HILOGI("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    sptr<AppStateSubscriber::AppStateSubscriberImpl> subscriberSptr = subscriber.GetImpl();
    if (subscriberSptr == nullptr) {
        HILOGE("subscriberSptr is null");
        return -1;
    }
    return dps->SubscribeAppState(subscriberSptr);
}

int32_t MemMgrClient::UnsubscribeAppState(const AppStateSubscriber &subscriber)
{
    HILOGI("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    sptr<AppStateSubscriber::AppStateSubscriberImpl> subscriberSptr = subscriber.GetImpl();
    if (subscriberSptr == nullptr) {
        HILOGE("subscriberSptr is null");
        return -1;
    }
    return dps->UnsubscribeAppState(subscriberSptr);
}

int32_t MemMgrClient::GetAvailableMemory(int32_t &memSize)
{
    HILOGI("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->GetAvailableMemory(memSize);
}

int32_t MemMgrClient::GetTotalMemory(int32_t &memSize)
{
    HILOGI("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->GetTotalMemory(memSize);
}

int32_t MemMgrClient::GetAvailableMemory()
{
    int32_t memSize = 0;
    if (GetAvailableMemory(memSize) != 0) {
        return -1;
    }
    return memSize;
}

int32_t MemMgrClient::GetTotalMemory()
{
    int32_t memSize = 0;
    if (GetTotalMemory(memSize) != 0) {
        return -1;
    }
    return memSize;
}

#else
int32_t MemMgrClient::RegisterActiveApps(int32_t pid, int32_t uid)
{
    return -1;
}

int32_t MemMgrClient::DeregisterActiveApps(int32_t pid, int32_t uid)
{
    return -1;
}

int32_t MemMgrClient::SubscribeAppState(const AppStateSubscriber &subscriber)
{
    return -1;
}

int32_t MemMgrClient::UnsubscribeAppState(const AppStateSubscriber &subscriber)
{
    return -1;
}

int32_t MemMgrClient::GetAvailableMemory(int32_t &memSize)
{
    return -1;
}

int32_t MemMgrClient::GetTotalMemory(int32_t &memSize)
{
    return -1;
}

int32_t MemMgrClient::GetAvailableMemory()
{
    return -1;
}

int32_t MemMgrClient::GetTotalMemory()
{
    return -1;
}
#endif // USE_PURGEABLE_MEMORY

int32_t MemMgrClient::OnWindowVisibilityChanged(const std::vector<sptr<MemMgrWindowInfo>> &MemMgrWindowInfo)
{
    HILOGD("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->OnWindowVisibilityChanged(MemMgrWindowInfo);
}

int32_t MemMgrClient::GetReclaimPriorityByPid(int32_t pid, int32_t &priority)
{
    HILOGD("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->GetReclaimPriorityByPid(pid, priority);
}

int32_t MemMgrClient::NotifyProcessStateChangedSync(const MemMgrProcessStateInfo &processStateInfo)
{
    HILOGD("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->NotifyProcessStateChangedSync(processStateInfo);
}

int32_t MemMgrClient::NotifyProcessStateChangedAsync(const MemMgrProcessStateInfo &processStateInfo)
{
    HILOGD("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->NotifyProcessStateChangedAsync(processStateInfo);
}

int32_t MemMgrClient::NotifyProcessStatus(int32_t pid, int32_t type, int32_t status, int saId)
{
    HILOGI("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->NotifyProcessStatus(pid, type, status, saId);
}

int32_t MemMgrClient::SetCritical(int32_t pid, bool critical, int32_t saId)
{
    HILOGI("called");
    auto dps = GetMemMgrService();
    if (dps == nullptr) {
        HILOGE("MemMgrService is null");
        return -1;
    }
    return dps->SetCritical(pid, critical, saId);
}

sptr<IMemMgr> MemMgrClient::GetMemMgrService()
{
    HILOGI("called");
    std::lock_guard<std::mutex> lock(mutex_);

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
