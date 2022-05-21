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

#ifndef OHOS_MEMORY_MEMMGR_PROCESS_PRIORITY_INFO_H
#define OHOS_MEMORY_MEMMGR_PROCESS_PRIORITY_INFO_H

#include <string>

#include "reclaim_priority_constants.h"

namespace OHOS {
namespace Memory {
class ProcessPriorityInfo {
public:
    explicit ProcessPriorityInfo(pid_t pid, int bundleUid, int priority);
    ProcessPriorityInfo(const ProcessPriorityInfo &copyProcess);
    int uid_;
    pid_t pid_;
    int priority_;
    bool isFreground; // true means freground, false means background
    bool isBackgroundRunning;
    bool isSuspendDelay;
    bool isEventStart;
    bool isDataAbilityStart;
    bool isDistDeviceConnected;
    void SetPriority(int targetPriority);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_PROCESS_PRIORITY_INFO_H
