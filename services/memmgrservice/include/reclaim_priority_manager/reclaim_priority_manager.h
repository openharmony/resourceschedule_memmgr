/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "memmgr_config_manager.h"

#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <set>

namespace OHOS {
namespace Memory {
struct ReclaimHandleRequest {
    int32_t callerPid;
    int32_t callerUid;
    pid_t pid;
    int uid;
    std::string callerBundleName;
    std::string bundleName;
    AppStateUpdateReason reason;
};

struct ReqProc {
    int pid;
    int uid;
    std::string bundleName;
};

struct UpdateRequest {
    ReqProc caller;
    ReqProc target;
    AppStateUpdateReason reason;
};

inline UpdateRequest SingleRequest(const ReqProc &target, AppStateUpdateReason reason)
{
    UpdateRequest request;
    request.target = target;
    request.reason = reason;
    return request;
}

inline UpdateRequest CallerRequest(const ReqProc &caller, const ReqProc &target, AppStateUpdateReason reason)
{
    UpdateRequest request;
    request.caller = caller;
    request.target = target;
    request.reason = reason;
    return request;
}

class ReclaimPriorityManager {
    DECLARE_SINGLE_INSTANCE_BASE(ReclaimPriorityManager);

public:
    struct BundleInfoPtrCmp {
        bool operator() (const std::shared_ptr<BundlePriorityInfo> p1, const std::shared_ptr<BundlePriorityInfo> p2)
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
        bool operator() (const BundlePriorityInfo &p1, const BundlePriorityInfo &p2)
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

    struct ProcInfoCmpByPriority {
        bool operator() (const ProcessPriorityInfo &p1, const ProcessPriorityInfo &p2)
        {
            if (p1.pid_ == p2.pid_) {
                return false;
            } else {
                if (p1.priority_ != p2.priority_) {
                    return p1.priority_ < p2.priority_;
                }

                return p1.pid_ < p2.pid_;
            }
        }
    };

    using BundlePrioSet = std::set<std::shared_ptr<BundlePriorityInfo>, BundleInfoPtrCmp>;
    using BunldeCopySet = std::set<BundlePriorityInfo, BundleInfoCmp>;
    // map <bundleUid, std::shared_ptr<BundlePriorityInfo>>
    using BundlePrioMap = std::map<int, std::shared_ptr<BundlePriorityInfo>>;
    using OsAccountsMap = std::map<int, std::shared_ptr<AccountBundleInfo>>;
    using ProcInfoVec = std::vector<ProcessPriorityInfo>;
    using ProcInfoSet = std::set<ProcessPriorityInfo, ProcInfoCmpByPriority>;
    bool Init();
    bool UpdateReclaimPriority(UpdateRequest request);
    bool OsAccountChanged(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod);

    inline bool Initailized()
    {
        return initialized_;
    };

    // for lmkd and memory reclaim
    void GetBundlePrioSet(BunldeCopySet &bundleSet);

    void GetOneKillableBundle(int minPrio, BunldeCopySet &bundleSet);

    void SetBundleState(int accountId, int uid, BundleState state);

    // for hidumper, usage: hdc shell hidumper -s 1909
    void Dump(int fd);

    void Reset();
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
    std::map<int32_t, std::string> updateReasonStrMapping_;
    std::string UNKOWN_REASON = "UNKOWN_REASON";
    ReclaimPriorityConfig config_;
    std::set<std::string> allKillableSystemApps_;
    using ChangeProcFunc = void (ReclaimPriorityManager::*)(ProcessPriorityInfo &proc, AppAction &action);
    std::map<AppStateUpdateReason, ChangeProcFunc> changeProcMapping_;

    ReclaimPriorityManager();
    void InitUpdateReasonStrMapping();
    void InitChangeProcMapping();
    bool GetEventHandler();
    void UpdateForegroundApps();
    bool IsFrontApp(const std::string& pkgName, int32_t uid, int32_t pid);
    void GetAllKillableSystemApps();
    void GetKillableSystemAppsFromAms(std::set<std::string> &killableApps);
    void HandlePreStartedProcs();
    bool UpdateReclaimPriorityInner(UpdateRequest request);
    bool HandleExtensionProcess(UpdateRequest &request);
    bool OsAccountChangedInner(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod);
    bool UpdateAllPrioForOsAccountChanged(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod);
    bool ApplyReclaimPriority(std::shared_ptr<BundlePriorityInfo> bundle, pid_t pid, AppAction action);
    bool IsProcExist(pid_t pid, int bundleUid, int accountId);
    bool IsOsAccountExist(int accountId);
    bool HandleCreateProcess(ReqProc &request, int accountId, bool isRender = false);
    bool HandleTerminateProcess(ProcessPriorityInfo proc, std::shared_ptr<BundlePriorityInfo> bundle,
            std::shared_ptr<AccountBundleInfo> account);
    void HandleUpdateProcess(AppStateUpdateReason reason, std::shared_ptr<BundlePriorityInfo> bundle,
            ProcessPriorityInfo &proc, AppAction &action);
    void UpdatePriorityByProcStatus(std::shared_ptr<BundlePriorityInfo> bundle, ProcessPriorityInfo &proc);
    bool HandleApplicationSuspend(std::shared_ptr<BundlePriorityInfo> bundle);
    std::shared_ptr<AccountBundleInfo> FindOsAccountById(int accountId);
    void RemoveOsAccountById(int accountId);
    void AddOsAccountInfo(std::shared_ptr<AccountBundleInfo> account);
    bool IsKillableSystemApp(std::shared_ptr<BundlePriorityInfo> bundle);
    void NotifyKillableSystemAppsAdded(std::set<std::string> &newKillableApps);
    void CheckAndSetImportantApp(pid_t pid, std::shared_ptr<BundlePriorityInfo> bundle);
    bool IsImportantProc(ProcessPriorityInfo proc, int &dstPriority);
    bool UpdateExtensionStatusForCaller(UpdateRequest &request);
    bool UpdateExtensionStatusForTarget(UpdateRequest &request);
    void UpdatePriorityByProcForExtension(ProcessPriorityInfo &proc);
    void UpdatePriorityByProcConnector(ProcessPriorityInfo &proc);
    bool HandleUpdateExtensionBundle(int bundleUid);
    void SetTimerForDiedProcessCheck(int64_t delayTime);
    void FilterDiedProcess();
    void HandleDiedProcessCheck();
    void HandleDiedExtensionBindToMe(std::map<pid_t, ProcessPriorityInfo>::iterator processPriorityInfoMap,
        const std::vector<unsigned int> &alivePids);
    void HandleDiedExtensionBindFromMe(std::map<pid_t, ProcessPriorityInfo>::iterator processPriorityInfoMap,
        const std::vector<unsigned int> &alivePids);
    void GetConnectedExtensionProc(const ProcessPriorityInfo &proc, ProcInfoVec &procVec);
    void CalculateExtensionProcPrio(ProcInfoVec &proc, ProcInfoSet &procSet);
    void SetConnectExtensionProcPrio(const ProcInfoSet &procSet);

    // two methods below used to manage totalBundlePrioSet_ by BundlePriorityInfo
    void AddBundleInfoToSet(std::shared_ptr<BundlePriorityInfo> bundle);
    void UpdateBundlePriority(std::shared_ptr<BundlePriorityInfo> bundle);
    void DeleteBundleInfoFromSet(std::shared_ptr<BundlePriorityInfo> bundle);

    std::string& AppStateUpdateResonToString(AppStateUpdateReason reason);

    void HandleForeground(ProcessPriorityInfo &proc, AppAction &action);
    void HandleBackground(ProcessPriorityInfo &proc, AppAction &action);
    void HandleSuspendDelayStart(ProcessPriorityInfo &proc, AppAction &action);
    void HandleSuspendDelayEnd(ProcessPriorityInfo &proc, AppAction &action);
    void HandleBackgroundRunningStart(ProcessPriorityInfo &proc, AppAction &action);
    void HandleBackgroundRunningEnd(ProcessPriorityInfo &proc, AppAction &action);
    void HandleEventStart(ProcessPriorityInfo &proc, AppAction &action);
    void HandleEventEnd(ProcessPriorityInfo &proc, AppAction &action);
    void HandleDistDeviceConnected(ProcessPriorityInfo &proc, AppAction &action);
    void HandleDistDeviceDisconnected(ProcessPriorityInfo &proc, AppAction &action);
    void HandleBindExtension(ProcessPriorityInfo &proc, AppAction &action);
    void HandleUnbindExtension(ProcessPriorityInfo &proc, AppAction &action);
    void HandleVisible(ProcessPriorityInfo &proc, AppAction &action);
    void HandleUnvisible(ProcessPriorityInfo &proc, AppAction &action);

    static inline int GetOsAccountLocalIdFromUid(int bundleUid)
    {
        return GET_OS_ACCOUNT_ID_BY_UID(bundleUid);
    }
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECALIM_PRIORITY_MANAGER_H
