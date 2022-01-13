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

#ifndef OHOS_MEMORY_MEMMGR_ACCOUNT_OBSERVER_H
#define OHOS_MEMORY_MEMMGR_ACCOUNT_OBSERVER_H

#include "account_subscriber.h"

namespace OHOS {
namespace Memory {
struct AccountCallback {
    std::function<void(const int &data)> OnOsAccountsChanged;
};

class AccountObserver {
public:
    AccountObserver(const AccountCallback &callback);
    ~AccountObserver();
    void OnAccountsChanged(const int &id);
protected:
private:
    std::shared_ptr<AccountSubscriber> subscriber_;
    AccountCallback callback_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_ACCOUNT_OBSERVER_H
