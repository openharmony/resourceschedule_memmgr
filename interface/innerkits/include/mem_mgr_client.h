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

#ifndef OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_CLIENT_H
#define OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_CLIENT_H

#include "i_mem_mgr.h"
#include "single_instance.h"

namespace OHOS {
namespace Memory {
class MemMgrClient {
    DECLARE_SINGLE_INSTANCE(MemMgrClient);

public:
    int32_t GetBundlePriorityList(BundlePriorityList &bundlePrioList);
    int32_t NotifyDistDevStatus(int32_t pid, int32_t uid, const std::string &name, bool connected);

private:
    sptr<IMemMgr> GetMemMgrService();
    std::mutex mutex_;
    sptr<IMemMgr> dpProxy_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_CLIENT_H
