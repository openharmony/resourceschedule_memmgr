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

#ifndef OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEM_MGR_PTR_UTIL_H
#define OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEM_MGR_PTR_UTIL_H

#include "hilog/log.h"

namespace OHOS {
namespace Memory {
#define MEMMGR_MAKE_SHARED(exec_expr0)                       \
    do {                                                     \
        try {                                                \
            exec_expr0;                                      \
        } catch (...) {                                      \
            HILOGE("make shared failed");                    \
        }                                                    \
    } while (0)

#define MEMMGR_MAKE_SHARED_RETURN(exec_expr0, exec_expr1)    \
    do {                                                     \
        try {                                                \
            exec_expr0;                                      \
        } catch (...) {                                      \
            HILOGE("make shared failed");                    \
            exec_expr1;                                      \
        }                                                    \
    } while (0)

#define MEMMGR_MAKE_UNIQUE(exec_expr0)                       \
    do {                                                     \
        try {                                                \
            exec_expr0;                                      \
        } catch (...) {                                      \
            HILOGE("make unique failed");                    \
        }                                                    \
    } while (0)

#define MEMMGR_MAKE_UNIQUE_RETURN(exec_expr0, exec_expr1)    \
    do {                                                     \
        try {                                                \
            exec_expr0;                                      \
        } catch (...) {                                      \
            HILOGE("make unique failed");                    \
            exec_expr1;                                      \
        }                                                    \
    } while (0)
} // namespace MemMgr
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEM_MGR_PTR_UTIL_H
