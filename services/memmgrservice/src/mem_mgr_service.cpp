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
#include "mem_mgr_event_center.h"
#include "reclaim_priority_manager.h"
#include "reclaim_strategy_manager.h"
#include "multi_account_manager.h"

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
        Memory::BundlePriority *bi = new Memory::BundlePriority(bundlePriorityInfo.uid_,
            bundlePriorityInfo.name_, bundlePriorityInfo.priority_, bundlePriorityInfo.accountId_);
        bundlePrioList.AddBundleInfo(*bi);
    }
    bundlePrioList.SetCount(bundlePrioList.Size());
    return 0;
}
} // namespace Memory
} // namespace OHOS
