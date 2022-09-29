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
#include "memmgr_log.h"
#include "xml_helper.h"
#include "reclaim_config.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "ReclaimConfig";
}

ReclaimConfig::ReclaimConfig()
{
}

void ReclaimConfig::SetDefaultConfig(int minScore, int maxScore, unsigned int mem2zramRatio,
                                     unsigned int zram2ufsRatio, unsigned int refaultThreshold)
{
    DECLARE_SHARED_POINTER(ZswapdParam, zswapdParam);

    MAKE_POINTER(zswapdParam, shared, ZswapdParam, "make default ZswapdParam failed", return,
        minScore, maxScore, mem2zramRatio, zram2ufsRatio, refaultThreshold);
    AddReclaimConfigToSet(zswapdParam);
}

void ReclaimConfig::ParseConfig(const xmlNodePtr &rootNodePtr)
{
    if (!XmlHelper::CheckNode(rootNodePtr) || !XmlHelper::HasChild(rootNodePtr)) {
        HILOGD("Node exsist:%{public}d,has child node:%{public}d",
               XmlHelper::CheckNode(rootNodePtr), XmlHelper::HasChild(rootNodePtr));
        return;
    }

    std::map<std::string, std::string> param;
    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        if (!XmlHelper::GetModuleParam(currNode, param)) {
            HILOGW("Get moudle param failed.");
            return;
        }
        SetZswapdParamConfig(param);
    }
}

void ReclaimConfig::SetZswapdParamConfig(std::map<std::string, std::string> &param)
{
    int minScore, maxScore;
    unsigned int mem2zramRatio, zram2ufsRatio, refaultThreshold;

    XmlHelper::SetIntParam(param, "minScore", minScore, 0);
    XmlHelper::SetIntParam(param, "maxScore", maxScore, RECLAIM_PRIORITY_MAX);
    XmlHelper::SetUnsignedIntParam(param, "mem2zramRatio", mem2zramRatio, MEMCG_MEM_2_ZRAM_RATIO);
    XmlHelper::SetUnsignedIntParam(param, "zram2ufsRatio", zram2ufsRatio, MEMCG_ZRAM_2_UFS_RATIO);
    XmlHelper::SetUnsignedIntParam(param, "refaultThreshold", refaultThreshold, MEMCG_REFAULT_THRESHOLD);

    DECLARE_SHARED_POINTER(ZswapdParam, zswapdParam);
    MAKE_POINTER(zswapdParam, shared, ZswapdParam, "make ZswapdParam failed", return,
        minScore, maxScore, mem2zramRatio, zram2ufsRatio, refaultThreshold);

    AddReclaimConfigToSet(zswapdParam);
}

void ReclaimConfig::AddReclaimConfigToSet(std::shared_ptr<ZswapdParam> zswapdParam)
{
    reclaimConfigSet_.insert(zswapdParam);
}

void ReclaimConfig::ClearReclaimConfigSet()
{
    reclaimConfigSet_.clear();
}

const ReclaimConfig::ReclaimConfigSet& ReclaimConfig::GetReclaimConfigSet()
{
    return reclaimConfigSet_;
}

ZswapdParam::ZswapdParam()
{
}

ZswapdParam::ZswapdParam(int minScore, int maxScore, unsigned int mem2zramRatio,
                         unsigned int zram2ufsRatio, unsigned int refaultThreshold)
    : minScore_(minScore),
      maxScore_(maxScore),
      mem2zramRatio_(mem2zramRatio),
      zram2ufsRatio_(zram2ufsRatio),
      refaultThreshold_(refaultThreshold)
{
}

int ZswapdParam::GetMinScore()
{
    return minScore_;
}

void ZswapdParam::SetMinScore(int minScore)
{
    minScore_ = minScore;
}

int ZswapdParam::GetMaxScore()
{
    return maxScore_;
}

void ZswapdParam::SetMaxScore(int maxScore)
{
    maxScore_ = maxScore;
}

unsigned int ZswapdParam::GetMem2zramRatio()
{
    return mem2zramRatio_;
}

void ZswapdParam::SetMem2zramRatio(unsigned int mem2zramRatio)
{
    mem2zramRatio_ = mem2zramRatio;
}

unsigned int ZswapdParam::GetZram2ufsRatio()
{
    return zram2ufsRatio_;
}

void ZswapdParam::SetZram2ufsRatio(unsigned int zram2ufsRatio)
{
    zram2ufsRatio_ = zram2ufsRatio;
}

unsigned int ZswapdParam::GetRefaultThreshold()
{
    return refaultThreshold_;
}

void ZswapdParam::SetRefaultThreshold(unsigned int refaultThreshold)
{
    refaultThreshold_ = refaultThreshold;
}

void ReclaimConfig::Dump(int fd)
{
    int index = 0;
    for (auto it = reclaimConfigSet_.begin(); it != reclaimConfigSet_.end(); it++) {
        index++;
        dprintf(fd, "ReclaimConfig %d:   \n", index);
        dprintf(fd, "                      minScore: %d\n", (*it)->GetMinScore());
        dprintf(fd, "                      maxScore: %d\n", (*it)->GetMaxScore());
        dprintf(fd, "                 mem2zramRatio: %u\n", (*it)->GetMem2zramRatio());
        dprintf(fd, "                 zram2ufsRatio: %u\n", (*it)->GetZram2ufsRatio());
        dprintf(fd, "              refaultThreshold: %u\n", (*it)->GetRefaultThreshold());
    }
}
} // namespace Memory
} // namespace OHOS
