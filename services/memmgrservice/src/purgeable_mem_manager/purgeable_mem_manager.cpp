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

#include "purgeable_mem_manager.h"

#include <algorithm>

#include "accesstoken_kit.h"
#include "app_mem_info.h"
#include "app_mgr_constants.h"
#include "ipc_skeleton.h"
#include "memcg_mgr.h"
#include "memmgr_config_manager.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "reclaim_priority_manager.h"
#include "system_memory_level_config.h"
#include "purgeablemem_config.h"
 
namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "PurgeableMemManager";
}

IMPLEMENT_SINGLE_INSTANCE(PurgeableMemManager);

PurgeableMemManager::PurgeableMemManager()
{
    initialized_ = GetEventHandler();
    if (initialized_) {
        HILOGI("init succeeded");
    } else {
        HILOGE("init failed");
    }
    appList_.clear();
}

bool PurgeableMemManager::GetEventHandler()
{
    if (!handler_) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

void PurgeableMemManager::AddSubscriberInner(const sptr<IAppStateSubscriber> &subscriber)
{
    auto remoteObj = subscriber->AsObject();
    auto findSubscriber = [&remoteObj](const auto &target) { return remoteObj == target->AsObject(); };
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers_);
    auto subscriberIter = std::find_if(appStateSubscribers_.begin(), appStateSubscribers_.end(), findSubscriber);
    if (subscriberIter != appStateSubscribers_.end()) {
        HILOGE("target subscriber already exist");
        return;
    }

    if (appStateSubscribers_.size() >= PURGEABLE_SUBSCRIBER_MAX_NUM) {
        HILOGE("the number of registered subscribers has reach the upper limit");
        return;
    }

    appStateSubscribers_.emplace_back(subscriber);

    if (subscriber->AsObject() == nullptr) {
        HILOGE("subscriber is null");
        return;
    }

    if (subscriberRecipients_.find(subscriber->AsObject()) != subscriberRecipients_.end()) {
        HILOGE("subscriberRecipients_ don't find subscriber");
        return;
    }
    sptr<RemoteDeathRecipient> deathRecipient = new (std::nothrow)
        RemoteDeathRecipient([this] (const wptr<IRemoteObject> &remote) { this->OnRemoteSubscriberDied(remote); });
    if (!deathRecipient) {
        HILOGE("create death recipient failed");
        return;
    }

    subscriber->AsObject()->AddDeathRecipient(deathRecipient);
    subscriberRecipients_.emplace(subscriber->AsObject(), deathRecipient);
    subscriber->OnConnected();
    HILOGI("add app state subscriber succeed, subscriber list size is: %{public}d",
        static_cast<int>(appStateSubscribers_.size()));
}

void PurgeableMemManager::AddSubscriber(const sptr<IAppStateSubscriber> &subscriber)
{
    if (!CheckCallingToken()) {
        HILOGW("AddSubscriber not allowed");
        return;
    }
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    if (subscriber == nullptr) {
        HILOGE("subscriber is null");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::AddSubscriberInner, this, subscriber);
    handler_->PostImmediateTask(func);
}

void PurgeableMemManager::RemoveSubscriberInner(const sptr<IAppStateSubscriber> &subscriber)
{
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        HILOGE("subscriber object is null");
        return;
    }
    auto findSubscriber = [&remote] (const auto &targetSubscriber) { return remote == targetSubscriber->AsObject(); };

    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers_);
    auto subscriberIter = find_if(appStateSubscribers_.begin(), appStateSubscribers_.end(), findSubscriber);
    if (subscriberIter == appStateSubscribers_.end()) {
        HILOGE("subscriber to remove is not exists");
        return;
    }

    auto iter = subscriberRecipients_.find(remote);
    if (iter != subscriberRecipients_.end()) {
        iter->first->RemoveDeathRecipient(iter->second);
        subscriberRecipients_.erase(iter);
    }
    subscriber->OnDisconnected();
    appStateSubscribers_.erase(subscriberIter);
    HILOGI("remove subscriber succeed, subscriber list size is: %{public}d",
        static_cast<int>(appStateSubscribers_.size()));
}

void PurgeableMemManager::RemoveSubscriber(const sptr<IAppStateSubscriber> &subscriber)
{
    if (!CheckCallingToken()) {
        HILOGW("RemoveSubscriber not allowed");
        return;
    }
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    if (subscriber == nullptr) {
        HILOGE("subscriber is null");
        return;
    }
    RemoveSubscriberInner(subscriber);
}

void PurgeableMemManager::OnRemoteSubscriberDiedInner(const wptr<IRemoteObject> &object)
{
    sptr<IRemoteObject> objectProxy = object.promote();
    if (!objectProxy) {
        HILOGE("get remote object failed");
        return;
    }
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers_);
    auto iter = appStateSubscribers_.begin();
    while (iter != appStateSubscribers_.end()) {
        if ((*iter)->AsObject() == objectProxy) {
            iter = appStateSubscribers_.erase(iter);
            HILOGI("remove the subscriber");
        } else {
            iter++;
        }
    }
    HILOGI("recipients remove the subscriber, subscriber list size is : %{public}d",
        static_cast<int>(appStateSubscribers_.size()));
    subscriberRecipients_.erase(objectProxy);
}

void PurgeableMemManager::OnRemoteSubscriberDied(const wptr<IRemoteObject> &object)
{
    if (object == nullptr) {
        HILOGE("remote object is null");
        return;
    }

    handler_->PostSyncTask([this, &object]() { this->OnRemoteSubscriberDiedInner(object); });
}

void PurgeableMemManager::RegisterActiveAppsInner(int32_t pid, int32_t uid)
{
    std::lock_guard<std::mutex> lockAppList(mutexAppList_);
    if (appList_.size() >= PURGEABLE_APPSTATE_MAX_NUM) {
        HILOGE("the number of registered apps has reached the upper limit");
        return;
    }

    if (appList_.find(pid) != appList_.end()) {
        HILOGE("the app has already registered");
        return;
    }
    int32_t state = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    std::pair<int32_t, int32_t> appinfo = std::make_pair(uid, state);
    appList_[pid] = appinfo;
    HILOGI("the app is registered, pid is: %{public}d, uid is %{public}d", pid, uid);
}

void PurgeableMemManager::RegisterActiveApps(int32_t pid, int32_t uid)
{
    if (!CheckCallingToken()) {
        HILOGW("AddSubscriber not allowed");
        return;
    }
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::RegisterActiveAppsInner, this, pid, uid);
    handler_->PostImmediateTask(func);
}

void PurgeableMemManager::DeregisterActiveAppsInner(int32_t pid, int32_t uid)
{
    std::lock_guard<std::mutex> lockAppList(mutexAppList_);
    if (appList_.find(pid) == appList_.end()) {
        HILOGE("the app is not registered");
        return;
    }
    std::pair<int32_t, int32_t> appinfo = appList_[pid];
    if (appinfo.first != uid) {
        HILOGE("uid don't match the pid");
        return;
    }
    appList_.erase(pid);
    HILOGI("the app is deregistered, pid is: %{public}d, uid is %{public}d", pid, uid);
}

void PurgeableMemManager::DeregisterActiveApps(int32_t pid, int32_t uid)
{
    if (!CheckCallingToken()) {
        HILOGW("AddSubscriber not allowed");
        return;
    }
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::DeregisterActiveAppsInner, this, pid, uid);
    handler_->PostImmediateTask(func);
}

void PurgeableMemManager::ChangeAppStateInner(int32_t pid, int32_t uid, int32_t state)
{
    {
        std::lock_guard<std::mutex> lockAppList(mutexAppList_);
        if (appList_.find(pid) == appList_.end()) {
            HILOGE("the app is not registered");
            return;
        }
        std::pair<int32_t, int32_t> appinfo = appList_[pid];
        if (appinfo.first != uid) {
            HILOGE("uid don't match the pid");
            return;
        }
        int32_t oldState = appList_[pid].second;
        appList_[pid].second = state;
        HILOGI("state is changed, old state: %{public}d, new state: %{public}d pid: %{public}d, uid: %{public}d",
            oldState, appList_[pid].second, pid, uid);
    }

    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers_);
    auto iter = appStateSubscribers_.begin();
    while (iter != appStateSubscribers_.end()) {
        HILOGI("do OnAppStateChanged");
        (*iter)->OnAppStateChanged(pid, uid, state);
        iter++;
    }
}

void PurgeableMemManager::ChangeAppState(int32_t pid, int32_t uid, int32_t state)
{
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::ChangeAppStateInner, this, pid, uid, state);
    handler_->PostImmediateTask(func);
}

void PurgeableMemManager::TrimAllSubscribers(const SystemMemoryLevel &level)
{
    HILOGD("enter! onTrim memory level is %{public}d \n", level);
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers_);
    auto iter = appStateSubscribers_.begin();
    while (iter != appStateSubscribers_.end()) {
        (*iter)->OnTrim(level);
        iter++;
    }
}

bool PurgeableMemManager::CheckCallingToken()
{
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        tokenFlag == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        return true;
    }
    return false;
}

void PurgeableMemManager::ReclaimSubscriberAll()
{
    HILOGD("enter! Force Subscribers Reclaim all");
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers_);
    std::lock_guard<std::mutex> lockAppList(mutexAppList_);
    auto subscriberIter = appStateSubscribers_.begin();
    int pid = -1, uid = -1;
    while (subscriberIter != appStateSubscribers_.end()) {
        HILOGI("do ForceReclaim");
        auto appListIter = appList_.begin();
        while (appListIter != appList_.end()) {
            pid = appListIter->first;
            std::pair<int32_t, int32_t> appinfo = appListIter->second;
            uid = appinfo.first;
            (*subscriberIter)->ForceReclaim(pid, uid);
            appListIter++;
        }
        subscriberIter++;
    }
}

void PurgeableMemManager::ReclaimSubscriberProc(const int32_t pid)
{
    HILOGD("enter! Force Subscribers Reclaim: pid=%{public}d", pid);
    int32_t uid = -1;
    {
        std::lock_guard<std::mutex> lockAppList(mutexAppList_);
        if (appList_.find(pid) == appList_.end()) {
            HILOGE("the app is not registered");
            return;
        }

        std::pair<int32_t, int32_t> appinfo = appList_[pid];
        uid = appinfo.first;
    }
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers_);
    auto iter = appStateSubscribers_.begin();
    while (iter != appStateSubscribers_.end()) {
        HILOGI("do ForceReclaim");
        (*iter)->ForceReclaim(pid, uid);
        iter++;
    }
}

bool PurgeableMemManager::GetPurgeableInfo(PurgeableMemoryInfo &info)
{
    switch (info.type) {
        case PurgeableMemoryType::PURGEABLE_HEAP:
            return PurgeableMemUtils::GetInstance().GetPurgeableHeapInfo(info.reclaimableKB);
        case PurgeableMemoryType::PURGEABLE_ASHMEM:
            return PurgeableMemUtils::GetInstance().GetPurgeableAshmInfo(info.reclaimableKB, info.ashmInfoToReclaim);
        case PurgeableMemoryType::PURGEABLE_SUBSCRIBER:
            return false;
        default:
            break;
    }
    return false;
}

bool PurgeableMemManager::GetMemcgPathByUserId(const int userId, std::string &memcgPath)
{
    if (userId == 0) { // get system memcg path when userId = 0
        memcgPath = KernelInterface::MEMCG_BASE_PATH;
        return true;
    }
    UserMemcg *memcg = MemcgMgr::GetInstance().GetUserMemcg(userId);
    if (memcg == nullptr) {
        return false;
    }
    memcgPath = memcg->GetMemcgPath_();
    return true;
}

bool PurgeableMemManager::PurgeTypeAll(const PurgeableMemoryType &type)
{
    switch (type) {
        case PurgeableMemoryType::PURGEABLE_HEAP:
            return PurgeableMemUtils::GetInstance().PurgeHeapAll();
        case PurgeableMemoryType::PURGEABLE_ASHMEM:
            return PurgeableMemUtils::GetInstance().PurgeAshmAll();
        case PurgeableMemoryType::PURGEABLE_SUBSCRIBER:
            return false;
        default:
            break;
    }
    return false;
}

bool PurgeableMemManager::PurgeHeap(const int userId, const int size)
{
    std::string memcgPath;
    if (!GetMemcgPathByUserId(userId, memcgPath)) {
        return false;
    }
    return PurgeableMemUtils::GetInstance().PurgeHeapMemcg(memcgPath, size);
}

bool PurgeableMemManager::PurgeAshm(const unsigned int ashmId, const unsigned int time)
{
    std::string ashmIdWithTime = std::to_string(ashmId) + std::string(" ") + std::to_string(time);
    return PurgeableMemUtils::GetInstance().PurgeAshmByIdWithTime(ashmIdWithTime);
}

bool PurgeableMemManager::PurgHeapOneMemcg(const std::vector<int> &memcgPids, const std::string &memcgPath,
                                           const int reclaimTargetKB, int &reclaimResultKB)
{
    if (reclaimResultKB >= reclaimTargetKB) {
        return true;
    }

    int unPinedSizeKB = 0;
    for (auto &pid : memcgPids) {
        int reclaimableKB = 0;
        if (!PurgeableMemUtils::GetInstance().GetProcPurgeableHeapInfo(pid, reclaimableKB)) {
            continue;
        }
        unPinedSizeKB += reclaimableKB;
    }

    int toReclaimSize = 0;
    if (reclaimResultKB + unPinedSizeKB <= reclaimTargetKB) {
        toReclaimSize = unPinedSizeKB;
    } else {
        toReclaimSize = reclaimTargetKB - reclaimResultKB;
    }

    if (toReclaimSize > 0 && PurgeableMemUtils::GetInstance().PurgeHeapMemcg(memcgPath, toReclaimSize)) {
        HILOGI("reclaim purgeable [HEAP] for memcg[%{public}s], recult=%{public}d KB", memcgPath.c_str(),
               toReclaimSize);
        reclaimResultKB += toReclaimSize;
        if (reclaimResultKB >= reclaimTargetKB) {
            return true;
        }
    }
    return false;
}

void PurgeableMemManager::PurgHeapMemcgOneByOne(const int reclaimTargetKB, int &reclaimResultKB)
{
    reclaimResultKB = 0;
    std::vector<int> memcgPids;
    std::string memcgPath;
    std::vector<int> userIds;
    KernelInterface::GetInstance().GetAllUserIds(userIds);
    for (auto userId : userIds) {
        memcgPath = KernelInterface::GetInstance().JoinPath(KernelInterface::MEMCG_BASE_PATH, std::to_string(userId));
        memcgPids.clear();
        if (!KernelInterface::GetInstance().GetMemcgPids(memcgPath, memcgPids) || memcgPids.size() == 0) {
            continue;
        }
        if (PurgHeapOneMemcg(memcgPids, memcgPath, reclaimTargetKB, reclaimResultKB)) {
            return;
        }
    }

    memcgPids.clear();
    if (KernelInterface::GetInstance().GetMemcgPids(KernelInterface::MEMCG_BASE_PATH, memcgPids) &&
        memcgPids.size() > 0) {
        PurgHeapOneMemcg(memcgPids, KernelInterface::MEMCG_BASE_PATH, reclaimTargetKB, reclaimResultKB);
    }
}

bool PurgeableMemManager::AshmReclaimPriorityCompare(const PurgeableAshmInfo &left, const PurgeableAshmInfo &right)
{
    if (left.minPriority != right.minPriority) {
        return left.minPriority > right.minPriority;
    } else {
        return left.sizeKB > right.sizeKB;
    }
}

void PurgeableMemManager::PurgAshmIdOneByOne(std::vector<PurgeableAshmInfo> &ashmInfoToReclaim,
                                             const int reclaimTargetKB, int &reclaimResultKB)
{
    if (!ashmInfoToReclaim.empty()) {
        std::sort(ashmInfoToReclaim.begin(), ashmInfoToReclaim.end(),
                  [this](const auto &lhs, const auto &rhs) { return AshmReclaimPriorityCompare(lhs, rhs); });
    }

    reclaimResultKB = 0;
    for (auto &it : ashmInfoToReclaim) {
        if (!IsPurgeWhiteApp(it.curAppName)) {
            HILOGD("[%{public}s] is not in purgeable app white list!", it.curAppName.c_str());
            continue;
        }
        if (PurgeableMemUtils::GetInstance().PurgeAshmByIdWithTime(it.idWithTime)) {
            HILOGI("reclaim purgeable [ASHM] for ashmem_id[%{public}s], adj=%{public}d, result=%{public}d KB",
                   it.idWithTime.c_str(), it.minPriority, it.sizeKB);
            reclaimResultKB += it.sizeKB;
            if (reclaimResultKB >= reclaimTargetKB) {
                return;
            }
        }
    }
}

int PurgeableMemManager::PurgeByTypeAndTarget(const PurgeableMemoryType &type, const int reclaimTargetKB)
{
    std::string typeDesc = PurgMemType2String(type);
    PurgeableMemoryInfo info;
    info.type = type;
    if (!GetPurgeableInfo(info)) {
        HILOGD("GetPurgeableInfo with type[%{public}s] failed!", typeDesc.c_str());
        return 0;
    }
    if (info.reclaimableKB <= 0) {
        HILOGD("no unpined purgeable [%{public}s] to reclaim!", typeDesc.c_str());
        return 0;
    }
    HILOGI("purgeable[%{public}s]: reclaimableKB=%{public}dKB, target=%{public}dKB", typeDesc.c_str(),
           info.reclaimableKB, reclaimTargetKB);

    int reclaimResultKB = 0;

    // reclaim all unpined purgeable of this type
    if (type != PurgeableMemoryType::PURGEABLE_ASHMEM &&
        info.reclaimableKB <= reclaimTargetKB && PurgeTypeAll(type)) {
        reclaimResultKB = info.reclaimableKB;
        HILOGI("reclaim all purgeable [%{public}s], result=%{public}d KB", typeDesc.c_str(), reclaimResultKB);
        return reclaimResultKB;
    }

    // reclaim one by one
    switch (type) {
        case PurgeableMemoryType::PURGEABLE_HEAP:
            PurgHeapMemcgOneByOne(reclaimTargetKB, reclaimResultKB);
            break;
        case PurgeableMemoryType::PURGEABLE_ASHMEM:
            PurgAshmIdOneByOne(info.ashmInfoToReclaim, reclaimTargetKB, reclaimResultKB);
            break;
        case PurgeableMemoryType::PURGEABLE_SUBSCRIBER:
            break;
        default:
            break;
    }
    return reclaimResultKB;
}

std::string PurgeableMemManager::PurgMemType2String(const PurgeableMemoryType &type)
{
    switch (type) {
        case PurgeableMemoryType::PURGEABLE_HEAP:
            return "HEAP";
        case PurgeableMemoryType::PURGEABLE_ASHMEM:
            return "ASHM";
        case PurgeableMemoryType::PURGEABLE_SUBSCRIBER:
            return "SUBSCRIBER";
        default:
            return "";
    }
}

#define CHECK_RECLAIM_CONDITION(reclaimTargetKB, action) \
    do {                                                 \
        if ((reclaimTargetKB) <= 0) {                    \
            action;                                      \
        }                                                \
    } while (0)

void PurgeableMemManager::TriggerByPsi(const SystemMemoryInfo &info)
{
    HILOGD("called");
    time_t now = time(0);
    if (lastTriggerTime_ != 0 && (now - lastTriggerTime_) < TRIGGER_INTERVAL_SECOND) {
        HILOGD("Less than %{public}u s from last trigger, no action is required.", TRIGGER_INTERVAL_SECOND);
        return;
    } else {
        lastTriggerTime_ = now;
    }

    unsigned int currentBuffer = static_cast<unsigned int>(KernelInterface::GetInstance().GetCurrentBuffer());
    DECLARE_SHARED_POINTER(SystemMemoryLevelConfig, config);
    MAKE_POINTER(config, shared, SystemMemoryLevelConfig, "The SystemMemoryLevelConfig is NULL.", return,
                 MemmgrConfigManager::GetInstance().GetSystemMemoryLevelConfig());

    // cal target reclaim count
    unsigned int targetBuffer = config->GetPurgeable();
    int reclaimTargetKB = targetBuffer - currentBuffer;
    CHECK_RECLAIM_CONDITION(reclaimTargetKB, return);
    HILOGI("reclaim purgeable memory start: currentBuffer=%{public}uKB, purgeableLevel=%{public}uKB, "
           "reclaimTarget=%{public}dKB", currentBuffer, targetBuffer, reclaimTargetKB);

    std::vector<PurgeableMemoryType> sequence = {PurgeableMemoryType::PURGEABLE_HEAP,
                                                 PurgeableMemoryType::PURGEABLE_ASHMEM,
                                                 PurgeableMemoryType::PURGEABLE_SUBSCRIBER};

    int totalReclaimedKB = 0;
    for (auto typePtr = sequence.begin(); typePtr < sequence.end(); typePtr++) {
        int reclaimed = PurgeByTypeAndTarget(*typePtr, reclaimTargetKB);
        HILOGI("reclaimed %{public}dKB purgeable [%{public}s]", reclaimed, PurgMemType2String(*typePtr).c_str());
        reclaimTargetKB -= reclaimed;
        totalReclaimedKB += reclaimed;
        if (reclaimTargetKB <= 0) {
            HILOGI("total reclaimed %{public}dKB purgeable memory, reached target size!", totalReclaimedKB);
            return;
        }
    }
    HILOGI("purgeable_heap and purgeable_ashmem total reclaimed %{public}dKB, not reach target size!",
           totalReclaimedKB);

    TrimAllSubscribers(info.level); // heap和ashmem类型的purgeable内存全部回收完依然不够target时，触发onTrim
}

void PurgeableMemManager::TriggerByManualDump(const SystemMemoryInfo &info)
{
    HILOGD("enter!\n");
    if (info.level > SystemMemoryLevel::UNKNOWN && info.level <= SystemMemoryLevel::MEMORY_LEVEL_CRITICAL) {
        TrimAllSubscribers(info.level);
    }
}

/*
 * There are three ways to trigger me;
 * 1. By command of "hidumper -s 1909", see MemMgrService::Dump
 * 2. By trigger of kernel memory psi, see MemoryLevelManager
 * 3. By trigger of kswapd uploading, see KswapdObserver
 */
void PurgeableMemManager::NotifyMemoryLevelInner(const SystemMemoryInfo &info)
{
    switch (info.source) {
        case MemorySource::MANUAL_DUMP:
            TriggerByManualDump(info);
            break;
        case MemorySource::KSWAPD: // fall through
        case MemorySource::PSI_MEMORY:
            TriggerByPsi(info);
            break;
        default:
            HILOGE("unsupported source:%{public}d", info.source);
            break;
    }
}

void PurgeableMemManager::NotifyMemoryLevel(const SystemMemoryInfo &info)
{
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::NotifyMemoryLevelInner, this, info);
    handler_->PostImmediateTask(func);
}

bool PurgeableMemManager::ForceReclaimByDump(const DumpReclaimInfo &dumpInfo)
{
    if (dumpInfo.reclaimType == PurgeableMemoryType::UNKNOWN) {
        return false;
    }

    bool ret;
    switch (dumpInfo.reclaimType) {
        case PurgeableMemoryType::PURGEABLE_HEAP:
            if (dumpInfo.ifReclaimTypeAll) {
                return PurgeableMemUtils::GetInstance().PurgeHeapAll();
            } else {
                return PurgeHeap(dumpInfo.memcgUserId, dumpInfo.reclaimHeapSizeKB);
            }
        case PurgeableMemoryType::PURGEABLE_ASHMEM:
            if (dumpInfo.ifReclaimTypeAll) {
                return PurgeableMemUtils::GetInstance().PurgeAshmAll();
            } else {
                return PurgeAshm(dumpInfo.ashmId, dumpInfo.ashmTime);
            }
        case PurgeableMemoryType::PURGEABLE_SUBSCRIBER:
            if (dumpInfo.ifReclaimTypeAll) {
                ReclaimSubscriberAll();
            } else {
                ReclaimSubscriberProc(dumpInfo.subscriberPid);
            }
            return true;
        case PurgeableMemoryType::PURGEABLE_ALL:
            ret = PurgeableMemUtils::GetInstance().PurgeHeapAll();
            ret = ret && PurgeableMemUtils::GetInstance().PurgeAshmAll();
            ReclaimSubscriberAll();
            return ret;
        default:
            return false;
    }
}

void PurgeableMemManager::DumpSubscribers(const int fd)
{
    HILOGD("enter!\n");
    std::lock_guard<std::mutex> lockAppList(mutexAppList_);
    int32_t pid, uid, state;
    auto appListIter = appList_.begin();
    while (appListIter != appList_.end()) {
        pid = appListIter->first;
        std::pair<int32_t, int32_t> appinfo = appListIter->second;
        uid = appinfo.first;
        state = appinfo.second;
        dprintf(fd, "pid:%d, uid:%d state:%s\n", pid, uid, (state == APP_STATE_FOREGROUND) ?
            "Foreground" : "Background");
        appListIter++;
    }
}

bool PurgeableMemManager::IsPurgeWhiteApp(const std::string &curAppName)
{
    PurgeablememConfig pmc = MemmgrConfigManager::GetInstance().GetPurgeablememConfig();
    std::set<std::string> purgeWhiteAppSet = pmc.GetPurgeWhiteAppSet();
    for (auto it = purgeWhiteAppSet.begin(); it != purgeWhiteAppSet.end(); it++) {
        if (curAppName == *it) {
            return true;
        }
    }
    return false;
}
} // namespace Memory
} // namespace OHOS