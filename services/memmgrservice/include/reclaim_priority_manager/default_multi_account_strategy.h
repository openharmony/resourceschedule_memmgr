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

#ifndef OHOS_MEMORY_MEMMGR_DEFAULT_MULTI_ACCOUNT_STRATEGY_H
#define OHOS_MEMORY_MEMMGR_DEFAULT_MULTI_ACCOUNT_STRATEGY_H

#include "multi_account_strategy.h"

namespace OHOS {
namespace Memory {
enum class DefaultMultiAccountPriority {
    LOW_PRIORITY = 100,
    MID_PRIORITY = 50,
    HIGH_PRIORITY = 0,
};

class DefaultMultiAccountStrategy : public MultiAccountStrategy {
public:
    void SetAccountPriority(int accountId);
    int RecalcBundlePriority(int accountId, int bundlePriority);
    virtual ~DefaultMultiAccountStrategy() {}
};
} // namespace Memory
} // namespace OHOS

#endif // OHOS_MEMORY_MEMMGR_DEFAULT_MULTI_ACCOUNT_STRATEGY_H