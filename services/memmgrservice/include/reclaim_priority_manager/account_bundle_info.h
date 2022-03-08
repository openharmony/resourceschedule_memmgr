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

#ifndef OHOS_MEMORY_MEMMGR_OS_ACCOUNT_PRIORITY_INFO_H
#define OHOS_MEMORY_MEMMGR_OS_ACCOUNT_PRIORITY_INFO_H

#include "reclaim_priority_constants.h"
#include "bundle_priority_info.h"

#include <string>
#include <list>
#include <map>

namespace OHOS {
namespace Memory {
class AccountBundleInfo {
public:
    int id_;

    explicit AccountBundleInfo(int accountId);
    bool HasBundle(int bundleId);
    std::shared_ptr<BundlePriorityInfo> FindBundleById(int bundleId);
    void AddBundleToOsAccount(std::shared_ptr<BundlePriorityInfo> bundle);
    void RemoveBundleById(int bundleUid);
    int GetBundlesCount();
    // map <bundleUid, std::shared_ptr<BundlePriorityInfo>>
    using BundlePrioMap = std::map<int, std::shared_ptr<BundlePriorityInfo>>;
    BundlePrioMap bundleIdInfoMapping_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_OS_ACCOUNT_PRIORITY_INFO_H
