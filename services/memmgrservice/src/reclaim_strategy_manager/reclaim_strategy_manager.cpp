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


#include "avail_buffer_manager.h"
#include "kernel_interface.h"
#include "memmgr_config_manager.h"
#include "memmgr_log.h"
#include "reclaim_strategy_constants.h"
#include "reclaim_strategy_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "ReclaimStrategyManager";
}

IMPLEMENT_SINGLE_INSTANCE(ReclaimStrategyManager);

ReclaimStrategyManager::ReclaimStrategyManager()
{
}

bool ReclaimStrategyManager::Init()
{
    initialized_ = false;
    do {
        if (!GetEventHandler()) {
            break;
        }
        memcgMgr_ = std::make_shared<MemcgMgr>();
        MemmgrConfigManager::GetInstance().Init();
        AvailBufferManager::GetInstance().Init();
        if (!memcgMgr_->SetRootMemcgPara()) {
            break;
        }
        initialized_ = true;
    } while (0);

    if (initialized_) {
        HILOGI("init successed");
    } else {
        HILOGE("init failed");
    }
    return initialized_;
}

bool ReclaimStrategyManager::GetEventHandler()
{
    if (handler_ == nullptr) {
        handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
    }
    if (handler_ == nullptr) {
        HILOGE("handler init failed");
        return false;
    }
    return true;
}

void ReclaimStrategyManager::NotifyAppStateChanged(std::shared_ptr<ReclaimParam> reclaimPara)
{
    if (!Initailized()) {
        HILOGE("has not been initialized_, skiped!");
        return;
    }
    std::function<bool()> func = std::bind(
        &ReclaimStrategyManager::HandleAppStateChanged, this, reclaimPara);
    handler_->PostImmediateTask(func);
}

bool ReclaimStrategyManager::HandleAppStateChanged(std::shared_ptr<ReclaimParam> reclaimPara)
{
    if (reclaimPara == nullptr) {
        HILOGE("reclaimPara nullptr");
        return false;
    }
    HILOGD("%{public}s", reclaimPara->ToString().c_str());
    bool ret = false;
    bool (ReclaimStrategyManager::*funcPtr)(std::shared_ptr<ReclaimParam>) = nullptr;
    switch (reclaimPara->action_) {
        case AppAction::CREATE_PROCESS_AND_APP:
        case AppAction::CREATE_PROCESS_ONLY: {
            funcPtr = &ReclaimStrategyManager::HandleProcessCreate;
            break;
        }
        case AppAction::APP_DIED:
        case AppAction::APP_FOREGROUND:
        case AppAction::APP_BACKGROUND:
        case AppAction::OTHERS: {
            HILOGD("OTHERS app action! %{public}d", reclaimPara->action_);
            break;
        }
        default:
            break;
    }
    if (funcPtr != nullptr) {
        ret = (this->*funcPtr)(reclaimPara);
    }
    reclaimPara.reset();
    return ret;
}

bool ReclaimStrategyManager::HandleProcessCreate(std::shared_ptr<ReclaimParam> reclaimPara)
{
    bool ret = memcgMgr_->AddProcToMemcg(std::to_string(reclaimPara->pid_), reclaimPara->accountId_);
    HILOGI("%{public}s. %{public}s", ret ? "succ" : "fail",  reclaimPara->ToString().c_str());
    return ret;
}

void ReclaimStrategyManager::NotifyAccountDied(int accountId)
{
    if (!Initailized()) {
        HILOGE("has not been initialized_, skiped!");
        return;
    }
    std::function<bool()> func = std::bind(
        &ReclaimStrategyManager::HandleAccountDied, this, accountId);
    handler_->PostImmediateTask(func);
}

void ReclaimStrategyManager::NotifyAccountPriorityChanged(int accountId, int priority)
{
    if (!Initailized()) {
        HILOGE("has not been initialized_, skiped!");
        return;
    }
    std::function<bool()> func = std::bind(
        &ReclaimStrategyManager::HandleAccountPriorityChanged, this, accountId, priority);
    handler_->PostImmediateTask(func);
}


bool ReclaimStrategyManager::HandleAccountDied(int accountId)
{
    return memcgMgr_->RemoveUserMemcg(accountId);
}

bool ReclaimStrategyManager::HandleAccountPriorityChanged(int accountId, int priority)
{
    ReclaimRatios* ratios = new ReclaimRatios();
    if (!GetReclaimRatiosByScore(priority, ratios)) {
        return false;
    }
    bool ret = memcgMgr_->UpdateMemcgScoreAndReclaimRatios(accountId, priority, ratios);
    HILOGI("update user reclaim retios %{public}s. userId=%{public}d score=%{public}d %{public}s",
           ret ? "succ" : "fail", accountId, priority, ratios->ToString().c_str());
    delete ratios;
    return ret;
}

bool ReclaimStrategyManager::GetReclaimRatiosByScore(int score, ReclaimRatios * const ratios)
{
    if (ratios == nullptr) {
        HILOGE("param ratios nullptr");
        return false;
    }
    HILOGD("before get ratios from MemmgrConfigManager %{public}s", ratios->NumsToString().c_str());
    MemmgrConfigManager::ReclaimRatiosConfigSet reclaimRatiosConfigSet =
        MemmgrConfigManager::GetInstance().GetReclaimRatiosConfigSet();
    for (auto i = reclaimRatiosConfigSet.begin(); i != reclaimRatiosConfigSet.end(); ++i) {
        if ((*i)->minScore <= score && (*i)->maxScore >= score) {
            HILOGI("get ratios from MemmgrConfigManager %{public}d %{public}d %{public}d",
                (*i)->mem2zramRatio, (*i)->zran2ufsRation, (*i)->refaultThreshold);
            ratios->SetRatios((*i)->mem2zramRatio, (*i)->zran2ufsRation, (*i)->refaultThreshold);
            return true;
        }
    }
    HILOGW("can not get ratios from MemmgrConfigManager"); // will using default para
    return true;
}
} // namespace Memory
} // namespace OHOS
