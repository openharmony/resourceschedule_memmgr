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

#ifndef OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_CONFIG_RECLAIM_CONFIG_H
#define OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_CONFIG_RECLAIM_CONFIG_H

#include <map>
#include <string>
#include <set>
#include <memory>

#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "kernel_interface.h"
#include "reclaim_strategy_constants.h"
#include "reclaim_priority_constants.h"
#include "memory_level_constants.h"

namespace OHOS {
namespace Memory {
class ZswapdParam {
public:
    int GetMinScore();
    void SetMinScore(int minScore);
    int GetMaxScore();
    void SetMaxScore(int maxScore);
    unsigned int GetMem2zramRatio();
    void SetMem2zramRatio(unsigned int mem2zramRatio);
    unsigned int GetZram2ufsRatio();
    void SetZram2ufsRatio(unsigned int zram2ufsRatio);
    unsigned int GetRefaultThreshold();
    void SetRefaultThreshold(unsigned int refaultThreshold);
    ZswapdParam();
    ZswapdParam(int minScore, int maxScore, unsigned int mem2zramRatio,
                unsigned int zram2ufsRatio, unsigned int refaultThreshold);

private:
    int minScore_;
    int maxScore_;
    unsigned int mem2zramRatio_;
    unsigned int zram2ufsRatio_;
    unsigned int refaultThreshold_;
};

struct ZswapdParamPtrCmp {
    bool operator()(const std::shared_ptr<ZswapdParam> p1, const std::shared_ptr<ZswapdParam> p2)
    {
        if (p1->GetMinScore() <= p2->GetMinScore()) {
            return true;
        } else {
            return false;
        }
    };
};

class ReclaimConfig {
public:
    void ParseConfig(const xmlNodePtr &rootNodePtr);
    void SetZswapdParamConfig(std::map<std::string, std::string> &param);
    using ReclaimConfigSet = std::set<std::shared_ptr<ZswapdParam>, ZswapdParamPtrCmp>;
    const ReclaimConfigSet& GetReclaimConfigSet();
    void AddReclaimConfigToSet(std::shared_ptr<ZswapdParam> reclaimConfig);
    void ClearReclaimConfigSet();
    ReclaimConfig();
    void SetDefaultConfig(int minScore, int maxScore, unsigned int mem2zramRatio,
                          unsigned int zram2ufsRatio, unsigned int refaultThreshold);
    void Dump(int fd);
private:
    ReclaimConfigSet reclaimConfigSet_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_CONFIG_RECLAIM_CONFIG_H