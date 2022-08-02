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

#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "multi_account_manager.h"
#include "kernel_interface.h"
#include "oom_score_adj_utils.h"
#include "reclaim_strategy_manager.h"
#include "singleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "bundle_mgr_proxy.h"
#include "reclaim_priority_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "ReclaimPriorityManager";
}
IMPLEMENT_SINGLE_INSTANCE(ReclaimPriorityManager);

ReclaimPriorityManager::ReclaimPriorityManager()
{
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::CREATE_PROCESS)] = "CREATE_PROCESS";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::PROCESS_READY)] = "PROCESS_READY";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::FOREGROUND)] = "FOREGROUND";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::BACKGROUND)] = "BACKGROUND";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::SUSPEND_DELAY_START)] = "SUSPEND_DELAY_START";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::SUSPEND_DELAY_END)] = "SUSPEND_DELAY_END";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::BACKGROUND_RUNNING_START)] =
        "BACKGROUND_RUNNING_START";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::BACKGROUND_RUNNING_END)] =
        "BACKGROUND_RUNNING_END";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::EVENT_START)] = "EVENT_START";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::EVENT_END)] = "EVENT_END";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::DATA_ABILITY_START)] = "DATA_ABILITY_START";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::DATA_ABILITY_END)] = "DATA_ABILITY_END";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::APPLICATION_SUSPEND)] = "APPLICATION_SUSPEND";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::PROCESS_TERMINATED)] = "PROCESS_TERMINATED";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::OS_ACCOUNT_CHANGED)] = "OS_ACCOUNT_CHANGED";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::DIST_DEVICE_CONNECTED)] =
        "DIST_DEVICE_CONNECTED";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::DIST_DEVICE_DISCONNECTED)] =
        "DIST_DEVICE_DISCONNECTED";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::FOREGROUND_BIND_EXTENSION)] =
        "FOREGROUND_BIND_EXTENSION";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::BACKGROUND_BIND_EXTENSION)] =
        "BACKGROUND_BIND_EXTENSION";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::NO_BIND_EXTENSION)] = "NO_BIND_EXTENSION";
}

std::string& ReclaimPriorityManager::AppStateUpdateResonToString(AppStateUpdateReason reason)
{
    auto ptr = updateReasonStrMapping_.find(static_cast<int32_t>(reason));
    if (ptr != updateReasonStrMapping_.end()) {
        return ptr->second;
    } else {
        return unkown_reason;
    }
}

bool ReclaimPriorityManager::Init()
{
    config_ = MemmgrConfigManager::GetInstance().GetReclaimPriorityConfig();
    initialized_ = GetEventHandler();
    GetAllKillableSystemApps();
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
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

void ReclaimPriorityManager::GetAllKillableSystemApps()
{
    HILOGI("called");
    // get killable system apps from xml
    allKillableSystemApps_.merge(config_.killalbeSystemApps_);
    // get killable system apps from fwk (finally from bms)
    std::set<std::string> killableSystemAppsFromAms_;
    GetKillableSystemAppsFromAms(killableSystemAppsFromAms_);
    allKillableSystemApps_.merge(killableSystemAppsFromAms_);
}

void ReclaimPriorityManager::GetKillableSystemAppsFromAms(std::set<std::string> &killableApps)
{
    // get killable system apps from fwk (finally from bms)
}

// if user install new killable system apps, fwk should tell me by calling this interface.
// if user uninstall some killable system apps, we can do nothing since killable info will be updated on next rebooting.
void ReclaimPriorityManager::NotifyKillableSystemAppsAdded(std::set<std::string> &newKillableApps)
{
    allKillableSystemApps_.merge(newKillableApps);
}

// handle process started before our service
void ReclaimPriorityManager::HandlePreStartedProcs()
{
    std::vector<unsigned int> preStartedPids;
    KernelInterface::GetInstance().GetAllProcPids(preStartedPids);
    for (unsigned int pid : preStartedPids) {
        unsigned int uid = 0;
        if (!KernelInterface::GetInstance().GetUidByPid(pid, uid)) {
            HILOGE("process[pid=%{public}d] started before me, but GetUidByPid failed.", pid);
            continue;
        }
        struct ProcInfo procInfo;
        std::string name;
        if (!KernelInterface::GetInstance().GetProcNameByPid(pid, name)) {
            HILOGE("process[pid=%{public}d, uid=%{public}d] started before me, but GetProcNameByPid failed.", pid, uid);
            continue;
        }
        bool killable = false;;
        if (allKillableSystemApps_.find(name) != allKillableSystemApps_.end()) {
            killable = true;
            OomScoreAdjUtils::WriteOomScoreAdjToKernel(pid, RECLAIM_PRIORITY_KILLABLE_SYSTEM);
            HILOGI("process[pid=%{public}d, uid=%{public}d, name=%{public}s] started before me, killable = %{public}d",
                pid, uid, name.c_str(), killable);
        }
    }
}

void ReclaimPriorityManager::GetBundlePrioSet(BunldeCopySet &bundleSet)
{
    // add lock
    std::lock_guard<std::mutex> setLock(totalBundlePrioSetLock_);

    HILOGD("iter %{public}zu bundles begin", totalBundlePrioSet_.size());
    int count = 0;
    for (auto itrBundle = totalBundlePrioSet_.rbegin(); itrBundle != totalBundlePrioSet_.rend(); ++itrBundle, ++count) {
        std::shared_ptr<BundlePriorityInfo> bundle = *itrBundle;
        if (bundle == nullptr) {
            HILOGD("bundle %{public}d/%{public}zu is nullptr", count, totalBundlePrioSet_.size());
            continue;
        }

        HILOGD("bundle %{public}d/%{public}zu, uid=%{publics}d", count, totalBundlePrioSet_.size(), bundle->uid_);
        BundlePriorityInfo tmpBundleInfo(bundle->name_, bundle->uid_, bundle->priority_,
                                         bundle->accountId_, bundle->state_);

        for (auto itrProcess = bundle->procs_.begin(); itrProcess != bundle->procs_.end(); itrProcess++) {
            ProcessPriorityInfo processInfo = itrProcess->second;
            ProcessPriorityInfo tmpProcess(processInfo);

            tmpBundleInfo.procs_.insert(std::make_pair(tmpProcess.pid_, tmpProcess));
        }

        HILOGD("insert bundle [%{public}d][%{public}s] to set, priority=%{public}d",
               tmpBundleInfo.uid_, tmpBundleInfo.name_.c_str(), tmpBundleInfo.priority_);
        bundleSet.insert(tmpBundleInfo);
    }
    HILOGD("iter bundles end");
}

void ReclaimPriorityManager::GetOneKillableBundle(int minPrio, BunldeCopySet &bundleSet)
{
    HILOGD("called, minPrio=%{public}d", minPrio);
    // add lock
    std::lock_guard<std::mutex> setLock(totalBundlePrioSetLock_);

    HILOGD("iter %{public}zu bundles begin", totalBundlePrioSet_.size());
    int count = 0;
    for (auto itrBundle = totalBundlePrioSet_.rbegin(); itrBundle != totalBundlePrioSet_.rend(); ++itrBundle) {
        std::shared_ptr<BundlePriorityInfo> bundle = *itrBundle;
        if (bundle == nullptr) {
            HILOGD("bundle %{public}d/%{public}zu is nullptr", count, totalBundlePrioSet_.size());
            continue;
        }
        if (bundle->priority_ < minPrio) {
            HILOGD("there is no bundle with priority bigger than %{public}d, break!", minPrio);
            break;
        }
        if (bundle->GetState() == BundleState::STATE_WAITING_FOR_KILL) {
            HILOGD("bundle<%{public}d, %{public}s}> is waiting to kill, skiped!", bundle->uid_, bundle->name_.c_str());
            continue;
        }

        try {
            auto ret = bundleSet.insert(*bundle);
            if (ret.second) {
                HILOGI("insert bundle<%{public}d, %{public}s}> to set, priority=%{public}d", bundle->uid_,
                    bundle->name_.c_str(), bundle->priority_);
                ++count;
                break;
            }
        } catch (...) {
            HILOGE("new BundlePriorityInfo failed, need kill quickly!");
            for (auto procEntry : bundle->procs_) {
                HILOGE("quick killing bundle<%{public}d, %{public}s}>, pid=%{public}d", bundle->uid_,
                    bundle->name_.c_str(), procEntry.second.pid_);
                KernelInterface::GetInstance().KillOneProcessByPid(procEntry.second.pid_);
            }
        }
    }
    HILOGD("iter bundles end");
}

void ReclaimPriorityManager::SetBundleState(int accountId, int uid, BundleState state)
{
    std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(accountId);
    if (account != nullptr) {
        auto pairPtr = account->bundleIdInfoMapping_.find(uid);
        if (pairPtr != account->bundleIdInfoMapping_.end()) {
            if (pairPtr->second != nullptr) {
                auto bundlePtr = pairPtr->second;
                bundlePtr->SetState(state);
            }
        }
    }
}

bool ReclaimPriorityManager::IsOsAccountExist(int accountId)
{
    if (osAccountsInfoMap_.find(accountId) == osAccountsInfoMap_.end()) {
        HILOGE("accountId not exist");
        return false;
    }
    return true;
}

void ReclaimPriorityManager::AddBundleInfoToSet(std::shared_ptr<BundlePriorityInfo> bundle)
{
    std::lock_guard<std::mutex> lock(totalBundlePrioSetLock_);
    auto ret = totalBundlePrioSet_.insert(bundle);
    if (ret.second) {
        HILOGD("success to insert bundle to set, uid=%{public}d, totalBundlePrioSet_.size=%{public}zu",
            bundle->uid_, totalBundlePrioSet_.size());
    }
}

void ReclaimPriorityManager::DeleteBundleInfoFromSet(std::shared_ptr<BundlePriorityInfo> bundle)
{
    std::lock_guard<std::mutex> lock(totalBundlePrioSetLock_);
    int delCount = totalBundlePrioSet_.erase(bundle);
    HILOGD("delete %{public}d bundles from set, uid=%{public}d, totalBundlePrioSet_.size=%{public}zu",
           delCount, bundle->uid_, totalBundlePrioSet_.size());
}

std::shared_ptr<AccountBundleInfo> ReclaimPriorityManager::FindOsAccountById(int accountId)
{
    auto iter = osAccountsInfoMap_.find(accountId);
    if (iter != osAccountsInfoMap_.end()) {
        return iter->second;
    }
    HILOGI("not found the account info");
    return nullptr;
}

void ReclaimPriorityManager::RemoveOsAccountById(int accountId)
{
    // erase the accountId data
    osAccountsInfoMap_.erase(accountId);
}

void ReclaimPriorityManager::AddOsAccountInfo(std::shared_ptr<AccountBundleInfo> account)
{
    osAccountsInfoMap_.insert(std::make_pair(account->id_, account));
}

bool ReclaimPriorityManager::IsProcExist(pid_t pid, int bundleUid, int accountId)
{
    if (pid == IGNORE_PID) {
        return true;
    }
    std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(accountId);
    if (account == nullptr || !account->HasBundle(bundleUid)) {
        HILOGE("account or bundle name not exist");
        return false;
    }
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(bundleUid);
    if (!bundle->HasProc(pid)) {
        HILOGE("pid not exist");
        return false;
    }
    return true;
}

bool ReclaimPriorityManager::UpdateReclaimPriority(pid_t pid, int bundleUid, const std::string &name,
    AppStateUpdateReason reason)
{
    if (!initialized_) {
        HILOGE("has not been initialized_, skiped!");
        return false;
    }
    std::function<bool()> updateReclaimPriorityInnerFunc =
        std::bind(&ReclaimPriorityManager::UpdateReclaimPriorityInner, this, pid, bundleUid, name, reason);
    return handler_->PostImmediateTask(updateReclaimPriorityInnerFunc);
}

sptr<AppExecFwk::IBundleMgr> GetBundleMgr()
{
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        HILOGE("failed to get system ability manager!");
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject_ = saMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject_ == nullptr) {
        HILOGE("bms not found!");
        return nullptr;
    }
    sptr<AppExecFwk::IBundleMgr> bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(remoteObject_);
    if (bundleMgr == nullptr) {
        HILOGE("bms interface cast failed!");
    }
    return bundleMgr;
}

bool ReclaimPriorityManager::IsKillableSystemApp(std::shared_ptr<BundlePriorityInfo> bundle)
{
    if (allKillableSystemApps_.find(bundle->name_) != allKillableSystemApps_.end()) {
        HILOGI("find bundle (%{public}s) in killable system app list", bundle->name_.c_str());
        return true;
    }

    sptr<AppExecFwk::IBundleMgr> bmsPtr = GetBundleMgr();
    if (bmsPtr == nullptr) {
        HILOGE("failed to get BundleMgr!");
        return false;
    }

    AppExecFwk::ApplicationInfo info;
    bool result = bmsPtr->GetApplicationInfo(bundle->name_.c_str(),
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, GetOsAccountLocalIdFromUid(bundle->uid_), info);
    if (result) {
        HILOGI("appInfo<%{public}s,%{public}d><keepAlive=%{public}d, isSystemApp=%{public}d, isLauncherApp=%{public}d>",
            bundle->name_.c_str(), bundle->uid_, info.keepAlive, info.isSystemApp, info.isLauncherApp);
        if (info.keepAlive) {
            auto ret = allKillableSystemApps_.insert(bundle->name_);
            if (ret.second) {
                HILOGI("add a new killable system app (%{public}s)", bundle->name_.c_str());
            }
        }
        return info.keepAlive;
    } else {
        HILOGE("bundleMgr GetApplicationInfo failed!");
    }
    return false;
}

void ReclaimPriorityManager::UpdateBundlePriority(std::shared_ptr<BundlePriorityInfo> bundle)
{
    HILOGD("begin-------------------------");
    DeleteBundleInfoFromSet(bundle);
    bundle->UpdatePriority();
    AddBundleInfoToSet(bundle);
    HILOGD("end----------------------------");
}

bool ReclaimPriorityManager::HandleCreateProcess(pid_t pid, int bundleUid, const std::string &bundleName, int accountId)
{
    HILOGD("called, bundleName=%{public}s, pid=%{public}d", bundleName.c_str(), pid);
    std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(accountId);
    if (account == nullptr) {
        DECLARE_SHARED_POINTER(AccountBundleInfo, tmpAccount);
        MAKE_POINTER(tmpAccount, shared, AccountBundleInfo, "cannot new account!!", return false, accountId);
        account = tmpAccount;
        AddOsAccountInfo(account);
    }
    std::shared_ptr<BundlePriorityInfo> bundle;
    AppAction action;
    if (account->HasBundle(bundleUid)) {
        // insert new ProcessInfo and update new priority
        bundle = account->FindBundleById(bundleUid);
        action = AppAction::CREATE_PROCESS_ONLY;
    } else {
        // need to new BundleInfo ,add to list and map
        MAKE_POINTER(bundle, shared, BundlePriorityInfo, "cannot new account!!", return false,
            bundleName, bundleUid, RECLAIM_PRIORITY_FOREGROUND);
        AddBundleInfoToSet(bundle);
        action = AppAction::CREATE_PROCESS_AND_APP;
    }
    ProcessPriorityInfo proc(pid, bundleUid, RECLAIM_PRIORITY_FOREGROUND);
    if (IsKillableSystemApp(bundle)) {
        HILOGI("[bundleName=%{public}s, pid=%{public}d] is a killable system app", bundleName.c_str(), pid);
        proc.priority_ = RECLAIM_PRIORITY_KILLABLE_SYSTEM;
    } else {
        HILOGI("[bundleName=%{public}s, pid=%{public}d] is not a killable system app", bundleName.c_str(), pid);
    }
    bundle->AddProc(proc);
    UpdateBundlePriority(bundle);
    account->AddBundleToOsAccount(bundle);
    bool ret = ApplyReclaimPriority(bundle, pid, action);
    HILOGI("create: bundleName=%{public}s, prio=%{public}d", bundleName.c_str(), bundle->priority_);
    return ret;
}

bool ReclaimPriorityManager::HandleTerminateProcess(ProcessPriorityInfo proc,
    std::shared_ptr<BundlePriorityInfo> bundle, std::shared_ptr<AccountBundleInfo> account)
{
    // clear proc and bundle if needed, delete the object
    HILOGI("terminated: bundleName=%{public}s, pid=%{public}d", bundle->name_.c_str(), proc.pid_);
    int removedProcessPrio = proc.priority_;
    bundle->RemoveProcByPid(proc.pid_);
    bool ret = true;

    if (bundle->GetProcsCount() == 0) {
        ret = ApplyReclaimPriority(bundle, proc.pid_, AppAction::APP_DIED);
        account->RemoveBundleById(bundle->uid_);
        DeleteBundleInfoFromSet(bundle);
    } else {
        if (removedProcessPrio <= bundle->priority_) {
            UpdateBundlePriority(bundle);
        }
    }
    if (account->GetBundlesCount() == 0) {
        RemoveOsAccountById(account->id_);
    }
    return ret;
}

bool ReclaimPriorityManager::HandleApplicationSuspend(std::shared_ptr<BundlePriorityInfo> bundle)
{
    if (bundle == nullptr) {
        return false;
    }
    HILOGI("application suspend: bundleName=%{public}s", bundle->name_.c_str());
    for (auto i = bundle->procs_.begin(); i != bundle->procs_.end(); ++i) {
        i->second.priority_ = RECLAIM_PRIORITY_SUSPEND;
    }
    UpdateBundlePriority(bundle);
    bool ret = ApplyReclaimPriority(bundle, IGNORE_PID, AppAction::OTHERS);
    return ret;
}

bool ReclaimPriorityManager::UpdateReclaimPriorityInner(pid_t pid, int bundleUid, const std::string &bundleName,
    AppStateUpdateReason reason)
{
    HILOGI("called, pid=%{public}d, bundleUid=%{public}d, bundleName=%{public}s, reason=%{public}s",
        pid, bundleUid, bundleName.c_str(), AppStateUpdateResonToString(reason).c_str());
    int accountId = GetOsAccountLocalIdFromUid(bundleUid);
    HILOGD("accountId=%{public}d", accountId);

    if (reason == AppStateUpdateReason::CREATE_PROCESS) {
        HILOGD("call HandleCreateProcess");
        bool ret = HandleCreateProcess(pid, bundleUid, bundleName, accountId);
        return ret;
    }

    if (!IsProcExist(pid, bundleUid, accountId)) {
        HILOGE("process not exist and not to create it!!");
        return false;
    }
    std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(bundleUid);
    if (bundle->priority_ == RECLAIM_PRIORITY_SYSTEM) {
        HILOGI("%{public}s is system app, skip!", bundleName.c_str());
        return true;
    }

    if (reason == AppStateUpdateReason::APPLICATION_SUSPEND) {
        HILOGD("call HandleApplicationSuspend");
        bool ret = HandleApplicationSuspend(bundle);
        return ret;
    }

    ProcessPriorityInfo &proc = bundle->FindProcByPid(pid);
    bool ret = true;
    AppAction action = AppAction::OTHERS;
    if (reason == AppStateUpdateReason::PROCESS_TERMINATED) {
        HILOGD("call HandleTerminateProcess");
        ret = HandleTerminateProcess(proc, bundle, account);
        return ret;
    } else {
        HILOGD("call HandleUpdateProcess");
        HandleUpdateProcess(reason, bundle, proc, action);
    }
    ret = ApplyReclaimPriority(bundle, pid, action);
    return ret;
}

void ReclaimPriorityManager::UpdatePriorityByProcStatus(std::shared_ptr<BundlePriorityInfo> bundle,
                                                        ProcessPriorityInfo &proc)
{
    int priorityBefore = proc.priority_;

    if (bundle->priority_ < RECLAIM_PRIORITY_FOREGROUND) { // is a system process
        HILOGD("%{public}d is a system process, skiped!", proc.pid_);
        return;
    }
    if (proc.isFreground) { // is a freground process
        HILOGD("%{public}d is a freground process", proc.pid_);
        if (proc.priority_ > RECLAIM_PRIORITY_FOREGROUND) {
            proc.SetPriority(RECLAIM_PRIORITY_FOREGROUND);
        }
    } else { // is a background process
        HILOGD("%{public}d is a background process, set process priority to %{public}d first", proc.pid_,
            RECLAIM_PRIORITY_BACKGROUND);
        proc.SetPriority(RECLAIM_PRIORITY_BACKGROUND);
    }
    if (proc.isSuspendDelay) { // is a background process with transient task
        HILOGD("%{public}d is a background process with transient task", proc.pid_);
        if (proc.priority_ > RECLAIM_PRIORITY_BG_SUSPEND_DELAY) {
            proc.SetPriority(RECLAIM_PRIORITY_BG_SUSPEND_DELAY);
        }
    } else if (proc.extensionBindStatus == EXTENSION_STATUS_FG_BIND) { // is a extension and bind to a fg process
        HILOGD("%{public}d is a extension process bind to a fg process", proc.pid_);
        if (proc.priority_ > RECLAIM_PRIORITY_FG_BIND_EXTENSION) {
            proc.SetPriority(RECLAIM_PRIORITY_FG_BIND_EXTENSION);
        }
    } else if (proc.isBackgroundRunning || proc.isEventStart || proc.isDataAbilityStart) {
        // is a background perceived process
        HILOGD("%{public}d is a background perceived process", proc.pid_);
        if (proc.priority_ > RECLAIM_PRIORITY_BG_PERCEIVED) {
            proc.SetPriority(RECLAIM_PRIORITY_BG_PERCEIVED);
        }
    } else if (proc.extensionBindStatus == EXTENSION_STATUS_BG_BIND) { // is a extension and bind to a bg process
        HILOGD("%{public}d is a extension process bind to a bg process", proc.pid_);
        if (proc.priority_ > RECLAIM_PRIORITY_BG_BIND_EXTENSION) {
            proc.SetPriority(RECLAIM_PRIORITY_BG_BIND_EXTENSION);
        }
    } else if (proc.isDistDeviceConnected) { // is a background process connected by distribute device
        HILOGD("%{public}d is a process connect to a dist device", proc.pid_);
        if (proc.priority_ > RECLAIM_PRIORITY_BG_DIST_DEVICE) {
            proc.SetPriority(RECLAIM_PRIORITY_BG_DIST_DEVICE);
        }
    } else if (proc.extensionBindStatus == EXTENSION_STATUS_NO_BIND) { // is a extension and no bind to any process
        HILOGD("%{public}d is a extension process no bind to any process", proc.pid_);
        if (proc.priority_ > RECLAIM_PRIORITY_NO_BIND_EXTENSION) {
            proc.SetPriority(RECLAIM_PRIORITY_NO_BIND_EXTENSION);
        }
    } else {
        // is a plain background process
    }
    HILOGI(": bundle[uid_=%{public}d,name=%{public}s,priority=%{public}d], proc[pid_=%{public}d, uid=%{public}d, "
        "isFreground=%{public}d, isBackgroundRunning=%{public}d, isSuspendDelay=%{public}d, isEventStart=%{public}d, "
        "isDataAbilityStart=%{public}d, isDistDeviceConnected=%{public}d, extensionBindStatus=%{public}d], "
        "priority:%{public}d-->%{public}d",
        bundle->uid_, bundle->name_.c_str(), bundle->priority_,
        proc.pid_, proc.uid_, proc.isFreground, proc.isBackgroundRunning, proc.isSuspendDelay, proc.isEventStart,
        proc.isDataAbilityStart, proc.isDistDeviceConnected, proc.extensionBindStatus, priorityBefore, proc.priority_);
    UpdateBundlePriority(bundle);
}

void ReclaimPriorityManager::HandleUpdateProcess(AppStateUpdateReason reason,
    std::shared_ptr<BundlePriorityInfo> bundle, ProcessPriorityInfo &proc, AppAction &action)
{
    HILOGD("called, bundle[uid_=%{public}d,name=%{public}s,priority=%{public}d], proc[pid_=%{public}d, uid=%{public}d,"
        "isFreground=%{public}d, isBackgroundRunning=%{public}d, isSuspendDelay=%{public}d, isEventStart=%{public}d,"
        "isDataAbilityStart=%{public}d, isDistDeviceConnected=%{public}d, extensionBindStatus=%{public}d, "
        "priority=%{public}d], case:%{public}s",
        bundle->uid_, bundle->name_.c_str(), bundle->priority_, proc.pid_, proc.uid_, proc.isFreground,
        proc.isBackgroundRunning, proc.isSuspendDelay, proc.isEventStart, proc.isDataAbilityStart,
        proc.isDistDeviceConnected, proc.extensionBindStatus, proc.priority_,
        AppStateUpdateResonToString(reason).c_str());
    switch (reason) {
        case AppStateUpdateReason::FOREGROUND: {
            proc.isFreground = true;
            action = AppAction::APP_FOREGROUND;
            break;
        }
        case AppStateUpdateReason::BACKGROUND: {
            proc.isFreground = false;
            action = AppAction::APP_BACKGROUND;
            break;
        }
        case AppStateUpdateReason::SUSPEND_DELAY_START:
            proc.isSuspendDelay = true;
            break;
        case AppStateUpdateReason::SUSPEND_DELAY_END:
            proc.isSuspendDelay = false;
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
        case AppStateUpdateReason::DIST_DEVICE_CONNECTED:
            proc.isDistDeviceConnected = true;
            break;
        case AppStateUpdateReason::DIST_DEVICE_DISCONNECTED:
            proc.isDistDeviceConnected = false;
            break;
        case AppStateUpdateReason::FOREGROUND_BIND_EXTENSION:
            proc.extensionBindStatus = EXTENSION_STATUS_FG_BIND;
            break;
        case AppStateUpdateReason::BACKGROUND_BIND_EXTENSION:
            proc.extensionBindStatus = EXTENSION_STATUS_BG_BIND;
            break;
        case AppStateUpdateReason::NO_BIND_EXTENSION:
            proc.extensionBindStatus = EXTENSION_STATUS_NO_BIND;
            break;
        default:
            break;
    }
    UpdatePriorityByProcStatus(bundle, proc);
}

bool ReclaimPriorityManager::ApplyReclaimPriority(std::shared_ptr<BundlePriorityInfo> bundle,
    pid_t pid, AppAction action)
{
    HILOGD("called");
    if (bundle == nullptr) {
        return false;
    }
    DECLARE_SHARED_POINTER(ReclaimParam, para);
    MAKE_POINTER(para, shared, ReclaimParam, "make ReclaimParam failed", return false,
        pid, bundle->uid_, bundle->name_, bundle->accountId_, bundle->priority_, action);
    ReclaimStrategyManager::GetInstance().NotifyAppStateChanged(para);
    return OomScoreAdjUtils::WriteOomScoreAdjToKernel(bundle);
}

bool ReclaimPriorityManager::OsAccountChanged(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod)
{
    if (!initialized_) {
        HILOGE("has not been initialized_, skiped!");
        return false;
    }
    if (accountId < 0) {
        HILOGE("invalid account id!");
        return false;
    }
    std::function<bool()> osAccountChangedInnerFunc =
        std::bind(&ReclaimPriorityManager::OsAccountChangedInner, this, accountId, switchMod);
    return handler_->PostImmediateTask(osAccountChangedInnerFunc);
}

bool ReclaimPriorityManager::OsAccountChangedInner(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod)
{
    return UpdateAllPrioForOsAccountChanged(accountId, switchMod);
}

bool ReclaimPriorityManager::UpdateAllPrioForOsAccountChanged(int accountId,
    AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod)
{
    if (!initialized_) {
        HILOGE("has not been initialized_, skiped!");
        return false;
    }
    HILOGI("UpdateReclaimPriority for all apps because of os account changed ");
    bool ret = MultiAccountManager::GetInstance().HandleOsAccountsChanged(accountId, switchMod, osAccountsInfoMap_);
    return ret;
}
} // namespace Memory
} // namespace OHOS
