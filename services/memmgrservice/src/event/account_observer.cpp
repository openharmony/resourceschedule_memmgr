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

#include "account_observer.h"
#include "memmgr_log.h"

#include "os_account_manager.h"
#include "os_account_info.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "AccountObserver";
}

AccountObserver::AccountObserver(const AccountCallback &callback) : callback_(callback)
{
    HILOGI("called");

    AccountSA::OsAccountInfo osAccountInfo;
    AccountSA::OsAccountManager::QueryCurrentOsAccount(osAccountInfo);
    HILOGI("curOsAccount: id=%{public}d, name=%{public}s", osAccountInfo.GetLocalId(), osAccountInfo.GetLocalName().c_str());
    // the account has been changed before
    OnAccountsChanged(osAccountInfo.GetLocalId());

    AccountSA::OsAccountSubscribeInfo  osAccountSubscribeInfo;
    osAccountSubscribeInfo.SetOsAccountSubscribeType(AccountSA::OS_ACCOUNT_SUBSCRIBE_TYPE::ACTIVED);
    osAccountSubscribeInfo.SetName("MemMgrAccountActivedSubscriber");

    subscriber_ = std::make_shared<AccountSubscriber>(osAccountSubscribeInfo,
                    std::bind(&AccountObserver::OnAccountsChanged, this, std::placeholders::_1));
    ErrCode errCode = AccountSA::OsAccountManager::SubscribeOsAccount(subscriber_);
    HILOGI("SubscribeOsAccount errCode=%{public}d", errCode);
}

AccountObserver::~AccountObserver()
{
    if (subscriber_) {
        AccountSA::OsAccountManager::UnsubscribeOsAccount(subscriber_);
    }
}

void AccountObserver::OnAccountsChanged(const int &id)
{
    HILOGI("called");
    if (callback_.OnOsAccountsChanged != nullptr) {
        callback_.OnOsAccountsChanged(id);
    }
}
} // namespace Memory
} // namespace OHOS
