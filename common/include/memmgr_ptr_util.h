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
#define DECLARE_SHARED_POINTER(classType, ptrName) std::shared_ptr<classType> ptrName = nullptr

#define DECLARE_UNIQUE_POINTER(classType, ptrName) std::unique_ptr<classType> ptrName = nullptr

#define MAKE_POINTER(returnValue, ptrType, classType, errLog, exec_expr0, ...)   \
    do {                                                                         \
        returnValue = nullptr;                                                   \
        try {                                                                    \
            returnValue = std::make_##ptrType<classType>(__VA_ARGS__);           \
        } catch (...) {                                                          \
            HILOGE(errLog);                                                      \
        };                                                                       \
        if (returnValue == nullptr) {                                            \
            exec_expr0;                                                          \
        }                                                                        \
    } while (0)
} // namespace MemMgr
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEM_MGR_PTR_UTIL_H
