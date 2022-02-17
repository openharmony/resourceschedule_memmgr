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

#ifndef OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_MANAGER_H
#define OHOS_MEMORY_MEMMGR_MEMMGR_CONFIG_MANAGER_H

#include <stdexcept>
#include <map>
#include <string>
#include <set>
#include "event_handler.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "kernel_interface.h"
#include "single_instance.h"
#include "reclaim_strategy_constants.h"
#include "reclaim_priority_constants.h"

namespace OHOS {
namespace Memory {
class AvailBufferSize {
public:
    int availBuffer;
    int minAvailBuffer;
    int highAvailBuffer;
    int swapReserve;
    AvailBufferSize(int availBuffer, int minAvailBuffer, int highAvailBuffer, int swapReserve);
};

class ReclaimRatiosConfig {
public:
    int minScore;
    int maxScore;
    int mem2zramRatio;
    int zran2ufsRation;
    int refaultThreshold;
    ReclaimRatiosConfig(int minScore, int maxScore, int mem2zramRatio, int zran2ufsRation, int refaultThreshold);
};

struct ReclaimRatiosConfigPtrCmp {
    bool operator()(const ReclaimRatiosConfig *p1, const ReclaimRatiosConfig *p2)
    {
        if (p1->minScore <= p2->minScore) {
            return true;
        } else {
            return false;
        }
    };
};

class MemmgrConfigManager {
    DECLARE_SINGLE_INSTANCE_BASE(MemmgrConfigManager);
public:
    bool Init();
    bool ReadParamFromXml();
    bool WriteReclaimRatiosConfigToKernel();
    using ReclaimRatiosConfigSet = std::set<ReclaimRatiosConfig *, ReclaimRatiosConfigPtrCmp>;
    bool GetXmlLoaded();
    AvailBufferSize *GetAvailBufferSize();
    const ReclaimRatiosConfigSet GetReclaimRatiosConfigSet();

private:
    bool ParseXmlRootNode(const xmlNodePtr &rootNodePtr);
    bool ParseKillConfig(const xmlNodePtr &rootNodePtr);
    bool ParseReclaimConfig(const xmlNodePtr &rootNodePtr);
    bool GetModuleParam(const xmlNodePtr &currNodePtr, std::map<std::string, std::string> &param);
    void SetIntParam(std::map<std::string, std::string> &param, std::string key, int* dst);
    bool SetReclaimParam(const xmlNodePtr &currNodePtr, std::map<std::string, std::string> &param);
    bool SetAvailBufferConfig(std::map<std::string, std::string> &param);
    bool SetZswapdParamConfig (std::map<std::string, std::string> &param);
    bool CheckNode(const xmlNodePtr &nodePtr);
    bool HasChild(const xmlNodePtr &rootNodePtr);
    bool CheckPathExist(const char *path);
    void ClearExistConfig();
    bool XmlLoaded = false;
    AvailBufferSize *availBufferSize =
        new AvailBufferSize(AVAIL_BUFFER, MIN_AVAIL_BUFFER, HIGH_AVAIL_BUFFER, SWAP_RESERVE);
    ReclaimRatiosConfigSet reclaimRatiosConfigSet {
        new ReclaimRatiosConfig(RECLAIM_PRIORITY_MIN, RECLAIM_PRIORITY_MAX, MEMCG_MEM_2_ZRAM_RATIO,
                                MEMCG_ZRAM_2_UFS_RATIO, MEMCG_REFAULT_THRESHOLD)};
    void AddReclaimRatiosConfigToSet(ReclaimRatiosConfig *reclaimRatiosConfig);
    void ClearReclaimRatiosConfigSet();
    MemmgrConfigManager();
    ~MemmgrConfigManager();
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECLAIM_CONFIG_MANAGER_H
