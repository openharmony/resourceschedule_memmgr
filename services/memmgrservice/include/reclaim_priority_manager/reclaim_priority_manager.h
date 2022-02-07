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

#ifndef OHOS_MEMORY_MEMMGR_RECALIM_PRIORITY_MANAGER_H
#define OHOS_MEMORY_MEMMGR_RECALIM_PRIORITY_MANAGER_H

#include "single_instance.h"
#include "event_handler.h"
#include "reclaim_priority_constants.h"
#include "process_priority_info.h"
#include "bundle_priority_info.h"
#include "os_account_priority_info.h"

#include <map>
#include <string>
#include <set>

namespace OHOS {
namespace Memory {
class ReclaimPriorityManager {
    DECLARE_SINGLE_INSTANCE_BASE(ReclaimPriorityManager);

public:
    struct BundleInfoPtrCmp {
        bool operator()(const BundlePriorityInfo* p1, const BundlePriorityInfo* p2)
        {
            if (p1->priority_ <= p2->priority_) {
                return true;
            } else {
                return false;
            }
        }
    };

    using BundlePrioSet = std::set<BundlePriorityInfo*, BundleInfoPtrCmp>;
    // map <bundleUid, BundlePriorityInfo*>
    using BundlePrioMap = std::map<int, BundlePriorityInfo*>;
    bool Init();
    bool UpdateReclaimPriority(pid_t pid, int bundleUid, std::string bundleName, AppStateUpdateReason priorityReason);
    bool UpdateAllReclaimPriority(AppStateUpdateReason priorityReason);
    bool CurrentOsAccountChanged(int curAccountId);

    // two methods below used to manage totalBundlePrioSet_ by BundlePriorityInfo
    void AddBundleInfoToSet(BundlePriorityInfo* bundle);
    void UpdateBundlePriority(BundlePriorityInfo* bundle);
    void DeleteBundleInfoFromSet(BundlePriorityInfo* bundle);

inline bool Initailized() 
    {
        return initialized_;
    };

protected:
    // for lmkd and memory reclaim
    const BundlePrioSet GetBundlePrioSet();

private:
    bool initialized_ = false;
    int preOsAccountId_ = 0;
    int curOsAccountId_ = 0;

    // map <accountId, accountInfo>
    std::map<int, OsAccountPriorityInfo> osAccountsInfoMap_;
    // total system prioritySet
    // when new a BundlePriorityInfo, it will be added into this set
    // when delete a BundlePriorityInfo, it will be removed from this set
    // when change the priority of BundlePriorityInfo, it will be removed and added from this set to re-sort it
    BundlePrioSet totalBundlePrioSet_;

    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    ReclaimPriorityManager();
    bool GetEventHandler();
    bool UpdateReclaimPriorityInner(pid_t pid, int bundleUid, std::string bundleName,
            AppStateUpdateReason priorityReason);
    bool CurrentOsAccountChangedInner(int curAccountId);
    bool ApplyReclaimPriority(BundlePriorityInfo *bundle);
    bool IsProcExist(pid_t pid, int bundleUid, int accountId);
    bool IsOsAccountExist(int accountId);
    bool HandleCreateProcess(int pid, int bundleUid, std::string bundleName, int accountId);
    void HandleTerminateProcess(ProcessPriorityInfo &proc, BundlePriorityInfo *bundle,
            OsAccountPriorityInfo *account);
    bool HandleApplicationSuspend(BundlePriorityInfo *bundle);
    OsAccountPriorityInfo* FindOsAccountById(int accountId);
    void RemoveOsAccountById(int accountId);
    void AddOsAccountInfo(OsAccountPriorityInfo account);
    bool IsSystemApp(BundlePriorityInfo* bundle);

    static inline int GetOsAccountLocalIdFromUid(int bundleUid)
    {
        return bundleUid / USER_ID_SHIFT;
    }
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECALIM_PRIORITY_MANAGER_H
