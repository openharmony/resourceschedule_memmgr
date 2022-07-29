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

#ifndef OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_NAND_LIFE_H
#define OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_NAND_LIFE_H

#include <stdexcept>
#include <map>
#include <string>
#include <set>
#include "event_handler.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"

namespace OHOS {
namespace Memory {
class NandLifeConfig {
public:
    void ParseConfig(const xmlNodePtr &rootNodePtr);
    void SetDailySwapOutQuotaMb(unsigned long long dailySwapOutQuotaMb);
    unsigned long long GetDailySwapOutQuotaMb(void);
    void SetTotalSwapOutQuotaMb(unsigned long long totalSwapOutQuotaMb);
    unsigned long long GetTotalSwapOutQuotaMb(void);

private:
    unsigned long long dailySwapOutQuotaMb_;
    unsigned long long totalSwapOutQuotaMb_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_NAND_LIFE_H