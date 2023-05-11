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
#include "system_memory_level_config.h"

#include "memmgr_log.h"
#include "memory_level_constants.h"
#include "xml_helper.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "SystemMemoryLevelConfig";
}

void SystemMemoryLevelConfig::SetPurgeable(unsigned int purgeable)
{
    purgeable_ = purgeable;
}

unsigned int SystemMemoryLevelConfig::GetPurgeable(void)
{
    return purgeable_;
}

void SystemMemoryLevelConfig::SetModerate(unsigned int moderate)
{
    moderate_ = moderate;
}

unsigned int SystemMemoryLevelConfig::GetModerate(void)
{
    return moderate_;
}

void SystemMemoryLevelConfig::SetLow(unsigned int low)
{
    low_ = low;
}

unsigned int SystemMemoryLevelConfig::GetLow(void)
{
    return low_;
}

void SystemMemoryLevelConfig::SetCritical(unsigned int critical)
{
    critical_ = critical;
}

unsigned int SystemMemoryLevelConfig::GetCritical(void)
{
    return critical_;
}

void SystemMemoryLevelConfig::ParseConfig(const xmlNodePtr &rootNodePtr)
{
    if (!XmlHelper::CheckNode(rootNodePtr) || !XmlHelper::HasChild(rootNodePtr)) {
        HILOGD("Node exsist:%{public}d,has child node:%{public}d",
               XmlHelper::CheckNode(rootNodePtr), XmlHelper::HasChild(rootNodePtr));
        return;
    }

    std::map<std::string, std::string> param;
    if (!XmlHelper::GetModuleParam(rootNodePtr, param)) {
        HILOGW("Get moudle param failed.");
        return;
    }

    unsigned int purgeable;
    unsigned int moderate;
    unsigned int low;
    unsigned int critical;
    XmlHelper::SetUnsignedIntParam(param, "purgeable", purgeable, MEMORY_LEVEL_PURGEABLE_DEFAULT);
    XmlHelper::SetUnsignedIntParam(param, "moderate", moderate, MEMORY_LEVEL_MODERATE_DEFAULT);
    XmlHelper::SetUnsignedIntParam(param, "low", low, MEMORY_LEVEL_LOW_DEFAULT);
    XmlHelper::SetUnsignedIntParam(param, "critical", critical, MEMORY_LEVEL_CRITICAL_DEFAULT);

    /* change MB to KB */
    purgeable *= KB_PER_MB;
    moderate *= KB_PER_MB;
    low *= KB_PER_MB;
    critical *= KB_PER_MB;

    SetPurgeable(purgeable);
    SetModerate(moderate);
    SetLow(low);
    SetCritical(critical);

    HILOGI("purgeable=%{public}u, moderate=%{public}u, low=%{public}u, critical=%{public}u.", purgeable, moderate, low,
           critical);
}

void SystemMemoryLevelConfig::Dump(int fd)
{
    dprintf(fd, "SystemMemoryLevelConfig:   \n");
    dprintf(fd, "                     purgeable: %u\n", purgeable_);
    dprintf(fd, "                      moderate: %u\n", moderate_);
    dprintf(fd, "                           low: %u\n", low_);
    dprintf(fd, "                      critical: %u\n", critical_);
}
} // namespace Memory
} // namespace OHOS
