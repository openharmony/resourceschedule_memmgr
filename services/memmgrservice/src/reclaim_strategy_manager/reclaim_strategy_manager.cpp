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
#include "reclaim_strategy_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "ReclaimStrategyManager";
const int DEFAULT_MEM2ZRAM_RATIO = 40; // default mem2zram ratio 40%
const int DEFAULT_ZRAN2UFS_RATIO = 0; // default zran2ufs ratio 0%
const int DEFAULT_REFAULT_THRESHOLD = 0; // default refault threshold 0%
}

IMPLEMENT_SINGLE_INSTANCE(ReclaimStrategyManager);

ReclaimStrategyManager::ReclaimStrategyManager()
{
}

ReclaimStrategyManager::~ReclaimStrategyManager()
{
}

bool ReclaimStrategyManager::Init()
{
    initialized_ = GetEventHandler();
    if (initialized_) {
        HILOGI("init successed");
    } else {
        HILOGE("init failed");
    }

    MemmgrConfigManager::GetInstance().Init();
    AvailBufferManager::GetInstance().Init();
    InitMemcgReclaimRatios();
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
    HILOGD("%{public}s pid=%{public}d uid=%{public}d userId=%{public}d score=%{public}d",
           reclaimPara->bundleName.c_str(), reclaimPara->pid, reclaimPara->bundleUid, reclaimPara->accountId,
           reclaimPara->appScore);
    bool ret = false;
    bool (ReclaimStrategyManager::*funcPtr)(std::shared_ptr<ReclaimParam>) = nullptr;
    switch (reclaimPara->action) {
        case AppAction::CREATE_PROCESS_AND_APP:
        case AppAction::CREATE_PROCESS_ONLY: {
            funcPtr = &ReclaimStrategyManager::HandleProcessCreate;
            break;
        }
        case AppAction::APP_DIED:
        case AppAction::APP_FOREGROUND:
        case AppAction::APP_BACKGROUND:
        case AppAction::OTHERS:
            HILOGI("OTHERS app action! %{public}d", reclaimPara->action);
            break;
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
    HILOGD("%{public}s pid=%{public}d uid=%{public}d userId=%{public}d",
           reclaimPara->bundleName.c_str(), reclaimPara->pid, reclaimPara->bundleUid, reclaimPara->accountId);
    memcg.AddProc(reclaimPara->pid); // add pid to memcg
    return true;
}

bool ReclaimStrategyManager::GetReclaimRatiosByAppScore(int score, ReclaimRatios* ratios)
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
            HILOGI("get ratios from MemmgrConfigManager %{public}d %{public}d %{public}d ",
                (*i)->mem2zramRatio, (*i)->zran2ufsRation, (*i)->refaultThreshold);
            ratios->UpdateReclaimRatios((*i)->mem2zramRatio, (*i)->zran2ufsRation, (*i)->refaultThreshold);
            return true;
        }
    }
    HILOGW("can not get ratios from MemmgrConfigManager");
    ratios->UpdateReclaimRatios(DEFAULT_MEM2ZRAM_RATIO, DEFAULT_ZRAN2UFS_RATIO, DEFAULT_REFAULT_THRESHOLD);
    return true;
}

bool ReclaimStrategyManager::InitMemcgReclaimRatios()
{
    memcg.SetScoreToKernel(300); // default score 300
    if (GetReclaimRatiosByAppScore(300, memcg.reclaimRatios)) {
        memcg.SetRatiosToKernel();
    }
    HILOGD("Init reclaim retios success");
    return true;
}

void ReclaimStrategyManager::UpdateMemcgReclaimInfo()
{
    memcg.UpdateSwapInfoFromKernel();
}
} // namespace Memory
} // namespace OHOS
