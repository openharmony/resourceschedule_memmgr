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
#include "memmgr_config_manager.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "reclaim_priority_constants.h"
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
        if (!GetEventHandler_()) {
            break;
        }
        AvailBufferManager::GetInstance().Init();
        if (!MemcgMgr::GetInstance().SetRootMemcgPara()) {
            break;
        }
        initialized_ = true;
    } while (0);

    if (!initialized_) {
        HILOGE("init failed");
        return false;
    }
    InitProcessBeforeMemmgr(); // add the process (which started before memmgr) to memcg
    HILOGI("init success");
    return initialized_;
}

bool ReclaimStrategyManager::GetEventHandler_()
{
    if (handler_ == nullptr) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

void ReclaimStrategyManager::InitProcessBeforeMemmgr()
{
    std::vector<unsigned int> pids;
    if (!KernelInterface::GetInstance().GetAllProcPids(pids)) {
        HILOGI("GetAllProcPids failed");
        return;
    }
    unsigned int uid =  0;
    unsigned int userId = 0;
    bool ret = false;
    for (auto pid : pids) {
        if (!KernelInterface::GetInstance().GetUidByPid(pid, uid)) {
            continue;
        }
        userId = GET_OS_ACCOUNT_ID_BY_UID(uid);
        if (userId < VALID_USER_ID_MIN) { // invalid userId
            continue;
        }
        ret = MemcgMgr::GetInstance().AddProcToMemcg(pid, userId);
        HILOGD("add pid=%{public}u to userId=%{public}u, %{public}s", pid, userId, ret ? "succ" : "fail");
    }
}

void ReclaimStrategyManager::NotifyAppStateChanged(std::shared_ptr<ReclaimParam> reclaimPara)
{
    if (!Initailized()) {
        HILOGE("has not been initialized_, skiped!");
        return;
    }
    std::function<bool()> func = std::bind(
        &ReclaimStrategyManager::HandleAppStateChanged_, this, reclaimPara);
    handler_->PostImmediateTask(func);
}

bool ReclaimStrategyManager::HandleAppStateChanged_(std::shared_ptr<ReclaimParam> reclaimPara)
{
    if (reclaimPara == nullptr) {
        HILOGE("reclaimPara nullptr");
        return false;
    }
    if (reclaimPara->accountId_ < VALID_USER_ID_MIN) {
        HILOGE("invalid userId %{public}d, less than MIN_VALUE(%{public}d)",
               reclaimPara->accountId_, VALID_USER_ID_MIN);
        return false;
    }
    HILOGI("%{public}s", reclaimPara->ToString().c_str());
    bool ret = false;
    bool (ReclaimStrategyManager::*funcPtr)(std::shared_ptr<ReclaimParam>) = nullptr;
    switch (reclaimPara->action_) {
        case AppAction::CREATE_PROCESS_AND_APP:
        case AppAction::CREATE_PROCESS_ONLY: {
            funcPtr = &ReclaimStrategyManager::HandleProcessCreate_;
            break;
        }
        case AppAction::APP_DIED:
        case AppAction::APP_FOREGROUND:
        case AppAction::APP_BACKGROUND:
        case AppAction::OTHERS: {
            HILOGI("others app action! %{public}s", ReclaimParam::GetAppActionStr(reclaimPara->action_).c_str());
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

bool ReclaimStrategyManager::HandleProcessCreate_(std::shared_ptr<ReclaimParam> reclaimPara)
{
    bool ret = MemcgMgr::GetInstance().AddProcToMemcg(reclaimPara->pid_, reclaimPara->accountId_);
    HILOGI("%{public}s, %{public}s", ret ? "succ" : "fail",  reclaimPara->ToString().c_str());
    return ret;
}

void ReclaimStrategyManager::NotifyAccountDied(int accountId)
{
    if (!Initailized()) {
        HILOGE("has not been initialized, skiped! accountId=%{public}d", accountId);
        return;
    }
    if (accountId < VALID_USER_ID_MIN) {
        HILOGE("invalid userId %{public}d, less than MIN_VALUE(%{public}d)",
               accountId, VALID_USER_ID_MIN);
        return;
    }
    std::function<bool()> func = std::bind(
        &ReclaimStrategyManager::HandleAccountDied_, this, accountId);
    handler_->PostImmediateTask(func);
}

void ReclaimStrategyManager::NotifyAccountPriorityChanged(int accountId, int priority)
{
    if (!Initailized()) {
        HILOGE("has not been initialized, skiped! accountId=%{public}d, priority=%{public}d", accountId, priority);
        return;
    }
    if (accountId < VALID_USER_ID_MIN) {
        HILOGE("invalid userId %{public}d, less than MIN_VALUE(%{public}d)",
               accountId, VALID_USER_ID_MIN);
        return;
    }
    std::function<bool()> func = std::bind(
        &ReclaimStrategyManager::HandleAccountPriorityChanged_, this, accountId, priority);
    handler_->PostImmediateTask(func);
}

bool ReclaimStrategyManager::HandleAccountDied_(int accountId)
{
    return MemcgMgr::GetInstance().RemoveUserMemcg(accountId);
}

bool ReclaimStrategyManager::HandleAccountPriorityChanged_(int accountId, int priority)
{
    if (MemcgMgr::GetInstance().GetUserMemcg(accountId) == nullptr) {
        HILOGI("account %{public}d not exist.", accountId);
        return false;
    }
    GetValidScore_(priority);
    std::unique_ptr<ReclaimRatios> ratios = nullptr;
    try {
        ratios = std::make_unique<ReclaimRatios>();
    } catch (...) {
        HILOGE("new ratios obj failed!");
        return false;
    }
    if (ratios == nullptr || !GetReclaimRatiosByScore_(priority, *ratios)) {
        HILOGE("get config ratios failed, will not update memcg ratio, userId=%{public}d", accountId);
        ratios = nullptr;
        return false;
    }
    bool ret = MemcgMgr::GetInstance().UpdateMemcgScoreAndReclaimRatios(accountId, priority, *ratios);
    HILOGI("UpdateMemcgScoreAndReclaimRatios %{public}s, userId=%{public}d score=%{public}d %{public}s",
           ret ? "succ" : "fail", accountId, priority, ratios->ToString().c_str());
    ratios = nullptr;
    return ret;
}

bool ReclaimStrategyManager::GetReclaimRatiosByScore_(int score, ReclaimRatios& ratios)
{
    HILOGD("before get ratios from MemmgrConfigManager %{public}s", ratios.NumsToString().c_str());
    MemmgrConfigManager::ReclaimRatiosConfigSet reclaimRatiosConfigSet =
        MemmgrConfigManager::GetInstance().GetReclaimRatiosConfigSet();
    for (auto i = reclaimRatiosConfigSet.begin(); i != reclaimRatiosConfigSet.end(); ++i) {
        if ((*i)->minScore <= score && (*i)->maxScore >= score) {
            ratios.SetRatiosByValue((*i)->mem2zramRatio, (*i)->zram2ufsRatio, (*i)->refaultThreshold);
            HILOGI("get ratios from MemmgrConfigManager %{public}s", ratios.NumsToString().c_str());
            return true;
        }
    }
    HILOGD("can not get ratios from MemmgrConfigManager");
    return false;
}

void ReclaimStrategyManager::GetValidScore_(int& priority)
{
    if (priority < RECLAIM_SCORE_MIN) {
        priority = RECLAIM_SCORE_MIN;
    } else if (priority > RECLAIM_SCORE_MAX) {
        priority = RECLAIM_SCORE_MAX;
    }
}
} // namespace Memory
} // namespace OHOS
