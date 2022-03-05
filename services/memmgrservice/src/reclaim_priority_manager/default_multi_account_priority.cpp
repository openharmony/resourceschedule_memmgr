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

#include "default_multi_account_strategy.h"

namespace OHOS {
namespace Memory {
void DefaultMultiAccountStrategy::SetAccountPriority(std::shared_ptr<AccountPriorityInfo> accountInfo)
{
    if (accountInfo == nullptr) {
        return;
    }

    int priority;
    if (accountInfo->GetIsActived()) {
        priority = static_cast<int>(DefaultMultiAccountPriority::HIGH_PRIORITY);
    } else {
        priority = static_cast<int>(DefaultMultiAccountPriority::MID_PRIORITY);
    }

    accountInfo->SetPriority(priority);
}

int DefaultMultiAccountStrategy::RecalcBundlePriority(std::shared_ptr<AccountPriorityInfo> accountInfo,
                                                      int bundlePriority)
{
    if (accountInfo == nullptr) {
        return -1;
    }

    return accountInfo->GetPriority() + bundlePriority;
}
} // namespace Memory
} // namespace OHOS