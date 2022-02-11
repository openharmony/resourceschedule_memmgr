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
    rootMemcg_ = new Memcg();
}

ReclaimStrategyManager::~ReclaimStrategyManager()
{
    delete rootMemcg_;
    while (!userMemcgsMap_.empty()) {
        auto iter = userMemcgsMap_.begin();
        delete iter->second;
        userMemcgsMap_.erase(iter);
    }
}

bool ReclaimStrategyManager::Init()
{
    initialized_ = GetEventHandler();
    initialized_ = initialized_ && SetRootMemcgPara();
    if (initialized_) {
        HILOGI("init successed");
    } else {
        HILOGE("init failed");
    }

    MemmgrConfigManager::GetInstance().Init();
    AvailBufferManager::GetInstance().Init();
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

    UpdateMemcgReclaimInfo();

    reclaimPara.reset();
    return ret;
}

bool ReclaimStrategyManager::HandleProcessCreate(std::shared_ptr<ReclaimParam> reclaimPara)
{
    HILOGI("%{public}s", reclaimPara->ToString().c_str());
    UserMemcg* memcg = UserMemcgsGet(reclaimPara->accountId_);
    if (memcg == nullptr) { // new user
        memcg = UserMemcgsAdd(reclaimPara->accountId_);
        memcg->CreateMemcgDir();
    }
    memcg->AddProc(std::to_string(reclaimPara->pid_)); // add pid to memcg
    return true;
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
    HILOGW("can not get ratios from MemmgrConfigManager");
    return true;
}

bool ReclaimStrategyManager::SetRootMemcgPara()
{
    if (!rootMemcg_ || !rootMemcg_->reclaimRatios_) {
        HILOGE("rootMemcg nullptr");
        return false;
    }
    rootMemcg_->SetScoreToKernel(APP_SCORE);
    rootMemcg_->SetReclaimRatios(ROOT_MEMCG_MEM_2_ZRAM_RATIO,
        ROOT_MEMCG_ZRAM_2_UFS_RATIO, ROOT_MEMCG_REFAULT_THRESHOLD);
    rootMemcg_->SetReclaimRatiosToKernel();
    HILOGI("Init rootMemcg reclaim retios success");
    return true;
}

void ReclaimStrategyManager::UpdateMemcgReclaimInfo()
{
    rootMemcg_->UpdateSwapInfoFromKernel();
}

UserMemcg* ReclaimStrategyManager::UserMemcgsAdd(int userId)
{
    HILOGD("userId=%{public}d", userId);
    UserMemcg* memcg = new UserMemcg(userId);
    userMemcgsMap_.insert(std::make_pair(userId, memcg));
    return memcg;
}

UserMemcg* ReclaimStrategyManager::UserMemcgsRemove(int userId)
{
    HILOGD("userId=%{public}d", userId);
    UserMemcg* memcg = UserMemcgsGet(userId);
    userMemcgsMap_.erase(userId);
    return memcg;
}

UserMemcg* ReclaimStrategyManager::UserMemcgsGet(int userId)
{
    std::map<int, UserMemcg*>::iterator it = userMemcgsMap_.find(userId);
    if (it == userMemcgsMap_.end()) {
        return nullptr;
    }
    return it->second;
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
    HILOGD("userId=%{public}d", accountId);
    UserMemcg* memcg = UserMemcgsRemove(accountId);
    if (memcg == nullptr) {
        HILOGI("account %{public}d not exist. not need to remove", accountId);
        return true;
    }
    memcg->RemoveMemcgDir();
    delete memcg;
    memcg = nullptr;
    return true;
}

bool ReclaimStrategyManager::HandleAccountPriorityChanged(int accountId, int priority)
{
    UserMemcg* memcg = UserMemcgsGet(accountId);
    if (memcg == nullptr) {
        HILOGI("account %{public}d not exist. cannot update ratios", accountId);
        return false;
    }
    HILOGI("update reclaim retios userId=%{public}d priority=%{public}d", accountId, priority);
    memcg->SetScoreToKernel(priority);
    if (GetReclaimRatiosByScore(priority, memcg->reclaimRatios_)) {
        memcg->SetReclaimRatiosToKernel();
    }
    return true;
}
} // namespace Memory
} // namespace OHOS
