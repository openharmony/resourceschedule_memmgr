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

#ifndef OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEMMGR_CONFIG_MANAGER_H
#define OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEMMGR_CONFIG_MANAGER_H

#include <map>
#include <string>
#include <set>
#include <memory>
#include "event_handler.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "kernel_interface.h"
#include "single_instance.h"
#include "reclaim_strategy_constants.h"
#include "reclaim_priority_constants.h"
#include "memory_level_constants.h"
#include "avail_buffer_config.h"
#include "kill_config.h"
#include "nand_life_config.h"
#include "reclaim_config.h"
#include "reclaim_priority_config.h"
#include "system_memory_level_config.h"
#include "switch_config.h"
#include "purgeablemem_config.h"

namespace OHOS {
namespace Memory {
class MemmgrConfigManager {
    DECLARE_SINGLE_INSTANCE_BASE(MemmgrConfigManager);
public:
    bool Init();
    void Dump(int fd);
    bool ReadParamFromXml();
    bool WriteReclaimRatiosConfigToKernel();
    bool GetXmlLoaded();
    AvailBufferConfig GetAvailBufferConfig();
    SystemMemoryLevelConfig GetSystemMemoryLevelConfig();
    const ReclaimConfig::ReclaimConfigSet& GetReclaimConfigSet();
    const ReclaimPriorityConfig& GetReclaimPriorityConfig();
    const KillConfig::KillLevelsMap& GetKillLevelsMap();
    const KillConfig& GetKillConfig();
    const NandLifeConfig& GetNandLifeConfig();
    const SwitchConfig& GetSwitchConfig();
    const PurgeablememConfig &GetPurgeablememConfig();

private:
    void InitDefaultConfig();
    bool ParseXmlRootNode(const xmlNodePtr &rootNodePtr);
    void ClearExistConfig();
    void ParseReclaimNode(const xmlNodePtr &rootNodePtr);
    bool xmlLoaded_ = false;
    AvailBufferConfig availBufferConfig_;
    KillConfig killConfig_;
    SystemMemoryLevelConfig systemMemoryLevelConfig_;
    NandLifeConfig nandLifeConfig_;
    SwitchConfig switchConfig_;
    ReclaimPriorityConfig reclaimPriorityConfig_;
    ReclaimConfig reclaimConfig_;
    PurgeablememConfig purgeablememConfig_;
    void ClearReclaimConfigSet();
    MemmgrConfigManager();
    ~MemmgrConfigManager();
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEMMGR_CONFIG_MANAGER_H
