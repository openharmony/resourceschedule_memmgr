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
#ifndef OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_PROXY_H
#define OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_PROXY_H

#include "i_mem_mgr.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Memory {
class MemMgrProxy : public IRemoteProxy<IMemMgr> {
public:
    explicit MemMgrProxy(const sptr<IRemoteObject>& impl) : IRemoteProxy<IMemMgr>(impl) {}
    ~MemMgrProxy() = default;
    int32_t GetBundlePriorityList(BundlePriorityList &bundlePrioList) override;
    int32_t NotifyDistDevStatus(int32_t pid, int32_t uid, const std::string &name, bool connected) override;
    int32_t GetKillLevelOfLmkd(int32_t &killLevel) override;
#ifdef USE_PURGEABLE_MEMORY
    int32_t RegisterActiveApps(int32_t pid, int32_t uid) override;
    int32_t DeregisterActiveApps(int32_t pid, int32_t uid) override;
    int32_t SubscribeAppState(const sptr<IAppStateSubscriber> &subscriber) override;
    int32_t UnsubscribeAppState(const sptr<IAppStateSubscriber> &subscriber) override;
    int32_t GetAvailableMemory() override;
    int32_t GetTotalMemory() override;
#endif

private:
    static inline BrokerDelegator<MemMgrProxy> delegator_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_PROXY_H
