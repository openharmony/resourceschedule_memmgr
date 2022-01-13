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

bool WriteOomScoreAdjToKernel(const BundlePriorityInfo *bInfo)
{
    if (bInfo == nullptr) {
        return false;
    }
    int pathSize = 30;
    int contentSize = 10;
    char path[pathSize];
    char content[contentSize];
    for (auto i = bInfo->processes_.begin(); i != bInfo->processes_.end(); ++i) {
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

bool ReclaimPriorityManager::IsUserExist(int accountId)
{
    if (osAccountsInfoMap_.find(accountId) == osAccountsInfoMap_.end()) {
        HILOGE("accountId not exist");
        return false;
    }
    return true;
}

void ReclaimPriorityManager::AddBundleInfoToSet(BundlePriorityInfo* bundleInfo)
{
    totalBundlePrioSet_.insert(bundleInfo);
}

void ReclaimPriorityManager::DeleteBundleInfoFromSet(BundlePriorityInfo* bundleInfo)
{
    totalBundlePrioSet_.erase(bundleInfo);
}

OsAccountPriorityInfo* ReclaimPriorityManager::FindUserInfoById(int accountId)
{
    return &(osAccountsInfoMap_.at(accountId));
}

void ReclaimPriorityManager::RemoveUserInfoById(int accountId)
{
    // erase the accountId data
    osAccountsInfoMap_.erase(accountId);
}

void ReclaimPriorityManager::AddOsAccountInfo(OsAccountPriorityInfo account)
{
    osAccountsInfoMap_.insert(std::make_pair(account.id_, account));
}

bool ReclaimPriorityManager::IsProcessExist(pid_t pid, int bundleUid, int accountId)
{
    if (!IsUserExist(accountId)) {
        return false;
    }
    OsAccountPriorityInfo *osAccountInfo = FindUserInfoById(accountId);
    if (!osAccountInfo->BundleExist(bundleUid)) {
        HILOGE("bundle name not exist");
        return false;
    }
    BundlePriorityInfo* bundleInfo = osAccountInfo->FindBundleInfoById(bundleUid);
    if (!bundleInfo->ProcessExistInBundle(pid)) {
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

bool ReclaimPriorityManager::IsSystemApp(BundlePriorityInfo &bundleInfo)
{
    // special case: launcher and system ui bundle
    if (bundleInfo.name_ == LAUNCHER_BUNDLE_NAME || bundleInfo.name_ == SYSTEM_UI_BUNDLE_NAME) {
        return true;
    }
    return false;
}

void ReclaimPriorityManager::UpdateBundlePriority(BundlePriorityInfo *bundleInfo)
{
    DeleteBundleInfoFromSet(bundleInfo);
    bundleInfo->UpdatePriority();
    AddBundleInfoToSet(bundleInfo);
}

bool ReclaimPriorityManager::HandleCreateProcess(int pid, int bundleUid, std::string bundleName, int accountId)
{
    if (!IsUserExist(accountId)) {
        OsAccountPriorityInfo newAccount(accountId, true);
        AddOsAccountInfo(newAccount);
    }
    OsAccountPriorityInfo* osAccountInfo = FindUserInfoById(accountId);
    BundlePriorityInfo *bundleInfo;
    if (osAccountInfo->BundleExist(bundleUid)) {
        // insert new ProcessInfo and update new priority
        bundleInfo =  osAccountInfo->FindBundleInfoById(bundleUid);
    } else {
        // need to new BundleInfo ,add to list and map
        bundleInfo = new BundlePriorityInfo(bundleName, bundleUid, RECLAIM_PRIORITY_FOREGROUND);
        AddBundleInfoToSet(bundleInfo);
    }
    ProcessPriorityInfo *processInfo;
    if (IsSystemApp(*bundleInfo)) {
        processInfo = new ProcessPriorityInfo(pid, bundleUid, RECLAIM_PRIORITY_SYSTEM);
    } else {
        processInfo = new ProcessPriorityInfo(pid, bundleUid, RECLAIM_PRIORITY_FOREGROUND);
    }
    bundleInfo->AddProcess(*processInfo);
    UpdateBundlePriority(bundleInfo);
    osAccountInfo->AddBundleToUser(bundleInfo);
    bool ret = ApplyReclaimPriority(bundleInfo, *processInfo);
    HILOGI("create: bundleName=%{public}s, prio=%{public}d", bundleName.c_str(), bundleInfo->priority_);
    return ret;
}

void ReclaimPriorityManager::HandleTerminateProcess(ProcessPriorityInfo &processInfo,
    BundlePriorityInfo *bundleInfo, OsAccountPriorityInfo *osAccountInfo)
{
    // clear processInfo and bundleInfo if needed, delete the object
    HILOGI("terminated: bundleName=%{public}s, pid=%{public}d", bundleInfo->name_.c_str(), processInfo.pid_);
    int removedProcessPrio = processInfo.priority_;
    bundleInfo->RemoveProcessById(processInfo.pid_);

    if (bundleInfo->GetProcessCount() == 0) {
        osAccountInfo->RemoveBundleById(bundleInfo->uid_);
        DeleteBundleInfoFromSet(bundleInfo);
        delete bundleInfo;
        bundleInfo = nullptr;
    } else {
        if (removedProcessPrio <= bundleInfo->priority_) {
            UpdateBundlePriority(bundleInfo);
        }
    }
    if (osAccountInfo->BundleCount() == 0) {
        RemoveUserInfoById(osAccountInfo->id_);
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

    if (!IsProcessExist(pid, bundleUid, accountId)) {
        HILOGE("process not exist and not to create it!!");
        return false;
    }

    OsAccountPriorityInfo* osAccountInfo = FindUserInfoById(accountId);
    BundlePriorityInfo *bundleInfo =  osAccountInfo->FindBundleInfoById(bundleUid);
    if (bundleInfo->priority_ == RECLAIM_PRIORITY_SYSTEM) {
        HILOGI("%{public}s is system app, skip!", bundleName.c_str());
        return true;
    }

    ProcessPriorityInfo &processInfo = bundleInfo->FindProcessInfoById(pid);

    bool ret = true;
    switch (reason) {
        case AppStateUpdateReason::CREATE_PROCESS: {
            HILOGE("not supposed to reach here!");
            return false;
        }
        case AppStateUpdateReason::PROCESS_TERMINATED: {
            HandleTerminateProcess(processInfo, bundleInfo, osAccountInfo);
            break;
        }
        case AppStateUpdateReason::FOREGROUND: {
            processInfo.SetPriority(RECLAIM_PRIORITY_FOREGROUND);
            UpdateBundlePriority(bundleInfo);
            break;
        }
        case AppStateUpdateReason::BACKGROUND: {
            processInfo.SetPriority(RECLAIM_PRIORITY_BACKGROUND);
            UpdateBundlePriority(bundleInfo);
            break;
        }
        case AppStateUpdateReason::PROCESS_SUSPEND: {
            processInfo.SetPriority(RECLAIM_PRIORITY_SUSPEND);
            UpdateBundlePriority(bundleInfo);
            break;
        }
        case AppStateUpdateReason::SUSPEND_DELAY_START:
            processInfo.isSuspendDelay = true;
            if (bundleInfo->priority_ > RECLAIM_PRIORITY_BG_SUSPEND_DELAY) {
                processInfo.SetPriority(RECLAIM_PRIORITY_BG_SUSPEND_DELAY);
                UpdateBundlePriority(bundleInfo);
            }
            break;
        case AppStateUpdateReason::SUSPEND_DELAY_END:
            processInfo.isSuspendDelay = false;
            if (bundleInfo->priority_ == RECLAIM_PRIORITY_BG_SUSPEND_DELAY) {
                processInfo.SetPriority(RECLAIM_PRIORITY_BACKGROUND);
                UpdateBundlePriority(bundleInfo);
            }
            break;
        case AppStateUpdateReason::BACKGROUND_RUNNING_START:
            processInfo.isBackgroundRunning = true;
            break;
        case AppStateUpdateReason::BACKGROUND_RUNNING_END:
            processInfo.isBackgroundRunning = false;
            break;
        case AppStateUpdateReason::EVENT_START:
            processInfo.isEventStart = true;
            break;
        case AppStateUpdateReason::EVENT_END:
            processInfo.isEventStart = false;
            break;
        case AppStateUpdateReason::DATA_ABILITY_START:
            processInfo.isDataAbilityStart = true;
            break;
        case AppStateUpdateReason::DATA_ABILITY_END:
            processInfo.isDataAbilityStart = false;
            break;
        default:
            break;
    }
    // if priority of the process or the bundle is smaller than RECLAIM_PRIORITY_BACKGROUND, it need not to update
    if (processInfo.isBackgroundRunning || processInfo.isEventStart || processInfo.isDataAbilityStart) {
        if (bundleInfo->priority_ > RECLAIM_PRIORITY_BG_PERCEIVED) {
            processInfo.SetPriority(RECLAIM_PRIORITY_BG_PERCEIVED);
            UpdateBundlePriority(bundleInfo);
        }
    } else if (bundleInfo->priority_ == RECLAIM_PRIORITY_BG_PERCEIVED) {
        processInfo.SetPriority(RECLAIM_PRIORITY_BACKGROUND);
        UpdateBundlePriority(bundleInfo);
    }
    ret = ApplyReclaimPriority(bundleInfo, processInfo);
    return ret;
}

bool ReclaimPriorityManager::ApplyReclaimPriority(BundlePriorityInfo *bundleInfo, ProcessPriorityInfo &processInfo)
{
    return WriteOomScoreAdjToKernel(bundleInfo);
}

bool ReclaimPriorityManager::CurrentOsAccountChanged(int curAccountId)
{
    if (!initialized_) {
        HILOGE("has not been initialized_, skiped!");
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
                OsAccountPriorityInfo* curUser = FindUserInfoById(curOsAccountId_);
                curUser->PromoteAllBundlePriority(curUser->priorityShift_);
                OsAccountPriorityInfo* preUser = FindUserInfoById(preOsAccountId_);
                preUser->ReduceAllBundlePriority(curUser->priorityShift_);
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
