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


#include "reclaim_priority_manager.h"
#include "memmgr_log.h"
#include "kernel_interface.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "ReclaimPriorityManager";
}
IMPLEMENT_SINGLE_INSTANCE(ReclaimPriorityManager);

bool WriteOomScoreAdjToKernel(const BundlePriorityInfo *bundle)
{
    if (bundle == nullptr) {
        return false;
    }
    int pathSize = 30;
    int contentSize = 10;
    char path[pathSize];
    char content[contentSize];
    for (auto i = bundle->procs_.begin(); i != bundle->procs_.end(); ++i) {
        int priority = i->second.priority_;
        int pid = i->second.pid_;
        snprintf(path, pathSize, "/proc/%d/oom_score_adj", pid);
        snprintf(content, contentSize, "%d", priority);
        KernelInterface::GetInstance().EchoToPath(path, content);
    }
    return true;
}

ReclaimPriorityManager::ReclaimPriorityManager()
{
}

bool ReclaimPriorityManager::Init()
{
    initialized_ = GetEventHandler();
    if (initialized_) {
        HILOGI("init successed");
    } else {
        HILOGE("init failed");
    }
    return initialized_;
}

bool ReclaimPriorityManager::GetEventHandler()
{
    if (!handler_) {
        handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
        if (handler_ == nullptr) {
            HILOGE("handler init failed");
            return false;
        }
    }
    return true;
}

const ReclaimPriorityManager::BundlePrioSet ReclaimPriorityManager::GetBundlePrioSet()
{
    return totalBundlePrioSet_;
}

bool ReclaimPriorityManager::IsOsAccountExist(int accountId)
{
    if (osAccountsInfoMap_.find(accountId) == osAccountsInfoMap_.end()) {
        HILOGE("accountId not exist");
        return false;
    }
    return true;
}

void ReclaimPriorityManager::AddBundleInfoToSet(BundlePriorityInfo* bundle)
{
    totalBundlePrioSet_.insert(bundle);
}

void ReclaimPriorityManager::DeleteBundleInfoFromSet(BundlePriorityInfo* bundle)
{
    totalBundlePrioSet_.erase(bundle);
}

OsAccountPriorityInfo* ReclaimPriorityManager::FindOsAccountById(int accountId)
{
    return &(osAccountsInfoMap_.at(accountId));
}

void ReclaimPriorityManager::RemoveOsAccountById(int accountId)
{
    // erase the accountId data
    osAccountsInfoMap_.erase(accountId);
}

void ReclaimPriorityManager::AddOsAccountInfo(OsAccountPriorityInfo account)
{
    osAccountsInfoMap_.insert(std::make_pair(account.id_, account));
}

bool ReclaimPriorityManager::IsProcExist(pid_t pid, int bundleUid, int accountId)
{
    if (!IsOsAccountExist(accountId)) {
        return false;
    }
    OsAccountPriorityInfo *account = FindOsAccountById(accountId);
    if (!account->HasBundle(bundleUid)) {
        HILOGE("bundle name not exist");
        return false;
    }
    BundlePriorityInfo* bundle = account->FindBundleById(bundleUid);
    if (!bundle->HasProc(pid)) {
        HILOGE("pid not exist");
        return false;
    }
    return true;
}

bool ReclaimPriorityManager::UpdateReclaimPriority(pid_t pid, int bundleUid,
    std::string name, AppStateUpdateReason reason)
{
    if (!initialized_) {
        HILOGE("has not been initialized_, skiped!");
        return false;
    }
    std::function<bool()> updateReclaimPriorityInnerFunc =
        std::bind(&ReclaimPriorityManager::UpdateReclaimPriorityInner, this, pid, bundleUid, name, reason);
    return handler_->PostImmediateTask(updateReclaimPriorityInnerFunc);
}

bool ReclaimPriorityManager::IsSystemApp(BundlePriorityInfo* bundle)
{
    // special case: launcher and system ui bundle
    if (bundle != nullptr && (bundle->name_.compare(LAUNCHER_BUNDLE_NAME) == 0 ||
            bundle->name_.compare(SYSTEM_UI_BUNDLE_NAME) == 0)) {
        return true;
    }
    return false;
}

void ReclaimPriorityManager::UpdateBundlePriority(BundlePriorityInfo *bundle)
{
    DeleteBundleInfoFromSet(bundle);
    bundle->UpdatePriority();
    AddBundleInfoToSet(bundle);
}

bool ReclaimPriorityManager::HandleCreateProcess(int pid, int bundleUid, std::string bundleName, int accountId)
{
    if (!IsOsAccountExist(accountId)) {
        OsAccountPriorityInfo newAccount(accountId, true);
        AddOsAccountInfo(newAccount);
    }
    OsAccountPriorityInfo* account = FindOsAccountById(accountId);
    BundlePriorityInfo *bundle;
    if (account->HasBundle(bundleUid)) {
        // insert new ProcessInfo and update new priority
        bundle =  account->FindBundleById(bundleUid);
    } else {
        // need to new BundleInfo ,add to list and map
        bundle = new BundlePriorityInfo(bundleName, bundleUid, RECLAIM_PRIORITY_FOREGROUND);
        AddBundleInfoToSet(bundle);
    }
    ProcessPriorityInfo *proc;
    if (IsSystemApp(bundle)) {
        proc = new ProcessPriorityInfo(pid, bundleUid, RECLAIM_PRIORITY_SYSTEM);
    } else {
        proc = new ProcessPriorityInfo(pid, bundleUid, RECLAIM_PRIORITY_FOREGROUND);
    }
    bundle->AddProc(*proc);
    UpdateBundlePriority(bundle);
    account->AddBundleToOsAccount(bundle);
    bool ret = ApplyReclaimPriority(bundle);
    HILOGI("create: bundleName=%{public}s, prio=%{public}d", bundleName.c_str(), bundle->priority_);
    return ret;
}

void ReclaimPriorityManager::HandleTerminateProcess(ProcessPriorityInfo &proc,
    BundlePriorityInfo *bundle, OsAccountPriorityInfo *account)
{
    // clear proc and bundle if needed, delete the object
    HILOGI("terminated: bundleName=%{public}s, pid=%{public}d", bundle->name_.c_str(), proc.pid_);
    int removedProcessPrio = proc.priority_;
    bundle->RemoveProcByPid(proc.pid_);

    if (bundle->GetProcsCount() == 0) {
        account->RemoveBundleById(bundle->uid_);
        DeleteBundleInfoFromSet(bundle);
        delete bundle;
        bundle = nullptr;
    } else {
        if (removedProcessPrio <= bundle->priority_) {
            UpdateBundlePriority(bundle);
        }
    }
    if (account->GetBundlesCount() == 0) {
        RemoveOsAccountById(account->id_);
    }
}

bool ReclaimPriorityManager::UpdateReclaimPriorityInner(pid_t pid, int bundleUid, 
    std::string bundleName, AppStateUpdateReason reason)
{
    int accountId = GetOsAccountLocalIdFromUid(bundleUid);

    if (reason == AppStateUpdateReason::CREATE_PROCESS) {
        bool ret = HandleCreateProcess(pid, bundleUid, bundleName, accountId);
        return ret;
    }

    if (!IsProcExist(pid, bundleUid, accountId)) {
        HILOGE("process not exist and not to create it!!");
        return false;
    }

    OsAccountPriorityInfo* account = FindOsAccountById(accountId);
    BundlePriorityInfo *bundle =  account->FindBundleById(bundleUid);
    if (bundle->priority_ == RECLAIM_PRIORITY_SYSTEM) {
        HILOGI("%{public}s is system app, skip!", bundleName.c_str());
        return true;
    }

    ProcessPriorityInfo &proc = bundle->FindProcByPid(pid);

    bool ret = true;
    switch (reason) {
        case AppStateUpdateReason::CREATE_PROCESS: {
            HILOGE("not supposed to reach here!");
            return false;
        }
        case AppStateUpdateReason::PROCESS_TERMINATED: {
            HandleTerminateProcess(proc, bundle, account);
            break;
        }
        case AppStateUpdateReason::FOREGROUND: {
            proc.SetPriority(RECLAIM_PRIORITY_FOREGROUND);
            UpdateBundlePriority(bundle);
            break;
        }
        case AppStateUpdateReason::BACKGROUND: {
            proc.SetPriority(RECLAIM_PRIORITY_BACKGROUND);
            UpdateBundlePriority(bundle);
            break;
        }
        case AppStateUpdateReason::PROCESS_SUSPEND: {
            proc.SetPriority(RECLAIM_PRIORITY_SUSPEND);
            UpdateBundlePriority(bundle);
            break;
        }
        case AppStateUpdateReason::SUSPEND_DELAY_START:
            proc.isSuspendDelay = true;
            if (bundle->priority_ > RECLAIM_PRIORITY_BG_SUSPEND_DELAY) {
                proc.SetPriority(RECLAIM_PRIORITY_BG_SUSPEND_DELAY);
                UpdateBundlePriority(bundle);
            }
            break;
        case AppStateUpdateReason::SUSPEND_DELAY_END:
            proc.isSuspendDelay = false;
            if (bundle->priority_ == RECLAIM_PRIORITY_BG_SUSPEND_DELAY) {
                proc.SetPriority(RECLAIM_PRIORITY_BACKGROUND);
                UpdateBundlePriority(bundle);
            }
            break;
        case AppStateUpdateReason::BACKGROUND_RUNNING_START:
            proc.isBackgroundRunning = true;
            break;
        case AppStateUpdateReason::BACKGROUND_RUNNING_END:
            proc.isBackgroundRunning = false;
            break;
        case AppStateUpdateReason::EVENT_START:
            proc.isEventStart = true;
            break;
        case AppStateUpdateReason::EVENT_END:
            proc.isEventStart = false;
            break;
        case AppStateUpdateReason::DATA_ABILITY_START:
            proc.isDataAbilityStart = true;
            break;
        case AppStateUpdateReason::DATA_ABILITY_END:
            proc.isDataAbilityStart = false;
            break;
        default:
            break;
    }
    // if priority of the process or the bundle is smaller than RECLAIM_PRIORITY_BACKGROUND, it need not to update
    if (proc.isBackgroundRunning || proc.isEventStart || proc.isDataAbilityStart) {
        if (bundle->priority_ > RECLAIM_PRIORITY_BG_PERCEIVED) {
            proc.SetPriority(RECLAIM_PRIORITY_BG_PERCEIVED);
            UpdateBundlePriority(bundle);
        }
    } else if (bundle->priority_ == RECLAIM_PRIORITY_BG_PERCEIVED) {
        proc.SetPriority(RECLAIM_PRIORITY_BACKGROUND);
        UpdateBundlePriority(bundle);
    }
    ret = ApplyReclaimPriority(bundle);
    return ret;
}

bool ReclaimPriorityManager::ApplyReclaimPriority(BundlePriorityInfo *bundle)
{
    return WriteOomScoreAdjToKernel(bundle);
}

bool ReclaimPriorityManager::CurrentOsAccountChanged(int curAccountId)
{
    if (!initialized_) {
        HILOGE("has not been initialized_, skiped!");
        return false;
    }
    if (curAccountId < 0) {
        HILOGE("invalid account id!");
        return false;
    }
    std::function<bool()> currentOsAccountChangedInnerFunc =
        std::bind(&ReclaimPriorityManager::CurrentOsAccountChangedInner, this, curAccountId);
    return handler_->PostImmediateTask(currentOsAccountChangedInnerFunc);
}

bool ReclaimPriorityManager::CurrentOsAccountChangedInner(int curAccountId)
{
    preOsAccountId_ = curOsAccountId_;
    curOsAccountId_ = curAccountId;
    for (auto i = osAccountsInfoMap_.begin(); i != osAccountsInfoMap_.end(); ++i) {
        i->second.isCurOsAccount_ = (i->first == curOsAccountId_) ? true : false;
        i->second.isPreOsAccount_ = (i->first == preOsAccountId_) ? true : false;
    }
    return UpdateAllReclaimPriority(AppStateUpdateReason::OS_ACCOUNT_HOT_SWITCH);
}

bool ReclaimPriorityManager::UpdateAllReclaimPriority(AppStateUpdateReason reason)
{
    if (!initialized_) {
        HILOGE("has not been initialized_, skiped!");
        return false;
    }
    switch (reason) {
        case AppStateUpdateReason::OS_ACCOUNT_HOT_SWITCH: {
            if (curOsAccountId_ == preOsAccountId_) { // now there is only one user
                HILOGI("now there is only one user<%{public}d>, do nothing", curOsAccountId_);
            } else {
                OsAccountPriorityInfo* curOsAccount = FindOsAccountById(curOsAccountId_);
                curOsAccount->PromoteAllBundlePriority(curOsAccount->priorityShift_);
                OsAccountPriorityInfo* preOsAccount = FindOsAccountById(preOsAccountId_);
                preOsAccount->ReduceAllBundlePriority(curOsAccount->priorityShift_);
            }
            break;
        }
        default:
            break;
    }
    HILOGI("UpdateReclaimPriority for all apps");
    return true;
}
} // namespace Memory
} // namespace OHOS
