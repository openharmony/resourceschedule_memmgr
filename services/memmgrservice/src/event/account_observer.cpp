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
#include "memmgr_ptr_util.h"

#include "os_account_manager.h"
#include "os_account_info.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "AccountObserver";
const int MAX_RETRY_TIMES = 10;
}

AccountObserver::AccountObserver(const AccountCallback &callback) : callback_(callback)
{
    HILOGI("called");
    if (!GetEventHandler()) {
        return;
    }
    Register();
}

bool AccountObserver::GetEventHandler()
{
    if (!handler_) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

void AccountObserver::Register()
{
    retryTimes_++;

    AccountSA::OsAccountSubscribeInfo osAccountSubscribeInfo;
    osAccountSubscribeInfo.SetOsAccountSubscribeType(AccountSA::OS_ACCOUNT_SUBSCRIBE_TYPE::ACTIVED);
    osAccountSubscribeInfo.SetName("MemMgrAccountActivedSubscriber");

    MAKE_POINTER(subscriber_, shared, AccountSubscriber, "make shared failed", return,
        osAccountSubscribeInfo, std::bind(&AccountObserver::OnAccountsChanged, this, std::placeholders::_1));
    ErrCode errCode = AccountSA::OsAccountManager::SubscribeOsAccount(subscriber_);
    if (errCode == ERR_OK) {
        HILOGI("Subscribe osAccount succeed.");
        return;
    }

    HILOGI("Subscribe osAccount failed, retCode = %{public}d.", errCode);
    if (retryTimes_ < MAX_RETRY_TIMES) {
        std::function<void()> RegisterEventListenerFunc = std::bind(&AccountObserver::Register, this);
        HILOGE("failed to SubscribeOsAccount, try again after 3s!, retryTimes=%{public}d/10", retryTimes_);
        handler_->PostTask(RegisterEventListenerFunc, 3000, AppExecFwk::EventQueue::Priority::LOW); // 3000 means 3s
    }
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
