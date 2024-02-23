/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "mem_mgr_constant.h"

namespace OHOS {
namespace Memory {
std::string GetReclaimPriorityString(int32_t priority)
{
    if (priority < RECLAIM_PRIORITY_SYSTEM || priority > RECLAIM_PRIORITY_UNKNOWN) {
        return RECLAIM_PRIORITY_UNKNOWN_DESC;
    } else if (priority < RECLAIM_ONDEMAND_SYSTEM) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_SYSTEM);
    } else if (priority < RECLAIM_PRIORITY_KILLABLE_SYSTEM) {
        return ReclaimPriorityMapping.at(RECLAIM_ONDEMAND_SYSTEM);
    } else if (priority < RECLAIM_PRIORITY_FOREGROUND) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_KILLABLE_SYSTEM);
    } else if (priority < RECLAIM_PRIORITY_VISIBLE) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_FOREGROUND);
    } else if (priority < RECLAIM_PRIORITY_BG_SUSPEND_DELAY) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_VISIBLE);
    } else if (priority < RECLAIM_PRIORITY_BG_PERCEIVED) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_BG_SUSPEND_DELAY);
    } else if (priority < RECLAIM_PRIORITY_BG_DIST_DEVICE) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_BG_PERCEIVED);
    } else if (priority < RECLAIM_PRIORITY_BACKGROUND) {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_BG_DIST_DEVICE);
    } else {
        return ReclaimPriorityMapping.at(RECLAIM_PRIORITY_BACKGROUND);
    }
}
} // namespace Memory
} // namespace OHOS
