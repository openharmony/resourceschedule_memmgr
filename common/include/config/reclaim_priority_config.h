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

#ifndef OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_CONFIG_RECLAIM_PRIORITY_CONFIG_H
#define OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_CONFIG_RECLAIM_PRIORITY_CONFIG_H

#include <map>
#include <string>
#include <set>
#include "libxml/parser.h"
#include "libxml/xpath.h"

namespace OHOS {
namespace Memory {
class ReclaimPriorityConfig {
public:
    void ParseConfig(const xmlNodePtr &rootNodePtr);
    void ParseReclaimPriorityKillableSystemAppsConfig(const xmlNodePtr &rootNodePtr);
    void ParseReclaimPriorityImportantBgAppsConfig(const xmlNodePtr &rootNodePtr);
    std::set<std::string> GetkillalbeSystemApps();
    std::map<std::string, int> GetImportantBgApps();
    void Dump(int fd);

private:
    std::set<std::string> killalbeSystemApps_;
    std::map<std::string, int> importantBgApps_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_CONFIG_RECLAIM_PRIORITY_CONFIG_H