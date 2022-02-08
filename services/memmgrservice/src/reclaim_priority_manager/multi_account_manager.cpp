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

#include "memmgr_log.h"
#include "default_multi_account_strategy.h"
#include "multi_account_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MultiAccountManager";
}

IMPLEMENT_SINGLE_INSTANCE(MultiAccountManager);

MultiAccountManager::MultiAccountManager()
{
    strategy_ = std::make_shared<DefaultMultiAccountStrategy>();
}

MultiAccountManager::~MultiAccountManager()
{
    if (strategy_) {
        strategy_ = nullptr;
    }
}

void MultiAccountManager::SetAccountPriority(int accountId, std::string accountName,
                                             AccountType accountType, bool isActived)
{
    std::shared_ptr<AccountPriorityInfo> accountInfo = GetAccountPriorityInfo(accountId);
    if (accountInfo == nullptr) {
        AddAccountPriorityInfo(std::make_shared<AccountPriorityInfo>(accountId, accountName, accountType, isActived));
    } else {
        accountInfo->SetName(accountName);
        accountInfo->SetType(accountType);
        accountInfo->SetIsActived(isActived);
    }

    int oldPriority = accountInfo->GetPriority();
    strategy_->SetAccountPriority(accountId);
    HILOGI("Set acccount priority success, accountId = %{public}d, old = %{public}d, new = %{public}d.",
           accountId, oldPriority, accountInfo->GetPriority());
}

int MultiAccountManager::RecalcBundlePriority(int accountId, int bundlePriority)
{
    if (GetAccountPriorityInfo(accountId) == nullptr) {
        HILOGI("Repeat calculate bundle priority fail, account non-exist, accountId = %{public}d.", accountId);
        return -1;
    }

    int recalcPriority = strategy_->RecalcBundlePriority(accountId, bundlePriority);
    HILOGI("Repeat calculate bundle priority success, accountId = %{public}d, old = %{public}d, new = %{public}d.",
           accountId, bundlePriority, recalcPriority);
    return recalcPriority;
}

void MultiAccountManager::AddAccountPriorityInfo(std::shared_ptr<AccountPriorityInfo> accountPriorityInfo)
{
    accountMap_.insert(std::pair<int, std::shared_ptr<AccountPriorityInfo>>(accountPriorityInfo->GetId(),
                                                                            accountPriorityInfo));
    HILOGI("Add account information success, accountId = %{public}d.", accountPriorityInfo->GetId());
}

std::shared_ptr<AccountPriorityInfo> MultiAccountManager::GetAccountPriorityInfo(int accountId)
{
    std::map<int, std::shared_ptr<AccountPriorityInfo>>::iterator iter = accountMap_.find(accountId);
    if (iter != accountMap_.end()) {
        return iter->second;
    }

    HILOGI("Get account information failed, accountId = %{public}d.", accountId);
    return nullptr;
}

std::shared_ptr<MultiAccountStrategy> MultiAccountManager::GetMultiAccountStratgy()
{
    return strategy_;
}

void MultiAccountManager::SetMultiAccountStrategy(std::shared_ptr<MultiAccountStrategy> strategy)
{
    strategy_ = strategy;
}
} // namespace Memory
} // namespace OHOS