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
#include "reclaim_priority_constants.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "DefaultMultiAccountStrategy";
}

bool DefaultMultiAccountStrategy::SetAccountPriority(std::shared_ptr<AccountPriorityInfo> accountInfo)
{
    if (accountInfo == nullptr) {
        HILOGI("Set the account priority failed because the accountInfo is null.");
        return false;
    }

    int priority;
    if (accountInfo->GetIsActived()) {
        priority = static_cast<int>(DefaultMultiAccountPriority::HIGH_PRIORITY);
    } else {
        priority = static_cast<int>(DefaultMultiAccountPriority::MID_PRIORITY);
    }

    accountInfo->SetPriority(priority);
    return true;
}

int DefaultMultiAccountStrategy::RecalcBundlePriority(std::shared_ptr<AccountPriorityInfo> accountInfo,
                                                      int bundlePriority)
{
    if (accountInfo == nullptr) {
        return RECLAIM_PRIORITY_MAX;
    }

    return accountInfo->GetPriority() + bundlePriority;
}
} // namespace Memory
} // namespace OHOS