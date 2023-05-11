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
// default value of the purgeable memory level
constexpr unsigned int MEMORY_LEVEL_PURGEABLE_DEFAULT = 1024; /* 1024M */
// default value of the moderate memory level
constexpr unsigned int MEMORY_LEVEL_MODERATE_DEFAULT = 800; /* 800M */
// default value of the low memory level
constexpr unsigned int MEMORY_LEVEL_LOW_DEFAULT = 700; /* 700M */
// default value of the critical memory level
constexpr unsigned int MEMORY_LEVEL_CRITICAL_DEFAULT = 600; /* 600M */

enum class SystemMemoryLevel {
    UNKNOWN = 0,
    MEMORY_LEVEL_PURGEABLE = 1,
    MEMORY_LEVEL_MODERATE = 2,
    MEMORY_LEVEL_LOW = 3,
    MEMORY_LEVEL_CRITICAL = 4,
};

enum class MemorySource {
    UNKNOWN = 0,
    PSI_MEMORY = 1,
    KSWAPD = 2,
    MANUAL_DUMP = 3,
};

struct SystemMemoryInfo {
    MemorySource source;
    SystemMemoryLevel level;
};

typedef struct SystemMemoryInfo SystemMemoryInfo;
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_MEMORY_LEVEL_CONSTANTS_H
