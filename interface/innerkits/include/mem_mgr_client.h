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

#ifndef OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_CLIENT_H
#define OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_CLIENT_H

#include "i_mem_mgr.h"
#include "single_instance.h"
#include "app_state_subscriber.h"

extern "C" {
    int32_t notify_process_status(int32_t pid, int32_t type, int32_t status, int saId = -1);
    int32_t set_critical(int32_t pid, bool critical, int32_t saId = -1);
}

namespace OHOS {
namespace Memory {
enum class MemoryTypeCode {
    DMABUF = 0,
};

enum class MemoryStatusCode {
    USED = 0,
    UNUSED = 1,
};

enum class DmabufRsInfo {
    INVALID = 0,
    IMAGE_ON_TREE;
    IMAGE_ON_TREE_IN_ROOT,
    IMAGE_OFF_TREE_IN_ROOT,
    IMAGE_OFF_TREE,

    SURFACE_ON_TREE,
    SURFACE_ON_TREE_IN_ROOT,
    SURFACE_OFF_TREE_IN_ROOT,
    SURFACE_OFF_TREE,
};

class MemMgrClient {
    DECLARE_SINGLE_INSTANCE(MemMgrClient);

public:
    int32_t GetBundlePriorityList(BundlePriorityList &bundlePrioList);
    int32_t NotifyDistDevStatus(int32_t pid, int32_t uid, const std::string &name, bool connected);
    int32_t GetKillLevelOfLmkd(int32_t &killLevel);
    int32_t RegisterActiveApps(int32_t pid, int32_t uid);
    int32_t DeregisterActiveApps(int32_t pid, int32_t uid);
    int32_t SubscribeAppState(const AppStateSubscriber &subscriber);
    int32_t UnsubscribeAppState(const AppStateSubscriber &subscriber);
    int32_t GetAvailableMemory(int32_t &memSize);
    int32_t GetTotalMemory(int32_t &memSize);
    int32_t GetAvailableMemory();
    int32_t GetTotalMemory();
    int32_t OnWindowVisibilityChanged(const std::vector<sptr<MemMgrWindowInfo>> &MemMgrWindowInfo);
    int32_t GetReclaimPriorityByPid(int32_t pid, int32_t &priority);
    int32_t NotifyProcessStateChangedSync(const MemMgrProcessStateInfo &processStateInfo);
    int32_t NotifyProcessStateChangedAsync(const MemMgrProcessStateInfo &processStateInfo);
    int32_t NotifyProcessStatus(int32_t pid, int32_t type, int32_t status, int saId = -1);
    int32_t SetCritical(int32_t pid, bool critical, int32_t saId = -1);
    int32_t MemoryStatusChanged(int32_t pid, int32_t type, int32_t status);
    int32_t SetDmabufUsage(int32_t fd, const std::string &usage);
    int32_t Reclaim(int32_t pid, int32_t fd);
    int32_t Resume(int32_t pid, int32_t fd);
    int32_t SetDmabufInfo(int32_t fd, DmabufRsInfo info);
    
private:
    sptr<IMemMgr> GetMemMgrService();
    std::mutex mutex_;
    sptr<IMemMgr> dpProxy_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_CLIENT_H
