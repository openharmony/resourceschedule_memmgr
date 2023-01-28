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

#include "mem_mgr_service.h"
#include "memmgr_log.h"
#include "system_ability_definition.h"
#include "memmgr_config_manager.h"
#include "mem_mgr_event_center.h"
#include "nandlife_controller.h"
#include "reclaim_priority_manager.h"
#include "reclaim_strategy_manager.h"
#include "multi_account_manager.h"
#include "low_memory_killer.h"
#ifdef USE_PURGEABLE_MEMORY
#include "purgeable_mem_manager.h"
#include "kernel_interface.h"
#endif

#include <unistd.h>

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrService";
}

IMPLEMENT_SINGLE_INSTANCE(MemMgrService);
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(&MemMgrService::GetInstance());

MemMgrService::MemMgrService() : SystemAbility(MEMORY_MANAGER_SA_ID, true)
{
}

bool MemMgrService::Init()
{
    MemmgrConfigManager::GetInstance().Init();

    // init reclaim priority manager
    if (!ReclaimPriorityManager::GetInstance().Init()) {
        HILOGE("ReclaimPriorityManager init failed");
        return false;
    }

    // init multiple account manager
    MultiAccountManager::GetInstance().Init();

    // init reclaim strategy manager
    if (!ReclaimStrategyManager::GetInstance().Init()) {
        HILOGE("ReclaimStrategyManager init failed");
        return false;
    }

    // init event center, then managers above can work by event trigger
    if (!MemMgrEventCenter::GetInstance().Init()) {
        HILOGE("MemMgrEventCenter init failed");
        return false;
    }

    // init nandlife controller
    NandLifeController::GetInstance().Init();
    HILOGI("init successed");
    return true;
}

void MemMgrService::OnStart()
{
    HILOGI("called");
    if (!Init()) {
        HILOGE("init failed");
        return;
    }
    if (!Publish(this)) {
        HILOGE("publish SA failed");
        return;
    }
    HILOGI("publish SA successed");

    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    AddSystemAbilityListener(COMMON_EVENT_SERVICE_ABILITY_ID);
    AddSystemAbilityListener(BACKGROUND_TASK_MANAGER_SERVICE_ID);
    AddSystemAbilityListener(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN);
    AddSystemAbilityListener(SUBSYS_APPLICATIONS_SYS_ABILITY_ID_BEGIN);
    AddSystemAbilityListener(APP_MGR_SERVICE_ID);
    AddSystemAbilityListener(ABILITY_MGR_SERVICE_ID);
}

void MemMgrService::OnStop()
{
    HILOGI("called");
}

// implements of innerkits list below

int32_t MemMgrService::GetBundlePriorityList(BundlePriorityList &bundlePrioList)
{
    HILOGI("called");
    ReclaimPriorityManager::BunldeCopySet bundleSet;
    ReclaimPriorityManager::GetInstance().GetBundlePrioSet(bundleSet);
    for (auto bundlePriorityInfo : bundleSet) {
        Memory::BundlePriority bi = Memory::BundlePriority(bundlePriorityInfo.uid_,
            bundlePriorityInfo.name_, bundlePriorityInfo.priority_, bundlePriorityInfo.accountId_);
        bundlePrioList.AddBundleInfo(bi);
    }
    bundlePrioList.SetCount(bundlePrioList.Size());
    return 0;
}

int32_t MemMgrService::NotifyDistDevStatus(int32_t pid, int32_t uid, const std::string &name, bool connected)
{
    HILOGI("called, pid=%{public}d, uid=%{public}d, name=%{public}s, connected=%{public}d", pid, uid, name.c_str(),
        connected);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(pid, uid, name,
        connected ? AppStateUpdateReason::DIST_DEVICE_CONNECTED : AppStateUpdateReason::DIST_DEVICE_DISCONNECTED);
    return 0;
}

int32_t MemMgrService::GetKillLevelOfLmkd(int32_t &killLevel)
{
    HILOGI("called");
    killLevel = LowMemoryKiller::GetInstance().GetKillLevel();
    return 0;
}

#ifdef USE_PURGEABLE_MEMORY
int32_t MemMgrService::RegisterActiveApps(int32_t pid, int32_t uid)
{
    HILOGI("called, pid=%{public}d, uid=%{public}d", pid, uid);
    PurgeableMemManager::GetInstance().RegisterActiveApps(pid, uid);
    return 0;
}

int32_t MemMgrService::DeregisterActiveApps(int32_t pid, int32_t uid)
{
    HILOGI("called, pid=%{public}d, uid=%{public}d", pid, uid);
    PurgeableMemManager::GetInstance().DeregisterActiveApps(pid, uid);
    return 0;
}

int32_t MemMgrService::SubscribeAppState(const sptr<IAppStateSubscriber> &subscriber)
{
    HILOGI("called");
    PurgeableMemManager::GetInstance().AddSubscriber(subscriber);
    return 0;
}

int32_t MemMgrService::UnsubscribeAppState(const sptr<IAppStateSubscriber> &subscriber)
{
    HILOGI("called");
    PurgeableMemManager::GetInstance().RemoveSubscriber(subscriber);
    return 0;
}

int32_t MemMgrService::GetAvailableMemory()
{
    HILOGI("called");
    int currentBuffer = KernelInterface::GetInstance().GetCurrentBuffer();
    return currentBuffer;
}

int32_t MemMgrService::GetTotalMemory()
{
    HILOGI("called");
    int totalBuffer = KernelInterface::GetInstance().GetTotalBuffer();
    return totalBuffer;  
}
#endif // USE_PURGEABLE_MEMORY

void MemMgrService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbilityId: %{public}d add", systemAbilityId);
    MemMgrEventCenter::GetInstance().RetryRegisterEventObserver(systemAbilityId);
}

void MemMgrService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    HILOGI("systemAbilityId: %{public}d remove", systemAbilityId);
    MemMgrEventCenter::GetInstance().RemoveEventObserver(systemAbilityId);
}

void ShowHelpInfo(int fd)
{
    dprintf(fd, "Usage:\n");
    dprintf(fd, "-h                          |help for memmgrservice dumper\n");
    dprintf(fd, "-a                          |dump all info\n");
    dprintf(fd, "-e                          |dump event observer\n");
    dprintf(fd, "-r                          |dump reclaim info and adj\n");
    dprintf(fd, "-c                          |dump config\n");
#ifdef USE_PURGEABLE_MEMORY
    dprintf(fd, "-t                          |triger onTrim, 0:moderate, 1:low, 2:critical\n");
    dprintf(fd, "-f                          |triger Reclaim\n");
    dprintf(fd, "   -p                          |the pid of process to reclaim, default to reclaim all\n");
    dprintf(fd, "-s                          |show all the pid which can be reclaimed\n");
#endif
}

int MemMgrService::Dump(int fd, const std::vector<std::u16string> &args)
{
    HILOGI("called");
    std::vector<std::string> params;
    for (auto& arg : args) {
        params.emplace_back(Str16ToStr8(arg));
    }

    if (params.empty() || (params.size() == 1 && params[0] == "-h")) {
        ShowHelpInfo(fd);
    } else if (params.size() == 1 && params[0] == "-a") {
        MemMgrEventCenter::GetInstance().Dump(fd);
        ReclaimPriorityManager::GetInstance().Dump(fd);
    } else if (params.size() == 1 && params[0] == "-e") {
        MemMgrEventCenter::GetInstance().Dump(fd);
    } else if (params.size() == 1 && params[0] == "-r") {
        ReclaimPriorityManager::GetInstance().Dump(fd);
    } else if (params.size() == 1 && params[0] == "-c") {
        MemmgrConfigManager::GetInstance().Dump(fd);
#ifdef USE_PURGEABLE_MEMORY
    } else if (params.size() >= 1 && (params[0] == "-t" || params[0] == "-f" || params[0] == "-s")) {
        PurgeableMemManager::GetInstance().Test(fd, params);
#endif
    } 
    return 0;
}
} // namespace Memory
} // namespace OHOS
