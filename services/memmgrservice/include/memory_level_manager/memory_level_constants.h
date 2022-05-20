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

#ifndef OHOS_MEMORY_MEMMGR_MEMORY_LEVEL_CONSTANTS_H
#define OHOS_MEMORY_MEMMGR_MEMORY_LEVEL_CONSTANTS_H

#include <string>
#include <sys/types.h>

namespace OHOS {
namespace Memory {
// default value of the moderate memory level
constexpr int MEMORY_LEVEL_MODERATE_DEFAULT = 819200;
// default value of the low memory level
constexpr int MEMORY_LEVEL_LOW_DEFAULT = 716800;
// default value of the critical memory level
constexpr int MEMORY_LEVEL_CRITICAL_DEFAULT = 614400;

enum class SystemMemoryLevel {
    MEMORY_LEVEL_MODERATE = 0,
    MEMORY_LEVEL_LOW = 1,
    MEMORY_LEVEL_CRITICAL = 2,
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_MEMORY_LEVEL_CONSTANTS_H
