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

#ifndef OHOS_MEMORY_MEMMGR_OOM_SCORE_ADJ_UTILS_H
#define OHOS_MEMORY_MEMMGR_OOM_SCORE_ADJ_UTILS_H

#include "bundle_priority_info.h"

namespace OHOS {
namespace Memory {
class OomScoreAdjUtils {
public:
    static bool WriteOomScoreAdjToKernel(std::shared_ptr<BundlePriorityInfo> bundle);
    static bool WriteOomScoreAdjToKernel(pid_t pid, int priority);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_OOM_SCORE_ADJ_UTILS_H
