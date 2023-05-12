/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_PURGEABLE_MEM_UTILS_H
#define OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_PURGEABLE_MEM_UTILS_H

#include <string>
#include <vector>

#include "single_instance.h"

namespace OHOS {
namespace Memory {
struct PurgeableAshmInfo {
    int sizeKB;
    int minPriority;
    std::string idWithTime;
};

class PurgeableMemUtils {
    DECLARE_SINGLE_INSTANCE(PurgeableMemUtils);

public:
    bool GetPurgeableHeapInfo(int &reclaimableKB);
    bool GetProcPurgeableHeapInfo(const int pid, int &reclaimableKB);
    bool PurgeHeapAll();
    bool PurgeHeapMemcg(const std::string &memcgPath, const int sizeKB);
    bool GetPurgeableAshmInfo(int &reclaimableKB, std::vector<PurgeableAshmInfo> &ashmInfoToReclaim);
    bool PurgeAshmAll();
    bool PurgeAshmByIdWithTime(const std::string &idWithTime);

    static const std::string PATH_PURGE_HEAP;
    static const std::string PATH_PURGEABLE_ASHMEM;
    static const std::string FILE_PURGE_MEMCG_HEAP;
    static const std::string ACTIVE_PURGEABLE_HEAP;
    static const std::string INACTIVE_PURGEABLE_HEAP;
    static const std::string PINED_PURGEABLE_HEAP;
    static const std::string PROC_PURGEABLE_HEAP;
    static const std::string PROC_PINED_PURGEABLE_HEAP;
    static const unsigned int ASHM_PARAM_SIZE_ONE_LINE;
    static const unsigned int ASHM_ID_INDEX;
    static const unsigned int ASHM_ADJ_INDEX;
    static const unsigned int ASHM_REF_COUNT_INDEX;
    static const unsigned int ASHM_PURGED_INDEX;
    static const unsigned int ASHM_SIZE_INDEX;
    static const unsigned int ASHM_TIME_INDEX;
    static const unsigned int HEAPINFO_SIZE_ONE_LINE;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_PURGEABLE_MEM_UTILS_H
