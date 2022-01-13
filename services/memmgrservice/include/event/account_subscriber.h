/*
 * Copyright (c) 2021 XXXX.
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

#ifndef OHOS_MEMORY_MEMMGR_ACCOUNT_SUBSCRIBER_H
#define OHOS_MEMORY_MEMMGR_ACCOUNT_SUBSCRIBER_H

#include "os_account_subscriber.h"
#include "os_account_subscribe_info.h"

namespace OHOS {
namespace Memory {
class AccountSubscriber : public AccountSA::OsAccountSubscriber {
public:
    AccountSubscriber(const AccountSA::OsAccountSubscribeInfo &subscriberInfo,
        const std::function<void(const int &)> &callback);
    ~AccountSubscriber();
    virtual void OnAccountsChanged(const int &id) override;
protected:
private:
    std::function<void(const int &)> callback_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_ACCOUNT_SUBSCRIBER_H
