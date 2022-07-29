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

#ifndef OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_SYSTEM_MEMORY_LEVEL_H
#define OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_SYSTEM_MEMORY_LEVEL_H

#include <stdexcept>
#include <map>
#include <string>
#include <set>
#include "event_handler.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"

namespace OHOS {
namespace Memory {
class SystemMemoryLevelConfig {
public:
    void ParseConfig(const xmlNodePtr &rootNodePtr);
    void SetModerate(unsigned int moderate);
    unsigned int GetModerate(void);
    void SetLow(unsigned int low);
    unsigned int GetLow(void);
    void SetCritical(unsigned int critical);
    unsigned int GetCritical(void);

private:
    unsigned int moderate_;
    unsigned int low_;
    unsigned int critical_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_SYSTEM_MEMORY_LEVEL_H