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

#include "purgeablemem_config.h"
#include <set>
#include <string>
#include "memmgr_log.h"
#include "xml_helper.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "PurgeablememConfig";
}

void PurgeablememConfig::ParseConfig(const xmlNodePtr &rootNodePtr)
{
    if (!XmlHelper::CheckNode(rootNodePtr) || !XmlHelper::HasChild(rootNodePtr)) {
        HILOGD("Node exsist:%{public}d,has child node:%{public}d",
               XmlHelper::CheckNode(rootNodePtr), XmlHelper::HasChild(rootNodePtr));
        return;
    }
    std::string nodeName;
    std::string procName;
    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        if (!XmlHelper::CheckNode(currNode)) {
            return;
        }
        std::map<std::string, std::string> param;
        XmlHelper::GetModuleParam(currNode, param);
        nodeName = std::string(reinterpret_cast<const char *>(currNode->name));
        if (nodeName == "purgeWhiteAppList") {
            XmlHelper::SetStringParam(param, "procName", procName, "");
            purgeWhiteAppSet_.insert(procName);
        }
    }
}

PurgeablememConfig::PurgeWhiteAppSet &PurgeablememConfig::GetPurgeWhiteAppSet()
{
    return purgeWhiteAppSet_;
}

void PurgeablememConfig::Dump(int fd)
{
    dprintf(fd, "purgeablememConfig:   \n");
    dprintf(fd, "        purgeWhiteAppList:   \n");
    for (auto it = purgeWhiteAppSet_.begin(); it != purgeWhiteAppSet_.end(); it++) {
        dprintf(fd, "%30s\n", (*it).c_str());
    }
}
} // namespace Memory
} // namespace OHOS