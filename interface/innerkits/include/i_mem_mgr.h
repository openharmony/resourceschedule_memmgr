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

#ifndef OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_INTERFACE_H
#define OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_INTERFACE_H

#include <vector>

#include "bundle_priority_list.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "mem_mgr_process_state_info.h"
#include "mem_mgr_window_info.h"
#include "memmgrservice_ipc_interface_code.h"
#ifdef USE_PURGEABLE_MEMORY
#include "iapp_state_subscriber.h"
#endif

namespace OHOS {
namespace Memory {
class IMemMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.memory.MemMgr");

    virtual int32_t GetBundlePriorityList(BundlePriorityList &bundlePrioList) = 0;

    virtual int32_t NotifyDistDevStatus(int32_t pid, int32_t uid, const std::string &name, bool connected) = 0;

    virtual int32_t GetKillLevelOfLmkd(int32_t &killLevel) = 0;

#ifdef USE_PURGEABLE_MEMORY
    virtual int32_t RegisterActiveApps(int32_t pid, int32_t uid) = 0;

    virtual int32_t DeregisterActiveApps(int32_t pid, int32_t uid) = 0;

    virtual int32_t SubscribeAppState(const sptr<IAppStateSubscriber> &subscriber) = 0;

    virtual int32_t UnsubscribeAppState(const sptr<IAppStateSubscriber> &subscriber) = 0;

    virtual int32_t GetAvailableMemory(int32_t &memSize) = 0;

    virtual int32_t GetTotalMemory(int32_t &memSize) = 0;
#endif

    virtual int32_t OnWindowVisibilityChanged(const std::vector<sptr<MemMgrWindowInfo>> &MemMgrWindowInfo) = 0;
    virtual int32_t GetReclaimPriorityByPid(int32_t pid, int32_t &priority) = 0;
    virtual int32_t NotifyProcessStateChangedSync(const MemMgrProcessStateInfo &processStateInfo) = 0;
    virtual int32_t NotifyProcessStateChangedAsync(const MemMgrProcessStateInfo &processStateInfo) = 0;
    virtual int32_t NotifyProcessStatus(int32_t pid, int32_t type, int32_t status, int saId = -1) = 0;
    virtual int32_t SetCritical(int32_t pid, bool critical, int32_t saId = -1) = 0;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_INTERFACE_H
