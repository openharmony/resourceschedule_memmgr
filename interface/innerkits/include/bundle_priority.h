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

#ifndef OHOS_MEMORY_MEMMGR_INNERKITS_BUNDLE_INFO_H
#define OHOS_MEMORY_MEMMGR_INNERKITS_BUNDLE_INFO_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace Memory {
class BundlePriority {
public:
    BundlePriority();
    BundlePriority(int32_t uid, std::string name, int32_t priority, int32_t accountId) : uid_(uid), name_(name),
        priority_(priority), accountId_(accountId) {};
    int32_t uid_ {0};
    std::string name_ {""};
    int32_t priority_ {0};
    int32_t accountId_ {0};
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INNERKITS_BUNDLE_INFO_H
