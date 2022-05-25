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
#include "memmgr_ptr_util.h"
#include "default_multi_account_strategy.h"
#include "reclaim_strategy_manager.h"
#include "kernel_interface.h"
#include "oom_score_adj_utils.h"
#include "reclaim_priority_constants.h"
#include "multi_account_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MultiAccountManager";
const int MAX_RETRY_TIMES = 10;
const int SLEEP_TIME = 3000;
}

IMPLEMENT_SINGLE_INSTANCE(MultiAccountManager);

MultiAccountManager::MultiAccountManager()
{
    MAKE_POINTER(strategy_, shared, DefaultMultiAccountStrategy, "make shared failed", return, /* no param */);
    MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return,
        AppExecFwk::EventRunner::Create());
}

MultiAccountManager::~MultiAccountManager()
{
    if (strategy_) {
        strategy_ = nullptr;
    }
}

void MultiAccountManager::Init()
{
    retryTimes_++;
    initialized_ = false;
    do {
        oldActiveAccountIds_.clear();
        ErrCode errCode = AccountSA::OsAccountManager::QueryActiveOsAccountIds(oldActiveAccountIds_);
        if (errCode != ERR_OK) {
            HILOGI("The manager initial failed, err = %{public}d.", static_cast<int>(errCode));
            break;
        }

        if (!UpdateAccountPriorityInfo(oldActiveAccountIds_)) {
            HILOGI("The manager initial failed.");
            break;
        }

        initialized_ = true;
    } while (0);

    if (initialized_) {
        HILOGI("The manager initial succeed, accountCount = %{public}zu.", oldActiveAccountIds_.size());
        return;
    }

    if (retryTimes_ < MAX_RETRY_TIMES) {
        if (handler_ == nullptr) {
            HILOGE("The manager initial failed and couldn't retry, because the handler is null.");
            return;
        }

        std::function<void()> initMultiAccountManagerFunc = std::bind(&MultiAccountManager::Init, this);
        handler_->PostTask(initMultiAccountManagerFunc, SLEEP_TIME, AppExecFwk::EventQueue::Priority::LOW);
        HILOGE("Manager initial failed, try again after 3s!, retryTimes = %{public}d/10", retryTimes_);
    }
}

bool MultiAccountManager::SetAccountPriority(int accountId, std::string accountName,
                                             AccountSA::OsAccountType accountType, bool isActived)
{
    std::shared_ptr<AccountPriorityInfo> accountInfo = GetAccountPriorityInfo(accountId);
    if (accountInfo == nullptr) {
        MAKE_POINTER(accountInfo, shared, AccountPriorityInfo, "make shared failed", return false,
            accountId, accountName, accountType, isActived);
        AddAccountPriorityInfo(accountInfo);
    } else {
        accountInfo->SetName(accountName);
        accountInfo->SetType(accountType);
        accountInfo->SetIsActived(isActived);
    }

    if (strategy_ == nullptr) {
        HILOGI("Set account priority failed, strategy is null.");
        return false;
    }

    int oldPriority = accountInfo->GetPriority();
    if (!strategy_->SetAccountPriority(accountInfo)) {
        HILOGI("Set account priority failed, accountId = %{public}d.", accountId);
        return false;
    }

    HILOGI("Set acccount priority succeed, accountId = %{public}d, old = %{public}d, new = %{public}d.",
           accountId, oldPriority, accountInfo->GetPriority());
    ReclaimStrategyManager::GetInstance().NotifyAccountPriorityChanged(accountId, accountInfo->GetPriority());
    return true;
}

int MultiAccountManager::RecalcBundlePriority(int accountId, int bundlePriority)
{
    std::shared_ptr<AccountPriorityInfo> accountInfo = GetAccountPriorityInfo(accountId);
    if (accountInfo == nullptr) {
        HILOGI("Repeat calculate bundle priority failed, account non-exist, accountId = %{public}d.", accountId);
        return RECLAIM_PRIORITY_MAX;
    }

    if (strategy_ == nullptr) {
        HILOGI("Repeat calculate bundle priority failed, strategy is null.");
        return RECLAIM_PRIORITY_MAX;
    }

    int recalcPriority = strategy_->RecalcBundlePriority(accountInfo, bundlePriority);
    if (recalcPriority > RECLAIM_PRIORITY_MAX) {
        recalcPriority = RECLAIM_PRIORITY_MAX;
    }
    return recalcPriority;
}

void MultiAccountManager::AddAccountPriorityInfo(std::shared_ptr<AccountPriorityInfo> accountPriorityInfo)
{
    accountMap_.insert(std::pair<int, std::shared_ptr<AccountPriorityInfo>>(accountPriorityInfo->GetId(),
                                                                            accountPriorityInfo));
    HILOGI("Add account information succeed, accountId = %{public}d.", accountPriorityInfo->GetId());
}

std::shared_ptr<AccountPriorityInfo> MultiAccountManager::GetAccountPriorityInfo(int accountId)
{
    std::map<int, std::shared_ptr<AccountPriorityInfo>>::iterator iter = accountMap_.find(accountId);
    if (iter != accountMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<MultiAccountStrategy> MultiAccountManager::GetMultiAccountStratgy()
{
    return strategy_;
}

bool MultiAccountManager::SetMultiAccountStrategy(std::shared_ptr<MultiAccountStrategy> strategy)
{
    if (strategy == nullptr) {
        HILOGI("Set the multiple account strategy failed because the strategy is null.");
        return false;
    }

    strategy_ = strategy;
    return true;
}

bool MultiAccountManager::GetSwitchedAccountIds(std::vector<int> &accountIds)
{
    std::vector<int> newActiveAccountIds;
    ErrCode errCode = AccountSA::OsAccountManager::QueryActiveOsAccountIds(newActiveAccountIds);
    if (errCode != ERR_OK) {
        HILOGI("Query active os accountIds failed, err = %{public}d.", static_cast<int>(errCode));
        return false;
    }

    for (int oldId : oldActiveAccountIds_) {
        if (std::find(newActiveAccountIds.begin(), newActiveAccountIds.end(), oldId) == newActiveAccountIds.end()) {
            accountIds.push_back(oldId);
            HILOGI("Get the switched account succeed, accountId = %{public}d.", oldId);
        }
    }

    oldActiveAccountIds_ = newActiveAccountIds;
    return true;
}

bool MultiAccountManager::UpdateAccountPriorityInfo(std::vector<int> &accountIds)
{
    for (int accountId : accountIds) {
        AccountSA::OsAccountInfo osAccountInfo;
        ErrCode errCode = AccountSA::OsAccountManager::QueryOsAccountById(accountId, osAccountInfo);
        if (errCode != ERR_OK) {
            HILOGI("Get os account failed, accountId = %{public}d, err = %{public}d.",
                   accountId, static_cast<int>(errCode));
            return false;
        }
        if (!SetAccountPriority(accountId, osAccountInfo.GetLocalName(), osAccountInfo.GetType(),
                                osAccountInfo.GetIsActived())) {
            HILOGI("Set account priority failed, accountId = %{public}d.", accountId);
            return false;
        }
    }
    return true;
}

std::shared_ptr<AccountBundleInfo> MultiAccountManager::GetAccountBundleInfo(int accountId,
    std::map<int, std::shared_ptr<AccountBundleInfo>> &osAccountsInfoMap_)
{
    std::map<int, std::shared_ptr<AccountBundleInfo>>::iterator iter = osAccountsInfoMap_.find(accountId);
    if (iter != osAccountsInfoMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

void MultiAccountManager::KillProcessesOfAccount(int accountId,
    std::map<int, std::shared_ptr<AccountBundleInfo>> &osAccountsInfoMap_)
{
    std::shared_ptr<AccountBundleInfo> accountBundleInfo = GetAccountBundleInfo(accountId, osAccountsInfoMap_);
    if (accountBundleInfo == nullptr) {
        HILOGI("Kill proccess of the account failed because the account non-exist, accountId = %{public}d.", accountId);
        return;
    }

    for (auto iter1 : accountBundleInfo->bundleIdInfoMapping_) {
        for (auto iter2 : iter1.second->procs_) {
            pid_t pid = iter2.first;
            if (!KernelInterface::GetInstance().KillOneProcessByPid(pid)) {
                HILOGI("Kill the process failed, pid = %{public}d.", pid);
                continue;
            }
            HILOGI("Kill the process succeed, pid = %{public}d.", pid);
        }
    }
}

bool MultiAccountManager::HandleAccountColdSwitch(std::vector<int> &switchedAccountIds,
    std::map<int, std::shared_ptr<AccountBundleInfo>> &osAccountsInfoMap_)
{
    for (int accountId : switchedAccountIds) {
        HILOGI("Account cold switch account = %{public}d.", accountId);
        KillProcessesOfAccount(accountId, osAccountsInfoMap_);
        ReclaimStrategyManager::GetInstance().NotifyAccountDied(accountId);
    }
    return true;
}

bool MultiAccountManager::HandleAccountHotSwitch(std::vector<int> &switchedAccountIds,
    std::map<int, std::shared_ptr<AccountBundleInfo>> &osAccountsInfoMap_)
{
    for (int accountId : switchedAccountIds) {
        std::shared_ptr<AccountBundleInfo> accountBundleInfo = GetAccountBundleInfo(accountId, osAccountsInfoMap_);
        if (accountBundleInfo == nullptr) {
            HILOGI("Search account bundle info failed, accountId = %{public}d.", accountId);
            continue;
        }

        for (auto iter : accountBundleInfo->bundleIdInfoMapping_) {
            std::shared_ptr<BundlePriorityInfo> bundleInfo = iter.second;
            int oldPriority = bundleInfo->priority_;
            bundleInfo->priority_ = RecalcBundlePriority(accountId, oldPriority);
            HILOGI("Account hot switch account = %{public}d bundle = %{public}d old = %{public}d new = %{public}d.",
                   accountId, iter.first, oldPriority, bundleInfo->priority_);
            bundleInfo->IncreaseProcsPriority(bundleInfo->priority_ - oldPriority);
            OomScoreAdjUtils::WriteOomScoreAdjToKernel(bundleInfo);
        }
    }
    return true;
}

bool MultiAccountManager::HandleOsAccountsChanged(int accountId, AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod,
                                                  std::map<int, std::shared_ptr<AccountBundleInfo>> &osAccountsInfoMap_)
{
    if (!initialized_) {
        HILOGI("The MultiAccountManager uninitialized.");
        return false;
    }

    std::vector<int> switchedAccountIds;
    if (!GetSwitchedAccountIds(switchedAccountIds)) {
        HILOGI("Get switched accountIds failed.");
        return false;
    }

    std::vector<int> updateAccountIds = switchedAccountIds;
    updateAccountIds.push_back(accountId);
    if (!UpdateAccountPriorityInfo(updateAccountIds)) {
        HILOGI("Update account priority information failed.");
        return false;
    }

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
