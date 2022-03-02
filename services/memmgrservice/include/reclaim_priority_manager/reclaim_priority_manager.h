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
#include "account_bundle_info.h"
#include "os_account_manager.h"
#include "reclaim_param.h"

#include <map>
#include <mutex>
#include <string>
#include <set>

namespace OHOS {
namespace Memory {
class ReclaimPriorityManager {
    DECLARE_SINGLE_INSTANCE_BASE(ReclaimPriorityManager);

public:
    struct BundleInfoPtrCmp {
        bool operator() (const BundlePriorityInfo *p1, const BundlePriorityInfo *p2)
        {
            if (p1->uid_ == p2->uid_) {
                // remove duplicate BundlePriorityInfo according to uid_
                return false;
            } else {
                if (p1->priority_ != p2->priority_) {
                    // in ascending order
                    return p1->priority_ < p2->priority_;
                } else {
                    // when in same priority_, sort by uid_
                    // it will be sorted by last used time
                    return p1->uid_ < p2->uid_;
                }
            }

        }
    };
    struct BundleInfoCmp {
        bool operator() (const BundlePriorityInfo p1, const BundlePriorityInfo p2)
        {
            if (p1.uid_ == p2.uid_) {
                // remove duplicate BundlePriorityInfo according to uid_
                return false;
            } else {
                if (p1.priority_ != p2.priority_) {
                    // in descending order
                    return p1.priority_ > p2.priority_;
                } else {
                    // when in same priority_, sort by uid_
                    // it will be sorted by last used time
                    return p1.uid_ < p2.uid_;
                }
            }
        }
    };

    using BundlePrioSet = std::set<BundlePriorityInfo*, BundleInfoPtrCmp>;
    using BunldeCopySet = std::set<BundlePriorityInfo, BundleInfoCmp>;
    // map <bundleUid, BundlePriorityInfo*>
    using BundlePrioMap = std::map<int, BundlePriorityInfo*>;
    using OsAccountsMap = std::map<int, AccountBundleInfo>;
    bool Init();
    bool UpdateReclaimPriority(pid_t pid, int bundleUid, std::string bundleName, AppStateUpdateReason priorityReason);
    bool OsAccountChanged(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod);

    // two methods below used to manage totalBundlePrioSet_ by BundlePriorityInfo
    void AddBundleInfoToSet(BundlePriorityInfo *bundle);
    void UpdateBundlePriority(BundlePriorityInfo *bundle);
    void DeleteBundleInfoFromSet(BundlePriorityInfo *bundle);

    inline bool Initailized()
    {
        return initialized_;
    };

    // for lmkd and memory reclaim
    void GetBundlePrioSet(BunldeCopySet &bundleSet);

    void SetBundleState(int accountId, int uid, BundleState state);

private:
    bool initialized_ = false;

    // map <accountId, accountInfo>
    OsAccountsMap osAccountsInfoMap_;
    // total system prioritySet
    // when new a BundlePriorityInfo, it will be added into this set
    // when delete a BundlePriorityInfo, it will be removed from this set
    // when change the priority of BundlePriorityInfo, it will be removed and added from this set to re-sort it
    BundlePrioSet totalBundlePrioSet_;
    std::mutex totalBundlePrioSetLock_;

    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    ReclaimPriorityManager();
    bool GetEventHandler();
    bool UpdateReclaimPriorityInner(pid_t pid, int bundleUid, std::string bundleName,
            AppStateUpdateReason priorityReason);
    bool OsAccountChangedInner(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod);
    bool UpdateAllPrioForOsAccountChanged(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod);
    bool ApplyReclaimPriority(BundlePriorityInfo *bundle, pid_t pid, AppAction action);
    bool IsProcExist(pid_t pid, int bundleUid, int accountId);
    bool IsOsAccountExist(int accountId);
    bool HandleCreateProcess(int pid, int bundleUid, std::string bundleName, int accountId);
    bool HandleTerminateProcess(ProcessPriorityInfo &proc, BundlePriorityInfo *bundle,
            AccountBundleInfo *account);
    void HandleUpdateProcess(AppStateUpdateReason reason, BundlePriorityInfo *bundle,
            ProcessPriorityInfo &proc, AppAction &action);
    bool HandleApplicationSuspend(BundlePriorityInfo *bundle);
    AccountBundleInfo* FindOsAccountById(int accountId);
    void RemoveOsAccountById(int accountId);
    void AddOsAccountInfo(AccountBundleInfo account);
    bool IsSystemApp(BundlePriorityInfo *bundle);

    static inline int GetOsAccountLocalIdFromUid(int bundleUid)
    {
        return bundleUid / USER_ID_SHIFT;
    }
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECALIM_PRIORITY_MANAGER_H
