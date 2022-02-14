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
#include "multi_account_kill.h"
#include "default_multi_account_strategy.h"
#include "reclaim_strategy_manager.h"
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

void MultiAccountManager::Init()
{
    oldActiveAccountIds_.clear();
    AccountSA::OsAccountManager::QueryActiveOsAccountIds(oldActiveAccountIds_);
}

void MultiAccountManager::SetAccountPriority(int accountId, std::string accountName,
                                             AccountType accountType, bool isActived)
{
    std::shared_ptr<AccountPriorityInfo> accountInfo = GetAccountPriorityInfo(accountId);
    if (accountInfo == nullptr) {
        accountInfo = std::make_shared<AccountPriorityInfo>(accountId, accountName, accountType, isActived);
        AddAccountPriorityInfo(accountInfo);
    } else {
        accountInfo->SetName(accountName);
        accountInfo->SetType(accountType);
        accountInfo->SetIsActived(isActived);
    }

    int oldPriority = accountInfo->GetPriority();
    strategy_->SetAccountPriority(accountId);
    HILOGI("Set acccount priority success, accountId = %{public}d, old = %{public}d, new = %{public}d.",
           accountId, oldPriority, accountInfo->GetPriority());
    ReclaimStrategyManager::GetInstance().NotifyAccountPriorityChanged(accountId, accountInfo->GetPriority());
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

void MultiAccountManager::GetSwitchedAccountIds(std::vector<int> &accountIds)
{
    std::vector<int> newActiveAccountIds;
    AccountSA::OsAccountManager::QueryActiveOsAccountIds(newActiveAccountIds);
    for (unsigned int i = 0; i < oldActiveAccountIds_.size(); i++) {
        bool isExist = false;
        for (unsigned int j = 0; j < newActiveAccountIds.size(); j++) {
            if (oldActiveAccountIds_.at(i) == newActiveAccountIds.at(j)) {
                isExist = true;
                break;
            }
        }
        if (!isExist) {
            accountIds.push_back(oldActiveAccountIds_.at(i));
            HILOGI("Get switch account success, accountId = %{public}d.", oldActiveAccountIds_.at(i));
        }
    }

    oldActiveAccountIds_ = newActiveAccountIds;
}

void MultiAccountManager::UpdateAccountPriorityInfo(int accountId)
{
    AccountSA::OsAccountInfo osAccountInfo;
    AccountSA::OsAccountManager::QueryOsAccountById(accountId, osAccountInfo);
    SetAccountPriority(accountId, osAccountInfo.GetLocalName(), static_cast<AccountType>(osAccountInfo.GetType()),
                       osAccountInfo.GetIsActived());
}

void MultiAccountManager::GetAccountProcesses(int accountId, std::map<int, OsAccountPriorityInfo> &osAccountsInfoMap_,
                                              std::vector<pid_t> &processes)
{
    processes.clear();
    OsAccountPriorityInfo* accountPriorityInfo = &osAccountsInfoMap_.at(accountId);
    std::map<int, BundlePriorityInfo*>::iterator iter;
    for (iter = accountPriorityInfo->bundleIdInfoMapping_.begin();
         iter != accountPriorityInfo->bundleIdInfoMapping_.end(); iter++) {
        BundlePriorityInfo *bundleInfo = iter->second;
        std::map<pid_t, ProcessPriorityInfo>::iterator iter2;
        for (iter2 = bundleInfo->procs_.begin(); iter2 != bundleInfo->procs_.end(); iter2++) {
            processes.push_back(iter2->first);
        }
    }
}

bool MultiAccountManager::HandleAccountColdSwitch(std::vector<int> &switchedAccountIds,
                                                  std::map<int, OsAccountPriorityInfo> &osAccountsInfoMap_)
{
    for (unsigned int accountId = 0; accountId < switchedAccountIds.size(); accountId++) {
        HILOGI("Account cold switch account = %{public}d.", accountId);
        std::vector<pid_t> processes;
        GetAccountProcesses(accountId, osAccountsInfoMap_, processes);
        MultiAccountKill::GetInstance().KillAccountProccesses(processes);
        ReclaimStrategyManager::GetInstance().NotifyAccountDied(accountId);
    }
    return true;
}

bool MultiAccountManager::HandleAccountHotSwitch(std::vector<int> &switchedAccountIds,
                                                 std::map<int, OsAccountPriorityInfo> &osAccountsInfoMap_)
{
    for (unsigned int accountId = 0; accountId < switchedAccountIds.size(); accountId++) {
        OsAccountPriorityInfo* accountPriorityInfo = &osAccountsInfoMap_.at(accountId);
        std::map<int, BundlePriorityInfo*>::iterator iter;
        for (iter = accountPriorityInfo->bundleIdInfoMapping_.begin();
             iter != accountPriorityInfo->bundleIdInfoMapping_.end(); iter++) {
            BundlePriorityInfo *bundleInfo = iter->second;
            int oldPriority = bundleInfo->priority_;
            bundleInfo->priority_ = RecalcBundlePriority(accountId, oldPriority);
            HILOGI("Account hot switch account = %{public}d bundle = %{public}d old = %{public}d new = %{public}d.",
                   accountId, iter->first, oldPriority, bundleInfo->priority_);
        }
    }
    return true;
}

bool MultiAccountManager::HandleOsAccountsChanged(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod,
                                                  std::map<int, OsAccountPriorityInfo> &osAccountsInfoMap_)
{
    std::vector<int> switchedAccountIds;
    GetSwitchedAccountIds(switchedAccountIds);

    /* Account info update */
    UpdateAccountPriorityInfo(accountId);
    for (unsigned int id = 0; id < switchedAccountIds.size(); id++) {
        UpdateAccountPriorityInfo(id);
    }

    /* Handle different switch mode */
    switch (switchMod) {
        case AccountSA::COLD_SWITCH:
            return HandleAccountColdSwitch(switchedAccountIds, osAccountsInfoMap_);
        case AccountSA::HOT_SWITCH:
            return HandleAccountHotSwitch(switchedAccountIds, osAccountsInfoMap_);
        default:
            HILOGI("Switch mode incorrect, mode = %{public}d.", static_cast<int>(switchMod));
            return false;
    }
}
} // namespace Memory
} // namespace OHOS
