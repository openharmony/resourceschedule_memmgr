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

#ifndef OHOS_MEMORY_MEMMGR_INNERKITS_MEM_MGR_CONSTANT_H
#define OHOS_MEMORY_MEMMGR_INNERKITS_MEM_MGR_CONSTANT_H

#include <string>

namespace OHOS {
namespace Memory {
// system app
constexpr int RECLAIM_PRIORITY_SYSTEM = -1000;
// persist(killable) system app
constexpr int RECLAIM_PRIORITY_KILLABLE_SYSTEM = -800;
// foreground process priority
constexpr int RECLAIM_PRIORITY_FOREGROUND = 0;
// perceived suspend delay case
constexpr int RECLAIM_PRIORITY_BG_SUSPEND_DELAY = 100;
// perceived background process priority
constexpr int RECLAIM_PRIORITY_BG_PERCEIVED = 200;
// background priority
constexpr int RECLAIM_PRIORITY_BACKGROUND = 400;
// unknown process priority
constexpr int RECLAIM_PRIORITY_UNKNOWN = 1000;

const std::map<int, std::string> ReclaimPriorityMapping = {
    { RECLAIM_PRIORITY_SYSTEM, "System" },
    { RECLAIM_PRIORITY_KILLABLE_SYSTEM, "Persistent" },
    { RECLAIM_PRIORITY_FOREGROUND, "Foreground" },
    { RECLAIM_PRIORITY_BG_SUSPEND_DELAY, "Suspend-delay" },
    { RECLAIM_PRIORITY_BG_PERCEIVED, "Perceived" },
    { RECLAIM_PRIORITY_BACKGROUND, "Background" },
};

const std::string RECLAIM_PRIORITY_UNKNOWN_DESC = "Undefined";
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INNERKITS_MEM_MGR_CONSTANT_H
