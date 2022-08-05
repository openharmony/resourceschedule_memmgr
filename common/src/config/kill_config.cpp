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
#include "reclaim_priority_constants.h"
#include "memmgr_log.h"
#include "xml_helper.h"
#include "kill_config.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "KillConfig";
    const unsigned int KILL_LEVEL_ITEMS__MAX_COUNT = 10;
}

void KillConfig::ClearKillLevelsMap()
{
    killLevelsMap_.clear();
}

void KillConfig::ParseConfig(const xmlNodePtr &rootNodePtr)
{
    if (!XmlHelper::CheckNode(rootNodePtr) || !XmlHelper::HasChild(rootNodePtr)) {
        HILOGD("Node exsist:%{public}d,has child node:%{public}d",
               XmlHelper::CheckNode(rootNodePtr), XmlHelper::HasChild(rootNodePtr));
        return;
    }
    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        std::map<std::string, std::string> param;
        XmlHelper::GetModuleParam(currNode, param);
        if (!ParseKillLevelNode(currNode, param) || killLevelsMap_.size() > KILL_LEVEL_ITEMS__MAX_COUNT) {
            /* if has error, clear set */
            killLevelsMap_.clear();
            break;
        }
    }
    if (killLevelsMap_.empty()) {
        return;
    }
    /* check if <memoryMB, minPriority> is increasing, if not, invalid. */
    int lastPriority = RECLAIM_PRIORITY_MIN - 1;
    for (auto it = killLevelsMap_.begin(); it != killLevelsMap_.end(); it++) {
        HILOGD("KillConfig: memory:%{public}u prio:%{public}d lastPrio:%{public}d",
               it->first, it->second, lastPriority);
        if (it->second <= lastPriority) {
            /* Priority list must be decreasing, if not, clear set */
            HILOGE("KillConfig: memory:%{public}u prio:%{public}d invalid", it->first, it->second);
            killLevelsMap_.clear();
            break;
        }
        lastPriority = it->second;
    }
}

const KillConfig::KillLevelsMap& KillConfig::GetKillLevelsMap()
{
    return killLevelsMap_;
}

bool KillConfig::ParseKillLevelNode(const xmlNodePtr &currNodePtr,
                                    std::map<std::string, std::string> &param)
{
    std::string name = std::string(reinterpret_cast<const char *>(currNodePtr->name));
    if (name.compare("comment") == 0) {
        HILOGD("%{public}s comment skip :<%{public}s>", __func__, name.c_str());
        return true;
    }
    if (name.compare("killLevel") != 0) {
        HILOGW("%{public}s unknown node :<%{public}s>", __func__, name.c_str());
        return false;
    }
    unsigned int memoryMB;
    int minPriority;
    XmlHelper::SetUnsignedIntParam(param, "memoryMB", memoryMB, 0);
    XmlHelper::SetIntParam(param, "minPriority", minPriority, RECLAIM_PRIORITY_MAX + 1);
    /* if read from xml fail, error */
    if (memoryMB == 0 || minPriority == (RECLAIM_PRIORITY_MAX + 1)) {
        HILOGE("node:<%{public}s> <%{public}u, %{public}d> read fail", name.c_str(), memoryMB, minPriority);
        return false;
    }
    if ((memoryMB * KB_PER_MB) > UINT_MAX) {
        HILOGE("<%{public}s> %{public}u MB to KB > UINT_MAX", name.c_str(), memoryMB);
        return false;
    }
    unsigned int memoryKB = memoryMB * KB_PER_MB; /* MB to KB */
    if (killLevelsMap_.count(memoryKB) > 0) { /* if key @memoryKB has exist, error */
        HILOGE("node:<%{public}s> <%{public}u, %{public}d> key has exist, mapSize:%{public}zu",
               name.c_str(), memoryKB, minPriority, killLevelsMap_.size());
        return false;
    }
    killLevelsMap_.insert(std::make_pair(memoryKB, minPriority));
    return true;
}
} // namespace Memory
} // namespace OHOS
