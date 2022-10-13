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
#include "switch_config.h"
#include "memmgr_log.h"
#include "xml_helper.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "SwitchConfig";
}

void SwitchConfig::SetBigMemKillSwitch(unsigned long long bigMemKillSwitch)
{
    bigMemKillSwitch_ = bigMemKillSwitch;
}

unsigned long long SwitchConfig::GetBigMemKillSwitch(void)
{
    return bigMemKillSwitch_;
}

void SwitchConfig::ParseConfig(const xmlNodePtr &rootNodePtr)
{
    if (!XmlHelper::CheckNode(rootNodePtr) || !XmlHelper::HasChild(rootNodePtr)) {
        HILOGD("Node exsist:%{public}d,has child node:%{public}d",
               XmlHelper::CheckNode(rootNodePtr), XmlHelper::HasChild(rootNodePtr));
        return;
    }

    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        std::string key = std::string(reinterpret_cast<const char *>(currNode->name));
        if (key == "bigMemKillSwitch") {
            if (!XmlHelper::ParseUnsignedLongLongContent(currNode, bigMemKillSwitch_)) {
                HILOGE("parse key :<%{public}s> error", key.c_str());
                return;
            }
            HILOGI("bigMemKillSwitch_=%{public}llu", bigMemKillSwitch_);
        } else {
            HILOGW("unknown key :<%{public}s>", key.c_str());
        }
    }
}

void SwitchConfig::Dump(int fd)
{
    dprintf(fd, "SwitchConfig:   \n");
    dprintf(fd, "                   bigMemKillSwitch: %llu\n", bigMemKillSwitch_);
}
} // namespace Memory
} // namespace OHOS