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
#include "memory_level_manager.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "app_mem_info.h"
#include "app_mgr_client.h"
#include "kernel_interface.h"
#include "reclaim_priority_manager.h"
#include "memmgr_config_manager.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "MemoryLevelManager";
}

IMPLEMENT_SINGLE_INSTANCE(MemoryLevelManager);

MemoryLevelManager::MemoryLevelManager()
{
    initialized_ = GetEventHandler();
    if (initialized_) {
        HILOGI("init succeeded");
    } else {
        HILOGE("init failed");
    }
}

bool MemoryLevelManager::GetEventHandler()
{
    if (!handler_) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

bool MemoryLevelManager::CalcSystemMemoryLevel(SystemMemoryLevel &level)
{
    int currentBuffer = KernelInterface::GetInstance().GetCurrentBuffer();
    std::shared_ptr<SystemMemoryLevelConfig> config = MemmgrConfigManager::GetInstance().GetSystemMemoryLevelConfig();
    if (config == nullptr) {
        HILOGE("The SystemMemoryLevelConfig is NULL.");
        return false;
    }

    if (currentBuffer <= config->critical) {
        level = SystemMemoryLevel::MEMORY_LEVEL_CRITICAL;
    } else if (currentBuffer <= config->low) {
        level = SystemMemoryLevel::MEMORY_LEVEL_LOW;
    } else if (currentBuffer <= config->moderate) {
        level = SystemMemoryLevel::MEMORY_LEVEL_MODERATE;
    } else {
        return false;
    }

    HILOGI("critical:%{public}d low:%{public}d moderate:%{public}d in config, curBuf:%{public}dKB, level:%{public}d.",
           config->critical, config->low, config->moderate, currentBuffer, static_cast<int>(level));
    return true;
}

bool MemoryLevelManager::CalcReclaimAppList(std::vector<std::shared_ptr<AppEntity>> &appList)
{
    ReclaimPriorityManager::BunldeCopySet bundleSet;
    ReclaimPriorityManager::GetInstance().GetBundlePrioSet(bundleSet);
    for (auto bundleInfo : bundleSet) {
        std::shared_ptr<AppEntity> app;
        MAKE_POINTER(app, shared, AppEntity, "make shared failed", return false, bundleInfo.uid_, bundleInfo.name_);
        appList.push_back(app);
        HILOGI("The app will be reclaimed, uid:%{public}d, name:%{public}s.", app->uid_, app->name_.c_str());
    }
    return true;
}

void MemoryLevelManager::PsiHandlerInner()
{
    HILOGD("[%{public}ld] called", ++calledCount_);

    /* Calculate the system memory level */
    SystemMemoryLevel level;
    if (!CalcSystemMemoryLevel(level)) {
        return;
    }

    DECLARE_UNIQUE_POINTER(AppExecFwk::AppMgrClient, appMgrClient_);
    MAKE_POINTER(appMgrClient_, unique, AppExecFwk::AppMgrClient, "make unique failed", return,
        /* no param */);
    switch (level) {
        case SystemMemoryLevel::MEMORY_LEVEL_MODERATE:
            appMgrClient_->NotifyMemoryLevel(AppExecFwk::MemoryLevel::MEMORY_LEVEL_MODERATE);
            break;
        case SystemMemoryLevel::MEMORY_LEVEL_LOW:
            appMgrClient_->NotifyMemoryLevel(AppExecFwk::MemoryLevel::MEMORY_LEVEL_LOW);
            break;
        case SystemMemoryLevel::MEMORY_LEVEL_CRITICAL:
            appMgrClient_->NotifyMemoryLevel(AppExecFwk::MemoryLevel::MEMORY_LEVEL_CRITICAL);
            break;
        default:
            break;
    }
}

void MemoryLevelManager::PsiHandler()
{
    if (!initialized_) {
        HILOGE("is not initialized, return!");
        return;
    }
    std::function<void()> func = std::bind(&MemoryLevelManager::PsiHandlerInner, this);
    handler_->PostImmediateTask(func);
}
} // namespace Memory
} // namespace OHOS
