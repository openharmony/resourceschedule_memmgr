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

#include "multi_account_manager.h"
#include "default_multi_account_strategy.h"

namespace OHOS {
namespace Memory {
void DefaultMultiAccountStrategy::SetAccountPriority(int accountId)
{
    auto accountPriorityInfo = MultiAccountManager::GetInstance().GetAccountPriorityInfo(accountId);
    if (accountPriorityInfo == nullptr) {
        return;
    }

    int priority;
    if (accountPriorityInfo->GetIsActived()) {
        priority = static_cast<int>(DefaultMultiAccountPriority::HIGH_PRIORITY);
    } else {
        priority = static_cast<int>(DefaultMultiAccountPriority::MID_PRIORITY);
    }

    accountPriorityInfo->SetPriority(priority);
}

int DefaultMultiAccountStrategy::RecalcBundlePriority(int accountId, int bundlePriority)
{
    auto accountPriorityInfo = MultiAccountManager::GetInstance().GetAccountPriorityInfo(accountId);
    if (accountPriorityInfo == nullptr) {
        return -1;
    }

    return accountPriorityInfo->GetPriority() + bundlePriority;
}
} // namespace Memory
} // namespace OHOS