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
#include "memory_level_constants.h"

namespace OHOS {
namespace Memory {
class AvailBufferSize {
public:
    unsigned int availBuffer;
    unsigned int minAvailBuffer;
    unsigned int highAvailBuffer;
    unsigned int swapReserve;
    AvailBufferSize(unsigned int availBuffer, unsigned int minAvailBuffer,
                    unsigned int highAvailBuffer, unsigned int swapReserve);
};

class SystemMemoryLevelConfig {
public:
    unsigned int moderate;
    unsigned int low;
    unsigned int critical;
    SystemMemoryLevelConfig(unsigned int moderate, unsigned int low, unsigned int critical);
};

class ReclaimPriorityConfig {
public:
    std::set<std::string> killalbeSystemApps_;
};

class NandLifeConfig {
public:
    int DAILY_SWAP_OUT_QUOTA_MB;
    int TOTAL_SWAP_OUT_QUOTA_MB;
};

class ReclaimRatiosConfig {
public:
    int minScore;
    int maxScore;
    unsigned int mem2zramRatio;
    unsigned int zram2ufsRatio;
    unsigned int refaultThreshold;
    ReclaimRatiosConfig(int minScore, int maxScore, unsigned int mem2zramRatio,
                        unsigned int zram2ufsRatio, unsigned int refaultThreshold);
};

struct ReclaimRatiosConfigPtrCmp {
    bool operator()(const std::shared_ptr<ReclaimRatiosConfig> p1, const std::shared_ptr<ReclaimRatiosConfig> p2)
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
    using ReclaimRatiosConfigSet = std::set<std::shared_ptr<ReclaimRatiosConfig>, ReclaimRatiosConfigPtrCmp>;
    using KillLevelsMap = std::map<unsigned int, int>;
    bool GetXmlLoaded();
    std::shared_ptr<AvailBufferSize> GetAvailBufferSize();
    std::shared_ptr<SystemMemoryLevelConfig> GetSystemMemoryLevelConfig();
    const ReclaimRatiosConfigSet GetReclaimRatiosConfigSet();
    const ReclaimPriorityConfig& GetReclaimPriorityConfig();
	const KillLevelsMap& GetKillLevelsMap();
    const NandLifeConfig& GetNandLifeConfig();

private:
    void InitDefaultConfig();
    bool ParseXmlRootNode(const xmlNodePtr &rootNodePtr);
    bool ParseKillConfig(const xmlNodePtr &rootNodePtr);
    bool ParseKillLevelNode(const xmlNodePtr &currNodePtr, std::map<std::string, std::string> &param);
    bool ParseReclaimConfig(const xmlNodePtr &rootNodePtr);
    bool ParseSystemMemoryLevelConfig(const xmlNodePtr &rootNodePtr);
    bool ParseReclaimPriorityConfig(const xmlNodePtr &rootNodePtr);
    bool ParseReclaimPriorityKillableSystemAppsConfig(const xmlNodePtr &rootNodePtr);
    bool ParseNandLifeConfig(const xmlNodePtr &rootNodePtr);
    bool GetModuleParam(const xmlNodePtr &currNodePtr, std::map<std::string, std::string> &param);
    void SetIntParam(std::map<std::string, std::string> &param, std::string key, int &dst);
    void SetUnsignedIntParam(std::map<std::string, std::string> &param, std::string key, unsigned int &dst);
    bool SetReclaimParam(const xmlNodePtr &currNodePtr, std::map<std::string, std::string> &param);
    bool SetAvailBufferConfig(std::map<std::string, std::string> &param);
    bool SetZswapdParamConfig (std::map<std::string, std::string> &param);
    bool CheckNode(const xmlNodePtr &nodePtr);
    bool HasChild(const xmlNodePtr &rootNodePtr);
    bool CheckPathExist(const char *path);
    void ClearExistConfig();
    bool xmlLoaded_ = false;
    std::shared_ptr<AvailBufferSize> availBufferSize_;
    std::shared_ptr<SystemMemoryLevelConfig> systemMemoryLevelConfig_;
    KillLevelsMap killLevelsMap_;
    ReclaimRatiosConfigSet reclaimRatiosConfigSet_;
    ReclaimPriorityConfig reclaimPriorityConfig_;
    NandLifeConfig nandLifeConfig_;
    void AddReclaimRatiosConfigToSet(std::shared_ptr<ReclaimRatiosConfig> reclaimRatiosConfig);
    void ClearReclaimRatiosConfigSet();
    MemmgrConfigManager();
    ~MemmgrConfigManager();
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECLAIM_CONFIG_MANAGER_H
