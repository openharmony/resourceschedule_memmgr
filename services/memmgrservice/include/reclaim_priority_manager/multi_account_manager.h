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

#ifndef OHOS_MEMORY_MEMMGR_MULTI_ACCOUNT_MANAGER_H
#define OHOS_MEMORY_MEMMGR_MULTI_ACCOUNT_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include "single_instance.h"
#include "account_priority_info.h"
#include "multi_account_strategy.h"

namespace OHOS {
namespace Memory {
class MultiAccountManager {
    DECLARE_SINGLE_INSTANCE_BASE(MultiAccountManager);
public:
    void SetAccountPriority(int accountId, std::string accountName, AccountType accountType, bool isActived);
    int RecalcBundlePriority(int accountId, int bundlePriority);
    std::shared_ptr<AccountPriorityInfo> GetAccountPriorityInfo(int accountId);
    void AddAccountPriorityInfo(std::shared_ptr<AccountPriorityInfo> accountPriorityInfo);
    std::shared_ptr<MultiAccountStrategy> GetMultiAccountStratgy();
    void SetMultiAccountStrategy(std::shared_ptr<MultiAccountStrategy> strategy);

private:
    std::map<int, std::shared_ptr<AccountPriorityInfo>> accountMap_;
    std::shared_ptr<MultiAccountStrategy> strategy_;
    MultiAccountManager();
    ~MultiAccountManager();
};
} // namespace Memory
} // namespace OHOS

#endif // OHOS_MEMORY_MEMMGR_MULTI_ACCOUNT_MANAGER_H