/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this->file except in compliance with the License.
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
#include "reclaim_strategy_constants.h"
#include "memcg_mgr.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemcgMgr";
} // namespace
MemcgMgr::MemcgMgr()
{
    rootMemcg_ = new Memcg();
}

MemcgMgr::~MemcgMgr()
{
    delete rootMemcg_;
    while (!userMemcgsMap_.empty()) {
        auto iter = userMemcgsMap_.begin();
        delete iter->second;
        userMemcgsMap_.erase(iter);
    }
}

Memcg* MemcgMgr::GetRootMemcg() const
{
    return rootMemcg_;
}

bool MemcgMgr::SetRootMemcgPara()
{
    if (!rootMemcg_ || !rootMemcg_->reclaimRatios_) {
        HILOGE("rootMemcg nullptr");
        return false;
    }
    rootMemcg_->SetScore(APP_SCORE);
    rootMemcg_->SetReclaimRatios(ROOT_MEMCG_MEM_2_ZRAM_RATIO,
        ROOT_MEMCG_ZRAM_2_UFS_RATIO, ROOT_MEMCG_REFAULT_THRESHOLD);
    rootMemcg_->SetScoreAndReclaimRatiosToKernel();
    HILOGI("Init rootMemcg reclaim retios success");
    return true;
}

UserMemcg* MemcgMgr::GetUserMemcg(int userId)
{
    std::map<int, UserMemcg*>::iterator it = userMemcgsMap_.find(userId);
    if (it == userMemcgsMap_.end()) {
        return nullptr;
    }
    return it->second;
}

UserMemcg* MemcgMgr::AddUserMemcg(int userId)
{
    HILOGI("userId=%{public}d", userId);
    UserMemcg* memcg = new UserMemcg(userId);
    userMemcgsMap_.insert(std::make_pair(userId, memcg));
    return memcg;
}

bool MemcgMgr::RemoveUserMemcg(int userId)
{
    HILOGI("userId=%{public}d", userId);
    UserMemcg* memcg = GetUserMemcg(userId);
    userMemcgsMap_.erase(userId);
    delete memcg;
    memcg = nullptr;
    return GetUserMemcg(userId) == nullptr;
}

bool MemcgMgr::UpdateMemcgScoreAndReclaimRatios(int userId, int score, ReclaimRatios * const ratios)
{
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) {
        HILOGI("account %{public}d not exist. cannot update score and ratios", userId);
        return false;
    }
    HILOGI("update reclaim retios userId=%{public}d score=%{public}d, %{public}s",
           userId, score, ratios->ToString().c_str());
    memcg->SetScore(score);
    return memcg->SetReclaimRatios(ratios) && memcg->SetScoreAndReclaimRatiosToKernel();
}

bool MemcgMgr::AddProcToMemcg(const std::string& pid, int userId)
{
    HILOGI("pid=%{public}s userId=%{public}d", pid.c_str(), userId);
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) { // new user
        memcg = AddUserMemcg(userId);
        memcg->CreateMemcgDir();
    }
    memcg->AddProc(pid); // add pid to memcg
    return true;
}

bool MemcgMgr::SwapInMemcg(int userId)
{
    HILOGI("userId=%{public}d", userId);
    return true;
}

SwapInfo* MemcgMgr::GetMemcgSwapInfo(int userId)
{
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) { // no such user
        return nullptr;
    }
    memcg->UpdateSwapInfoFromKernel();
    return memcg->swapInfo_;
}

MemInfo* MemcgMgr::GetMemcgMemInfo(int userId)
{
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) { // no such user
        return nullptr;
    }
    memcg->UpdateMemInfoFromKernel();
    return memcg->memInfo_;
}

bool MemcgMgr::MemcgSwapIn(int userId)
{
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) { // no such user
        return false;
    }
    return memcg->SwapIn();
}
} // namespace Memory
} // namespace OHOS
