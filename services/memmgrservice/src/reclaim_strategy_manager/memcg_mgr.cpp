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

IMPLEMENT_SINGLE_INSTANCE(MemcgMgr);

MemcgMgr::MemcgMgr()
{
    rootMemcg_ = new (std::nothrow) Memcg();
    if (rootMemcg_ == nullptr) {
        HILOGE("new obj failed!");
    }
}

MemcgMgr::~MemcgMgr()
{
    delete rootMemcg_;
    rootMemcg_ = nullptr;
    while (!userMemcgsMap_.empty()) {
        auto iter = userMemcgsMap_.begin();
        delete iter->second;
        iter->second = nullptr;
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
    bool ret = rootMemcg_->SetScoreAndReclaimRatiosToKernel();
    HILOGI("Init rootMemcg reclaim retios success");
    return ret;
}

UserMemcg* MemcgMgr::GetUserMemcg(unsigned int userId)
{
    std::map<int, UserMemcg*>::iterator it = userMemcgsMap_.find(userId);
    if (it == userMemcgsMap_.end()) {
        return nullptr;
    }
    return it->second;
}

UserMemcg* MemcgMgr::AddUserMemcg(unsigned int userId)
{
    HILOGI("userId=%{public}d", userId);
    UserMemcg* memcg = new (std::nothrow) UserMemcg(userId);
    if (memcg == nullptr) {
        HILOGE("new obj failed!");
        return nullptr;
    }
    userMemcgsMap_.insert(std::make_pair(userId, memcg));
    memcg->CreateMemcgDir();
    return memcg;
}

bool MemcgMgr::RemoveUserMemcg(unsigned int userId)
{
    HILOGI("userId=%{public}d", userId);
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) {
        HILOGI("account %{public}d not exist. cannot remove", userId);
        return false;
    }
    memcg->RemoveMemcgDir();
    userMemcgsMap_.erase(userId);
    delete memcg;
    memcg = nullptr;
    return GetUserMemcg(userId) == nullptr;
}

bool MemcgMgr::UpdateMemcgScoreAndReclaimRatios(unsigned int userId, int score, ReclaimRatios * const ratios)
{
    if (ratios == nullptr) {
        HILOGI("parma ratios nullptr. userId=%{public}d score=%{public}d", userId, score);
        return false;
    }
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

bool MemcgMgr::AddProcToMemcg(unsigned int pid, unsigned int userId)
{
    HILOGI("pid=%{public}d userId=%{public}d", pid, userId);
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) { // new user
        memcg = AddUserMemcg(userId);
    }
    if (memcg == nullptr) {
        HILOGE("AddUserMemcg failed %{public}d", userId);
        return false;
    }
    return memcg->AddProc(pid); // add pid to memcg
}

bool MemcgMgr::SwapInMemcg(unsigned int userId)
{
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) { // no such user
        return false;
    }
    return memcg->SwapIn();
}

SwapInfo* MemcgMgr::GetMemcgSwapInfo(unsigned int userId)
{
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) { // no such user
        return nullptr;
    }
    memcg->UpdateSwapInfoFromKernel();
    return memcg->swapInfo_;
}

MemInfo* MemcgMgr::GetMemcgMemInfo(unsigned int userId)
{
    UserMemcg* memcg = GetUserMemcg(userId);
    if (memcg == nullptr) { // no such user
        return nullptr;
    }
    memcg->UpdateMemInfoFromKernel();
    return memcg->memInfo_;
}
} // namespace Memory
} // namespace OHOS
