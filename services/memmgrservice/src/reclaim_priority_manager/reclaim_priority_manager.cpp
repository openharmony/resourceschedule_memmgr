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

#include "reclaim_priority_manager.h"

#include "app_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "iservice_registry.h"
#include "kernel_interface.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "multi_account_manager.h"
#include "oom_score_adj_utils.h"
#include "reclaim_strategy_manager.h"
#include "render_process_info.h"
#include "singleton.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "ReclaimPriorityManager";
constexpr int TIMER_DIED_PROC_FAST_CHECK_MS = 10000;
constexpr int TIMER_DIED_PROC_SLOW_CHECK_MS = 3 * 60 * 1000; // 3min
constexpr int MAX_TOTALBUNDLESET_SIZE = 2000;
}
IMPLEMENT_SINGLE_INSTANCE(ReclaimPriorityManager);

ReclaimPriorityManager::ReclaimPriorityManager()
{
    InitUpdateReasonStrMapping();
    InitChangeProcMapping();
}

void ReclaimPriorityManager::InitUpdateReasonStrMapping()
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
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::APPLICATION_SUSPEND)] = "APPLICATION_SUSPEND";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::PROCESS_TERMINATED)] = "PROCESS_TERMINATED";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::OS_ACCOUNT_CHANGED)] = "OS_ACCOUNT_CHANGED";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::DIST_DEVICE_CONNECTED)] =
        "DIST_DEVICE_CONNECTED";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::DIST_DEVICE_DISCONNECTED)] =
        "DIST_DEVICE_DISCONNECTED";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::BIND_EXTENSION)] = "BIND_EXTENSION";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::UNBIND_EXTENSION)] = "UNBIND_EXTENSION";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::RENDER_CREATE_PROCESS)] =
        "RENDER_CREATE_PROCESS";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::VISIBLE)] = "VISIBLE";
    updateReasonStrMapping_[static_cast<int32_t>(AppStateUpdateReason::UN_VISIBLE)] = "UN_VISIBLE";
}

std::string& ReclaimPriorityManager::AppStateUpdateResonToString(AppStateUpdateReason reason)
{
    auto ptr = updateReasonStrMapping_.find(static_cast<int32_t>(reason));
    if (ptr != updateReasonStrMapping_.end()) {
        return ptr->second;
    } else {
        return UNKOWN_REASON;
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
    SetTimerForDiedProcessCheck(TIMER_DIED_PROC_SLOW_CHECK_MS);
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

void ReclaimPriorityManager::Dump(int fd)
{
    // add lock
    std::lock_guard<std::mutex> setLock(totalBundlePrioSetLock_);

    dprintf(fd, "priority list of all managed apps\n");
    dprintf(fd, "     uid                                            name   priority\n");
    for (auto bundlePtr : totalBundlePrioSet_) {
        if (bundlePtr == nullptr) {
            dprintf(fd, "bundlePtr is nullptr.\n");
            continue;
        }
        dprintf(fd, "%8d %42s %5d %3d\n", bundlePtr->uid_, bundlePtr->name_.c_str(), bundlePtr->priority_,
            bundlePtr->GetProcsCount());
    }
    dprintf(fd, "-----------------------------------------------------------------\n\n");

    dprintf(fd,
        "priority list of all managed processes, status:(fg,visible,bgtask,trantask,evt,dist,extb,ext,render)\n");
    dprintf(fd, "    pid       uid                                   bundle priority status\
                      connnectorpids               connnectoruids               processuids\n");
    for (auto bundlePtr : totalBundlePrioSet_) {
        dprintf(fd, "|-----------------------------------------\n");
        if (bundlePtr == nullptr) {
            dprintf(fd, "bundlePtr is nullptr.\n");
            continue;
        }
        for (auto procEntry : bundlePtr->procs_) {
            ProcessPriorityInfo &proc = procEntry.second;
            dprintf(fd, "|%8d %8d %42s %5d %d%d%d%d%d%d%d%d%d %30s\n",
                proc.pid_, bundlePtr->uid_, bundlePtr->name_.c_str(),
                proc.priority_, proc.isFreground, proc.isVisible_, proc.isBackgroundRunning,
                proc.isSuspendDelay, proc.isEventStart, proc.isDistDeviceConnected,
                proc.extensionBindStatus, proc.isExtension_, proc.isRender_, proc.ProcsBindToMe().c_str());
        }
    }
    dprintf(fd, "-----------------------------------------------------------------\n");
}

sptr<AppExecFwk::IAppMgr> GetAppMgrProxy()
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto appObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    return iface_cast<AppExecFwk::IAppMgr>(appObject);
}

bool ReclaimPriorityManager::IsFrontApp(const std::string& pkgName, int32_t uid, int32_t pid)
{
    sptr<AppExecFwk::IAppMgr> appMgrProxy_ = GetAppMgrProxy();
    if (!appMgrProxy_) {
        HILOGE("GetAppMgrProxy failed");
        return false;
    }
    std::vector<AppExecFwk::AppStateData> fgAppList;
    if (appMgrProxy_->GetForegroundApplications(fgAppList) != 0) {
        HILOGE("GetForegroundApplications failed");
        return false;
    }
    for (auto fgApp : fgAppList) {
        if (fgApp.bundleName == pkgName && fgApp.uid == uid) {
            return true;
        }
    }
    return false;
}

void ReclaimPriorityManager::GetAllKillableSystemApps()
{
    HILOGI("called");
    // get killable system apps from xml
    allKillableSystemApps_.merge(config_.GetkillalbeSystemApps());
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
        if (allKillableSystemApps_.find(name) != allKillableSystemApps_.end()) {
            OomScoreAdjUtils::WriteOomScoreAdjToKernel(pid, RECLAIM_PRIORITY_KILLABLE_SYSTEM);
            HILOGI("process[pid=%{public}d, uid=%{public}d, name=%{public}s] started before me, killable = %{public}d",
                pid, uid, name.c_str(), true);
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
    std::lock_guard<std::mutex> setLock(totalBundlePrioSetLock_);
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
    auto ret = totalBundlePrioSet_.insert(bundle);
    if (ret.second) {
        HILOGD("success to insert bundle to set, uid=%{public}d, totalBundlePrioSet_.size=%{public}zu",
            bundle->uid_, totalBundlePrioSet_.size());
    }
}

void ReclaimPriorityManager::DeleteBundleInfoFromSet(std::shared_ptr<BundlePriorityInfo> bundle)
{
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
    std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(accountId);
    if (account == nullptr || !account->HasBundle(bundleUid)) {
        HILOGE("account or bundle name not exist");
        return false;
    }
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(bundleUid);
    if (bundle == nullptr) {
        return false;
    }
    if (pid == IGNORE_PID) {
        return true;
    }
    if (!bundle->HasProc(pid)) {
        HILOGE("pid not exist");
        return false;
    }
    return true;
}

bool ReclaimPriorityManager::UpdateReclaimPriority(UpdateRequest request)
{
    if (!initialized_) {
        HILOGE("has not been initialized_, skiped!");
        return false;
    }
    std::function<bool()> updateReclaimPriorityInnerFunc =
                        std::bind(&ReclaimPriorityManager::UpdateReclaimPriorityInner, this, request);
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
        HILOGD("find bundle (%{public}s) in killable system app list", bundle->name_.c_str());
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
        HILOGD("appInfo<%{public}s,%{public}d><keepAlive=%{public}d, isSystemApp=%{public}d, isLauncherApp=%{public}d>",
            bundle->name_.c_str(), bundle->uid_, info.keepAlive, info.isSystemApp, info.isLauncherApp);
        if (info.keepAlive) {
            auto ret = allKillableSystemApps_.insert(bundle->name_);
            if (ret.second) {
                HILOGD("add a new killable system app (%{public}s)", bundle->name_.c_str());
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

bool ReclaimPriorityManager::HandleCreateProcess(ReqProc &target, int accountId, bool isRender)
{
    std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(accountId);
    if (isRender) {
        OomScoreAdjUtils::WriteOomScoreAdjToKernel(target.pid, RECLAIM_PRIORITY_FOREGROUND);
        return true;
    }
    if (account == nullptr) {
        DECLARE_SHARED_POINTER(AccountBundleInfo, tmpAccount);
        MAKE_POINTER(tmpAccount, shared, AccountBundleInfo, "cannot new account!!", return false, accountId);
        account = tmpAccount;
        AddOsAccountInfo(account);
    }
    std::shared_ptr<BundlePriorityInfo> bundle;
    AppAction action;
    if (account->HasBundle(target.uid)) {
        // insert new ProcessInfo and update new priority
        bundle = account->FindBundleById(target.uid);
        action = AppAction::CREATE_PROCESS_ONLY;
    } else {
        // need to new BundleInfo ,add to list and map
        MAKE_POINTER(bundle, shared, BundlePriorityInfo, "cannot new account!!", return false,
            target.bundleName, target.uid, RECLAIM_PRIORITY_BACKGROUND);
        AddBundleInfoToSet(bundle);
        action = AppAction::CREATE_PROCESS_AND_APP;
    }

    int priority = RECLAIM_PRIORITY_BACKGROUND;
    bool isImportantProc = IsImportantProc(target.processName, priority);
    ProcessPriorityInfo proc(target.pid, target.uid, priority, isImportantProc);
    if (IsKillableSystemApp(bundle) && proc.priority_ > RECLAIM_PRIORITY_KILLABLE_SYSTEM) {
        proc.priority_ = RECLAIM_PRIORITY_KILLABLE_SYSTEM;
    }
    bundle->AddProc(proc);
    UpdateBundlePriority(bundle);
    account->AddBundleToOsAccount(bundle);
    bool ret = ApplyReclaimPriority(bundle, target.pid, action);
    HILOGI("create: bundleName=%{public}s, prio=%{public}d", target.bundleName.c_str(), bundle->priority_);
    return ret;
}

bool ReclaimPriorityManager::HandleTerminateProcess(ProcessPriorityInfo proc,
    std::shared_ptr<BundlePriorityInfo> bundle, std::shared_ptr<AccountBundleInfo> account)
{
    HILOGI("terminated: bundleName=%{public}s, pid=%{public}d", bundle->name_.c_str(), proc.pid_);

    // clear proc and bundle if needed, delete the object
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

void ReclaimPriorityManager::SetTimerForDiedProcessCheck(int64_t delayTime)
{
    std::function<void()> timerFunc = std::bind(&ReclaimPriorityManager::HandleDiedProcessCheck, this);
    handler_->PostTask(timerFunc, delayTime, AppExecFwk::EventQueue::Priority::HIGH);
}

void ReclaimPriorityManager::FilterDiedProcess()
{
    std::vector<unsigned int> alivePids;
    if (!KernelInterface::GetInstance().GetAllProcPids(alivePids)) {
        return;
    }

    std::lock_guard<std::mutex> lock(totalBundlePrioSetLock_);
    for (auto itrBundle = totalBundlePrioSet_.begin(); itrBundle != totalBundlePrioSet_.end();) {
        std::shared_ptr<BundlePriorityInfo> bundle = *itrBundle;
        if (bundle == nullptr) {
            continue;
        }
        for (auto itrProcess = bundle->procs_.begin(); itrProcess != bundle->procs_.end();) {
            auto itProc = std::find(alivePids.begin(), alivePids.end(), itrProcess->second.pid_);
            if (itProc == alivePids.end()) {
                itrProcess = bundle->procs_.erase(itrProcess);
                continue;
            } else {
                HandleDiedExtensionBindToMe(itrProcess, alivePids);
                HandleDiedExtensionBindFromMe(itrProcess, alivePids);
            }
            ++itrProcess;
        }
        if (bundle->GetProcsCount() == 0) {
            std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(bundle->accountId_);
            if (account != nullptr) {
                account->RemoveBundleById(bundle->uid_);
                itrBundle = totalBundlePrioSet_.erase(itrBundle);
                continue;
            }
        }
        ++itrBundle;
    }
}

void ReclaimPriorityManager::HandleDiedExtensionBindToMe(
    std::map<pid_t, ProcessPriorityInfo>::iterator processPriorityInfoMap, const std::vector<unsigned int> &alivePids)
{
    std::vector<int32_t> diedProcsForConnectors;
    for (auto &pair : processPriorityInfoMap->second.procsBindToMe_) {
        auto connector = pair.first;
        if (std::find(alivePids.begin(), alivePids.end(), connector) == alivePids.end()) {
            diedProcsForConnectors.push_back(connector);
        }
    }
    for (int32_t diedPid : diedProcsForConnectors) {
        processPriorityInfoMap->second.ProcUnBindToMe(diedPid);
    }
}

void ReclaimPriorityManager::HandleDiedExtensionBindFromMe(
    std::map<pid_t, ProcessPriorityInfo>::iterator processPriorityInfoMap, const std::vector<unsigned int> &alivePids)
{
    std::vector<int32_t> diedProcsForConnectors;
    for (auto &pair : processPriorityInfoMap->second.procsBindFromMe_) {
        auto connector = pair.first;
        if (std::find(alivePids.begin(), alivePids.end(), connector) == alivePids.end()) {
            diedProcsForConnectors.push_back(connector);
        }
    }
    for (int32_t diedPid : diedProcsForConnectors) {
        processPriorityInfoMap->second.ProcUnBindFromMe(diedPid);
    }
}

void ReclaimPriorityManager::HandleDiedProcessCheck()
{
    FilterDiedProcess();
    if (totalBundlePrioSet_.size() > MAX_TOTALBUNDLESET_SIZE) {
        SetTimerForDiedProcessCheck(TIMER_DIED_PROC_FAST_CHECK_MS);
    } else {
        SetTimerForDiedProcessCheck(TIMER_DIED_PROC_SLOW_CHECK_MS);
    }
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

bool ReclaimPriorityManager::UpdateExtensionStatusForTarget(UpdateRequest &request)
{
    ReqProc caller = request.caller;
    ReqProc target = request.target;
    int accountId = GetOsAccountLocalIdFromUid(target.uid);
    if (!IsProcExist(target.pid, target.uid, accountId)) {
        HILOGE("process not exist and not to create it!!");
        return false;
    }
    std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(target.uid);
    ProcessPriorityInfo &proc = bundle->FindProcByPid(target.pid);

    if (proc.priority_ <= RECLAIM_PRIORITY_KILLABLE_SYSTEM || bundle->priority_ <= RECLAIM_PRIORITY_KILLABLE_SYSTEM ||
        IsKillableSystemApp(bundle)) {
        HILOGD("%{public}s is system app, skip!", target.bundleName.c_str());
        return true;
    }

    if (request.reason == AppStateUpdateReason::BIND_EXTENSION) {
        proc.isFreground = false; // current process is a extension, it never be a fg app.
        proc.isExtension_ = true;
        proc.ProcBindToMe(caller.pid, caller.uid);
    } else if (request.reason == AppStateUpdateReason::UNBIND_EXTENSION) {
        proc.isFreground = false; // current process is a extension, it never be a fg app.
        proc.isExtension_ = true;
        proc.ProcUnBindToMe(caller.pid);
    }
    AppAction action = AppAction::OTHERS;
    HandleUpdateProcess(request.reason, bundle, proc, action);
    return ApplyReclaimPriority(bundle, target.pid, action);
}

bool ReclaimPriorityManager::UpdateExtensionStatusForCaller(UpdateRequest &request)
{
    ReqProc caller = request.caller;
    ReqProc target = request.target;
    int callerAccountId = GetOsAccountLocalIdFromUid(caller.uid);
    if (!IsProcExist(caller.pid, caller.uid, callerAccountId)) {
        HILOGE("caller process not exist and not to create it!!");
        return false;
    }
    std::shared_ptr<AccountBundleInfo> callerAccount = FindOsAccountById(callerAccountId);
    std::shared_ptr<BundlePriorityInfo> callerBundle = callerAccount->FindBundleById(caller.uid);
    ProcessPriorityInfo &callerProc = callerBundle->FindProcByPid(caller.pid);

    if (request.reason == AppStateUpdateReason::BIND_EXTENSION) {
        callerProc.ProcBindFromMe(target.pid, target.uid);
    } else if (request.reason == AppStateUpdateReason::UNBIND_EXTENSION) {
        callerProc.ProcUnBindFromMe(target.pid);
    }
    return true;
}

void ReclaimPriorityManager::GetConnectedExtensionProc(const ProcessPriorityInfo &proc, ProcInfoVec &procVec)
{
    std::set<int32_t> isExtensionProcVisitedSet;
    std::queue<ProcessPriorityInfo> extensionProcQue;
    extensionProcQue.push(proc);

    while (!extensionProcQue.empty()) {
        ProcessPriorityInfo extensionProc(extensionProcQue.front());
        extensionProcQue.pop();
        if (isExtensionProcVisitedSet.count(extensionProc.pid_)) {
            continue;
        }
        if (extensionProc.isExtension_) {
            procVec.push_back(extensionProc);
            isExtensionProcVisitedSet.insert(extensionProc.pid_);
        }

        for (const auto &pair : extensionProc.procsBindFromMe_) {
            if (isExtensionProcVisitedSet.count(pair.first)) {
                continue;
            }
            int accountId = GetOsAccountLocalIdFromUid(pair.second);
            if (!IsProcExist(pair.first, pair.second, accountId)) {
                continue;
            }
            std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(accountId);
            std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(pair.second);
            ProcessPriorityInfo &procBindFromMe = bundle->FindProcByPid(pair.first);
            extensionProcQue.push(procBindFromMe);
        }
    }
}

void ReclaimPriorityManager::CalculateExtensionProcPrio(ProcInfoVec &procVec, ProcInfoSet &procSet)
{
    for (auto &extensionProc : procVec) {
        int32_t minExtensionPriority = RECLAIM_PRIORITY_BACKGROUND;
        for (const auto &connectorProcPair : extensionProc.procsBindToMe_) {
            int accountId = GetOsAccountLocalIdFromUid(connectorProcPair.second);
            if (!IsProcExist(connectorProcPair.first, connectorProcPair.second, accountId)) {
                minExtensionPriority = 0;
                continue;
            }

            std::shared_ptr<AccountBundleInfo> connectorAccount = FindOsAccountById(accountId);
            std::shared_ptr<BundlePriorityInfo> bundle = connectorAccount->FindBundleById(connectorProcPair.second);
            ProcessPriorityInfo &procBindToMe = bundle->FindProcByPid(connectorProcPair.first);
            minExtensionPriority =
                minExtensionPriority < procBindToMe.priority_ ? minExtensionPriority : procBindToMe.priority_;
        }

        extensionProc.priority_ = minExtensionPriority;
        procSet.insert(extensionProc);
    }
}

void ReclaimPriorityManager::SetConnectExtensionProcPrio(const ProcInfoSet &procSet)
{
    int32_t deltaPriority = 100;
    for (const auto &extensionProcess : procSet) {
        int32_t minExtensionPriority = RECLAIM_PRIORITY_BACKGROUND;
        for (const auto &connectorProcPair : extensionProcess.procsBindToMe_) {
            int accountId = GetOsAccountLocalIdFromUid(connectorProcPair.second);
            if (!IsProcExist(connectorProcPair.first, connectorProcPair.second, accountId)) {
                minExtensionPriority = 0;
                continue;
            }
            std::shared_ptr<AccountBundleInfo> connectorAccount = FindOsAccountById(accountId);
            std::shared_ptr<BundlePriorityInfo> bundle = connectorAccount->FindBundleById(connectorProcPair.second);
            ProcessPriorityInfo &procBindToMe = bundle->FindProcByPid(connectorProcPair.first);
            int32_t procBindToMePrio = procBindToMe.priority_ < 0 ? 0 : procBindToMe.priority_;
            minExtensionPriority =
                minExtensionPriority < procBindToMePrio ? minExtensionPriority : procBindToMePrio;
        }

        int extensionAccountId = GetOsAccountLocalIdFromUid(extensionProcess.uid_);
        if (!IsProcExist(extensionProcess.pid_, extensionProcess.uid_, extensionAccountId)) {
            continue;
        }

        std::shared_ptr<AccountBundleInfo> extensionAccount = FindOsAccountById(extensionAccountId);
        std::shared_ptr<BundlePriorityInfo> extensionBundle = extensionAccount->FindBundleById(extensionProcess.uid_);
        ProcessPriorityInfo &procExtensionUpdate = extensionBundle->FindProcByPid(extensionProcess.pid_);
        procExtensionUpdate.SetPriority(minExtensionPriority + deltaPriority);
        if (procExtensionUpdate.isImportant_) {
            SetImportantProcPriority(procExtensionUpdate);
        }
        UpdateBundlePriority(extensionBundle);
        OomScoreAdjUtils::WriteOomScoreAdjToKernel(procExtensionUpdate.pid_, procExtensionUpdate.priority_);
    }
}

bool ReclaimPriorityManager::HandleExtensionProcess(UpdateRequest &request)
{
    UpdateExtensionStatusForCaller(request);
    return UpdateExtensionStatusForTarget(request);
}


bool ReclaimPriorityManager::UpdateReclaimPriorityInner(UpdateRequest request)
{
    // This function can only be called by UpdateReclaimPriority, otherwise it may deadlock.
    std::lock_guard<std::mutex> setLock(totalBundlePrioSetLock_);
    ReqProc target = request.target;
    int accountId = GetOsAccountLocalIdFromUid(target.uid);
    HILOGD("accountId=%{public}d", accountId);

    if (request.reason == AppStateUpdateReason::BIND_EXTENSION ||
        request.reason == AppStateUpdateReason::UNBIND_EXTENSION) {
        return HandleExtensionProcess(request);
    }

    if (request.reason == AppStateUpdateReason::CREATE_PROCESS) {
        return HandleCreateProcess(target, accountId);
    }

    if (request.reason == AppStateUpdateReason::RENDER_CREATE_PROCESS) {
        HILOGD("call HandleCreateProcess");
        return HandleCreateProcess(target, accountId, true);
    }

    if (!IsProcExist(target.pid, target.uid, accountId)) {
        HILOGE("process not exist and not to create it!!");
        return false;
    }
    std::shared_ptr<AccountBundleInfo> account = FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(target.uid);
    if (bundle->priority_ <= RECLAIM_PRIORITY_KILLABLE_SYSTEM) {
        HILOGI("%{public}s is system app, skip!", target.bundleName.c_str());
        return true;
    }

    if (request.reason == AppStateUpdateReason::APPLICATION_SUSPEND) {
        return HandleApplicationSuspend(bundle);
    }

    ProcessPriorityInfo &proc = bundle->FindProcByPid(target.pid);
    AppAction action = AppAction::OTHERS;
    if (request.reason == AppStateUpdateReason::PROCESS_TERMINATED) {
        return HandleTerminateProcess(proc, bundle, account);
    } else {
        HandleUpdateProcess(request.reason, bundle, proc, action);
    }
    return ApplyReclaimPriority(bundle, target.pid, action);
}

bool ReclaimPriorityManager::IsImportantProc(const std::string procName, int &dstPriority)
{
    std::map<std::string, int> importantProcs = config_.GetImportantBgApps();
    if (importantProcs.count(procName)) {
        dstPriority = importantProcs.at(procName);
        HILOGD("is an important proc, procName=%{public}s, importPriority=%{public}d", procName.c_str(), dstPriority);
        return true;
    }
    return false;
}

void ReclaimPriorityManager::SetImportantProcPriority(ProcessPriorityInfo &proc)
{
    if (proc.priority_ > proc.priorityIfImportant_) {
        proc.priority_ = proc.priorityIfImportant_;
    }
}

void ReclaimPriorityManager::UpdatePriorityByProcForExtension(ProcessPriorityInfo &proc)
{
    ProcInfoVec allConnectedExtensionProcVec;
    GetConnectedExtensionProc(proc, allConnectedExtensionProcVec);
    if (allConnectedExtensionProcVec.size() == 0) {
        return;
    }
    ProcInfoSet allConnectedExtensionProcSet;
    CalculateExtensionProcPrio(allConnectedExtensionProcVec, allConnectedExtensionProcSet);

    SetConnectExtensionProcPrio(allConnectedExtensionProcSet);
}

void ReclaimPriorityManager::UpdatePriorityByProcConnector(ProcessPriorityInfo &proc)
{
    if (proc.procsBindFromMe_.size() == 0) {
        return;
    }
    proc.SetPriority(RECLAIM_PRIORITY_BACKGROUND);
    int minPriority = RECLAIM_PRIORITY_UNKNOWN;
    for (auto &pair : proc.procsBindFromMe_) {
        int32_t connectorUid = pair.second;
        int connectorAccountId = GetOsAccountLocalIdFromUid(connectorUid);
        std::shared_ptr<AccountBundleInfo> connectorAccount = FindOsAccountById(connectorAccountId);
        if (connectorAccount == nullptr || !connectorAccount->HasBundle(connectorUid)) {
            minPriority = 0; // native
            continue;
        }
        std::shared_ptr<BundlePriorityInfo> connectorBundle = connectorAccount->FindBundleById(connectorUid);
        if (connectorBundle == nullptr) {
            return;
        }
        for (auto procEntry : connectorBundle->procs_) {
            ProcessPriorityInfo &connectorProc = procEntry.second;
            minPriority = connectorProc.priority_ < minPriority ? connectorProc.priority_ : minPriority;
        }
    }
    proc.SetPriority(minPriority + 100); //raise the priority of the lowest-priority process by 100
}

void ReclaimPriorityManager::UpdatePriorityByProcStatus(std::shared_ptr<BundlePriorityInfo> bundle,
                                                        ProcessPriorityInfo &proc)
{
    if (bundle->priority_ < RECLAIM_PRIORITY_FOREGROUND) { // is a system process
        return;
    }
    if (proc.isRender_) { // priority of render follow its host and it is updated by action of its host
        return;
    }
    if (proc.isFreground) { // is a freground process
        if (proc.priority_ > RECLAIM_PRIORITY_FOREGROUND) {
            proc.SetPriority(RECLAIM_PRIORITY_FOREGROUND);
        }
    } else if (!proc.isExtension_) { // is a background process
        proc.SetPriority(RECLAIM_PRIORITY_BACKGROUND);
    }
    if (proc.isVisible_) {
        if (proc.priority_ > RECLAIM_PRIORITY_VISIBLE) {
            proc.SetPriority(RECLAIM_PRIORITY_VISIBLE);
        }
    }
    if (proc.isSuspendDelay) { // is a background process with transient task
        if (proc.priority_ > RECLAIM_PRIORITY_BG_SUSPEND_DELAY) {
            proc.SetPriority(RECLAIM_PRIORITY_BG_SUSPEND_DELAY);
        }
    } else if (proc.isBackgroundRunning || proc.isEventStart) {
        // is a background perceived process
        if (proc.priority_ > RECLAIM_PRIORITY_BG_PERCEIVED) {
            proc.SetPriority(RECLAIM_PRIORITY_BG_PERCEIVED);
        }
    } else if (proc.isDistDeviceConnected) { // is a background process connected by distribute device
        if (proc.priority_ > RECLAIM_PRIORITY_BG_DIST_DEVICE) {
            proc.SetPriority(RECLAIM_PRIORITY_BG_DIST_DEVICE);
        }
    } else {
        // is a plain background process
    }

    if (proc.isImportant_) {
        SetImportantProcPriority(proc);
    }
    UpdateBundlePriority(bundle);
    UpdatePriorityByProcForExtension(proc);
}

void ReclaimPriorityManager::HandleForeground(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isFreground = true;
    action = AppAction::APP_FOREGROUND;
}

void ReclaimPriorityManager::HandleBackground(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isFreground = false;
    action = AppAction::APP_BACKGROUND;
}

void ReclaimPriorityManager::HandleSuspendDelayStart(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isSuspendDelay = true;
}

void ReclaimPriorityManager::HandleSuspendDelayEnd(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isSuspendDelay = false;
}

void ReclaimPriorityManager::HandleBackgroundRunningStart(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isBackgroundRunning = true;
}

void ReclaimPriorityManager::HandleBackgroundRunningEnd(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isBackgroundRunning = false;
}

void ReclaimPriorityManager::HandleEventStart(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isEventStart = true;
}

void ReclaimPriorityManager::HandleEventEnd(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isEventStart = false;
}

void ReclaimPriorityManager::HandleDistDeviceConnected(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isDistDeviceConnected = true;
}

void ReclaimPriorityManager::HandleDistDeviceDisconnected(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isDistDeviceConnected = false;
}

void ReclaimPriorityManager::HandleBindExtension(ProcessPriorityInfo &proc, AppAction &action)
{
    if (proc.ExtensionConnectorsCount() > 0) {
        proc.extensionBindStatus = EXTENSION_STATUS_FG_BIND;
    }
}

void ReclaimPriorityManager::HandleUnbindExtension(ProcessPriorityInfo &proc, AppAction &action)
{
    if (proc.ExtensionConnectorsCount() == 0) {
        proc.extensionBindStatus = EXTENSION_STATUS_NO_BIND;
    }
}

void ReclaimPriorityManager::HandleVisible(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isVisible_ = true;
}

void ReclaimPriorityManager::HandleUnvisible(ProcessPriorityInfo &proc, AppAction &action)
{
    proc.isVisible_ = false;
}

void ReclaimPriorityManager::InitChangeProcMapping()
{
    changeProcMapping_[AppStateUpdateReason::FOREGROUND] = &ReclaimPriorityManager::HandleForeground;
    changeProcMapping_[AppStateUpdateReason::BACKGROUND] = &ReclaimPriorityManager::HandleBackground;
    changeProcMapping_[AppStateUpdateReason::SUSPEND_DELAY_START] = &ReclaimPriorityManager::HandleSuspendDelayStart;
    changeProcMapping_[AppStateUpdateReason::SUSPEND_DELAY_END] = &ReclaimPriorityManager::HandleSuspendDelayEnd;
    changeProcMapping_[AppStateUpdateReason::BACKGROUND_RUNNING_START] =
        &ReclaimPriorityManager::HandleBackgroundRunningStart;
    changeProcMapping_[AppStateUpdateReason::BACKGROUND_RUNNING_END] =
        &ReclaimPriorityManager::HandleBackgroundRunningEnd;
    changeProcMapping_[AppStateUpdateReason::EVENT_START] = &ReclaimPriorityManager::HandleEventStart;
    changeProcMapping_[AppStateUpdateReason::EVENT_END] = &ReclaimPriorityManager::HandleEventEnd;
    changeProcMapping_[AppStateUpdateReason::DIST_DEVICE_CONNECTED] =
        &ReclaimPriorityManager::HandleDistDeviceConnected;
    changeProcMapping_[AppStateUpdateReason::DIST_DEVICE_DISCONNECTED] =
        &ReclaimPriorityManager::HandleDistDeviceDisconnected;
    changeProcMapping_[AppStateUpdateReason::BIND_EXTENSION] = &ReclaimPriorityManager::HandleBindExtension;
    changeProcMapping_[AppStateUpdateReason::UNBIND_EXTENSION] = &ReclaimPriorityManager::HandleUnbindExtension;
    changeProcMapping_[AppStateUpdateReason::VISIBLE] = &ReclaimPriorityManager::HandleVisible;
    changeProcMapping_[AppStateUpdateReason::UN_VISIBLE] = &ReclaimPriorityManager::HandleUnvisible;
}


void ReclaimPriorityManager::HandleUpdateProcess(AppStateUpdateReason reason,
    std::shared_ptr<BundlePriorityInfo> bundle, ProcessPriorityInfo &proc, AppAction &action)
{
    HILOGD("called, bundle[uid_=%{public}d,name=%{public}s,priority=%{public}d], proc[pid_=%{public}d, uid=%{public}d,"
        "isFreground=%{public}d, isBackgroundRunning=%{public}d, isSuspendDelay=%{public}d, isEventStart=%{public}d,"
        "isDistDeviceConnected=%{public}d, extensionBindStatus=%{public}d, priority=%{public}d], case:%{public}s",
        bundle->uid_, bundle->name_.c_str(), bundle->priority_, proc.pid_, proc.uid_, proc.isFreground,
        proc.isBackgroundRunning, proc.isSuspendDelay, proc.isEventStart, proc.isDistDeviceConnected,
        proc.extensionBindStatus, proc.priority_, AppStateUpdateResonToString(reason).c_str());
    auto it = changeProcMapping_.find(reason);
    if (it != changeProcMapping_.end()) {
        auto changeProcPtr = it->second;
        (this->*changeProcPtr)(proc, action);
    }
    UpdatePriorityByProcStatus(bundle, proc);
}

bool ReclaimPriorityManager::ApplyReclaimPriority(std::shared_ptr<BundlePriorityInfo> bundle,
    pid_t pid, AppAction action)
{
    HILOGD("called");
    if (bundle == nullptr) {
        HILOGD("bundle is nullptr");
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

void ReclaimPriorityManager::Reset()
{
    // add locks
    std::lock_guard<std::mutex> setLock(totalBundlePrioSetLock_);

    HILOGI("clear totalBundlePrioSet(size: %{public}zu) and osAccountslnfoMap(size: %{public}zu) ",
        totalBundlePrioSet_.size(), osAccountsInfoMap_.size());
    totalBundlePrioSet_.clear();
    osAccountsInfoMap_.clear();
}

} // namespace Memory
} // namespace OHOS
