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

#ifndef OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_KILL_H
#define OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_KILL_H

#include <stdexcept>
#include <map>
#include "libxml/parser.h"

namespace OHOS {
namespace Memory {
class KillConfig {
public:
    using KillLevelsMap = std::map<unsigned int, int>;
    void ClearKillLevelsMap();
    const KillLevelsMap& GetKillLevelsMap();
    void ParseConfig(const xmlNodePtr &rootNodePtr);
    bool ParseKillLevelNode(const xmlNodePtr &currNodePtr, std::map<std::string, std::string> &param);

private:
    KillLevelsMap killLevelsMap_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_KILL_H