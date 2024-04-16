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

#ifndef OHOS_MEMORY_MEMMGR_SERVICE_H
#define OHOS_MEMORY_MEMMGR_SERVICE_H

#include "mem_mgr_stub.h"
#include "single_instance.h"
#include "system_ability.h"

namespace OHOS {
namespace Memory {
class MemMgrService : public SystemAbility, public MemMgrStub {
    DECLARE_SYSTEM_ABILITY(MemMgrService);
    DECLARE_SINGLE_INSTANCE_BASE(MemMgrService);

public:
    MemMgrService();
    ~MemMgrService() = default;
    virtual int32_t GetBundlePriorityList(BundlePriorityList &bundlePrioList) override;
    virtual int32_t NotifyDistDevStatus(int32_t pid, int32_t uid, const std::string &name, bool connected) override;
    virtual int32_t GetKillLevelOfLmkd(int32_t &killLevel) override;
#ifdef USE_PURGEABLE_MEMORY
    virtual int32_t RegisterActiveApps(int32_t pid, int32_t uid) override;
    virtual int32_t DeregisterActiveApps(int32_t pid, int32_t uid) override;
    virtual int32_t SubscribeAppState(const sptr<IAppStateSubscriber> &subscriber) override;
    virtual int32_t UnsubscribeAppState(const sptr<IAppStateSubscriber> &subscriber) override;
    virtual int32_t GetAvailableMemory(int32_t &memSize) override;
    virtual int32_t GetTotalMemory(int32_t &memSize) override;
#endif
    virtual int32_t OnWindowVisibilityChanged(const std::vector<sptr<MemMgrWindowInfo>> &MemMgrWindowInfo) override;
    virtual int32_t GetReclaimPriorityByPid(int32_t pid, int32_t &priority) override;
    virtual int32_t NotifyProcessStateChangedSync(const MemMgrProcessStateInfo &processStateInfo) override;
    virtual int32_t NotifyProcessStateChangedAsync(const MemMgrProcessStateInfo &processStateInfo) override;
    virtual int32_t NotifyProcessStatus(int32_t pid, int32_t type, int32_t status, int32_t saId = -1) override;
    virtual int32_t SetCritical(int32_t pid, bool critical, int32_t saId = -1) override;
    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;
    virtual int Dump(int fd, const std::vector<std::u16string> &args) override;

protected:
    void OnStart() override;
    void OnStop() override;

private:
    int32_t windowManagerUid_ = 5523;
    bool Init();
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_SERVICE_H
