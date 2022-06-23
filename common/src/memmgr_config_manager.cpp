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

#include <fstream>
#include <kernel_interface.h>
#include <stdexcept>
#include <string>

#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "memmgr_config_manager.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "MemmgrConfigManager";
    const std::string XML_PATH = "/etc/memmgr/";
    const std::string MEMCG_PATH = KernelInterface::MEMCG_BASE_PATH;
    const unsigned int KILL_LEVEL_ITEMS__MAX_COUNT = 10;
} // namespace
IMPLEMENT_SINGLE_INSTANCE(MemmgrConfigManager);

bool MemmgrConfigManager::Init()
{
    killLevelsMap_.clear();
    ReadParamFromXml();
    WriteReclaimRatiosConfigToKernel();
    return this->xmlLoaded_;
}

MemmgrConfigManager::MemmgrConfigManager()
{
    InitDefaultConfig();
}

MemmgrConfigManager::~MemmgrConfigManager()
{
    ClearReclaimRatiosConfigSet();
    killLevelsMap_.clear();
}

AvailBufferSize::AvailBufferSize(unsigned int availBuffer, unsigned int minAvailBuffer,
                                 unsigned int highAvailBuffer, unsigned int swapReserve)
    : availBuffer(availBuffer),
      minAvailBuffer(minAvailBuffer),
      highAvailBuffer(highAvailBuffer),
      swapReserve(swapReserve) {};
ReclaimRatiosConfig::ReclaimRatiosConfig(int minScore, int maxScore, unsigned int mem2zramRatio,
                                         unsigned int zram2ufsRatio, unsigned int refaultThreshold)
    : minScore(minScore), maxScore(maxScore), mem2zramRatio(mem2zramRatio),
      zram2ufsRatio(zram2ufsRatio), refaultThreshold(refaultThreshold)
{
}

SystemMemoryLevelConfig::SystemMemoryLevelConfig(unsigned int moderate, unsigned int low, unsigned int critical)
    : moderate(moderate), low(low), critical(critical)
{
}

void MemmgrConfigManager::InitDefaultConfig()
{
    MAKE_POINTER(this->availBufferSize_, shared, AvailBufferSize, "make AvailBufferSize failed", return,
        AVAIL_BUFFER, MIN_AVAIL_BUFFER, HIGH_AVAIL_BUFFER, SWAP_RESERVE);

    DECLARE_SHARED_POINTER(ReclaimRatiosConfig, reclaimRatiosConfig);
    MAKE_POINTER(reclaimRatiosConfig, shared, ReclaimRatiosConfig, "make ReclaimRatiosConfig failed", return,
        RECLAIM_PRIORITY_MIN, RECLAIM_PRIORITY_MAX, MEMCG_MEM_2_ZRAM_RATIO, MEMCG_ZRAM_2_UFS_RATIO,
        MEMCG_REFAULT_THRESHOLD);
    AddReclaimRatiosConfigToSet(reclaimRatiosConfig);
}

bool MemmgrConfigManager::GetXmlLoaded()
{
    return this->xmlLoaded_;
}

void MemmgrConfigManager::ClearExistConfig()
{
    ClearReclaimRatiosConfigSet();
}

bool MemmgrConfigManager::ReadParamFromXml()
{
    std::string path = KernelInterface::GetInstance().JoinPath(XML_PATH, "memmgr_config.xml");
    char absPatch[PATH_MAX] = {0};
    HILOGI(":%{public}s", path.c_str());
    if (path.length() > PATH_MAX ||
        realpath(path.c_str(), absPatch) == NULL) {
            return false;
    }
    if (!CheckPathExist(path.c_str())) {
        HILOGE("bad profile path! path:%{public}s", path.c_str());
        return false;
    }
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        HILOGE("xmlReadFile error!");
        return false;
    }
    ClearExistConfig();
    xmlNodePtr rootNodePtr = xmlDocGetRootElement(docPtr.get());
    this->xmlLoaded_ = ParseXmlRootNode(rootNodePtr);
    return this->xmlLoaded_;
}

bool MemmgrConfigManager::ParseXmlRootNode(const xmlNodePtr &rootNodePtr)
{
    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        std::string name = std::string(reinterpret_cast<const char *>(currNode->name));
        if (name.compare("reclaimConfig") == 0) {
            ParseReclaimConfig(currNode);
            continue;
        }
        if (name.compare("killConfig") == 0) {
            ParseKillConfig(currNode);
            continue;
        }
        if (name.compare("systemMemoryLevelConfig") == 0) {
            ParseSystemMemoryLevelConfig(currNode);
            continue;
        }
        if (name.compare("reclaimPriorityConfig") == 0) {
            ParseReclaimPriorityConfig(currNode);
            continue;
        }
        HILOGW("unknown node :<%{public}s>", name.c_str());
        return false;
    }
    return true;
}

const ReclaimPriorityConfig& MemmgrConfigManager::GetReclaimPriorityConfig()
{
    return reclaimPriorityConfig_;
}

const MemmgrConfigManager::KillLevelsMap& MemmgrConfigManager::GetKillLevelsMap()
{
    return killLevelsMap_;
}

bool MemmgrConfigManager::ParseReclaimPriorityConfig(const xmlNodePtr &rootNodePtr)
{
    if (!CheckNode(rootNodePtr) || !HasChild(rootNodePtr)) {
        return true;
    }
    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        if (!CheckNode(currNode)) {
            return false;
        }
        std::string name = std::string(reinterpret_cast<const char *>(currNode->name));
        if (name.compare("killalbeSystemApps") == 0) {
            ParseReclaimPriorityKillableSystemAppsConfig(currNode);
            continue;
        }
        HILOGW("unknown node :<%{public}s>", name.c_str());
        return false;
    }
    return true;
}

bool MemmgrConfigManager::ParseReclaimPriorityKillableSystemAppsConfig(const xmlNodePtr &rootNodePtr)
{
    if (!CheckNode(rootNodePtr) || !HasChild(rootNodePtr)) {
        return true;
    }
    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        if (!CheckNode(currNode)) {
            return false;
        }
        std::string name = std::string(reinterpret_cast<const char *>(currNode->name));
        if (name.compare("killableSysApp") == 0) {
            auto contentPtr = xmlNodeGetContent(currNode);
            if (contentPtr == nullptr) {
                continue;
            }
            std::string value = std::string(reinterpret_cast<char *>(contentPtr));
            HILOGW("read a killable app: %{public}s", value.c_str());
            if (value.size() == 0) {
                HILOGE("read a empty killable app: %{public}s, ignore it!", value.c_str());
                continue;
            }
            reclaimPriorityConfig_.killalbeSystemApps_.insert(value);
            xmlFree(contentPtr);
            continue;
        }
        HILOGW("unknown node :<%{public}s>", name.c_str());
        return false;
    }
    return true;
}

bool MemmgrConfigManager::ParseSystemMemoryLevelConfig(const xmlNodePtr &rootNodePtr)
{
    if (!CheckNode(rootNodePtr) || !HasChild(rootNodePtr)) {
        return true;
    }

    std::map<std::string, std::string> param;
    if (!GetModuleParam(rootNodePtr, param)) {
        HILOGW("Get module param failed.");
        return false;
    }

    unsigned int moderate = 0;
    unsigned int low = 0;
    unsigned int critical = 0;

    SetUnsignedIntParam(param, "moderate", moderate);
    SetUnsignedIntParam(param, "low", low);
    SetUnsignedIntParam(param, "critical", critical);

    /* check xml input */
    if (UINT_MAX >= (moderate * KB_PER_MB) && moderate > low && low > critical && critical > 0) {
        /* change MB to KB */
        moderate *= KB_PER_MB;
        low *= KB_PER_MB;
        critical *= KB_PER_MB;
        HILOGI("Use xml values %{public}u %{public}u %{public}u", moderate, low, critical);
    } else {
        moderate = MEMORY_LEVEL_MODERATE_KB_DEFAULT;
        low = MEMORY_LEVEL_LOW_KB_DEFAULT;
        critical = MEMORY_LEVEL_CRITICAL_KB_DEFAULT;
        HILOGW("Use default values instead of invalid values.");
    }

    MAKE_POINTER(systemMemoryLevelConfig_, shared, SystemMemoryLevelConfig,
        "make SystemMemoryLevelConfig failed", return false, moderate, low, critical);
    return true;
}

bool MemmgrConfigManager::ParseKillConfig(const xmlNodePtr &rootNodePtr)
{
    if (!CheckNode(rootNodePtr) || !HasChild(rootNodePtr)) {
        return true;
    }
    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        std::map<std::string, std::string> param;
        GetModuleParam(currNode, param);
        if (!ParseKillLevelNode(currNode, param) || killLevelsMap_.size() > KILL_LEVEL_ITEMS__MAX_COUNT) {
            /* if has error, clear set */
            killLevelsMap_.clear();
            break;
        }
    }
    if (killLevelsMap_.empty()) {
        return false;
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
    return !killLevelsMap_.empty();
}

bool MemmgrConfigManager::ParseKillLevelNode(const xmlNodePtr &currNodePtr,
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
    unsigned int memoryMB = 0;
    int minPriority = RECLAIM_PRIORITY_MAX + 1;
    SetUnsignedIntParam(param, "memoryMB", memoryMB);
    SetIntParam(param, "minPriority", minPriority);
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

bool MemmgrConfigManager::ParseReclaimConfig(const xmlNodePtr &rootNodePtr)
{
    if (!CheckNode(rootNodePtr) || !HasChild(rootNodePtr)) {
        return true;
    }
    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        std::map<std::string, std::string> param;
        GetModuleParam(currNode, param);
        SetReclaimParam(currNode, param);
    }
    HILOGW(" set size :<%{public}zu>", this->reclaimRatiosConfigSet_.size());
    return true;
}

bool MemmgrConfigManager::GetModuleParam(const xmlNodePtr &rootNodePtr, std::map<std::string, std::string> &param)
{
    for (xmlNodePtr currNode = rootNodePtr->xmlChildrenNode; currNode != nullptr; currNode = currNode->next) {
        if (!CheckNode(currNode)) {
            return false;
        }
        std::string key = std::string(reinterpret_cast<const char *>(currNode->name));
        auto contentPtr = xmlNodeGetContent(currNode);
        std::string value;
        if (contentPtr != nullptr) {
            value = std::string(reinterpret_cast<char *>(contentPtr));
            xmlFree(contentPtr);
        }
        param.insert(std::pair<std::string, std::string>(key, value));
        HILOGI("key:<%{public}s>, value:<%{public}s>", key.c_str(), value.c_str());
    }
    return true;
}

void MemmgrConfigManager::SetIntParam(std::map<std::string, std::string> &param, std::string key, int &dst)
{
    std::map<std::string, std::string>::iterator iter = param.find(key);
    if (iter != param.end() && (iter->second).size() > 0) {
        try {
            dst = std::stoi(iter->second);
            return;
        } catch (std::out_of_range&) {
            HILOGW("stoi() failed: out_of_range");
            return;
        } catch (std::invalid_argument&) {
            HILOGW("stoi() failed: invalid_argument");
            return;
        }
    }
    HILOGW("find param failed key:<%{public}s>", key.c_str());
}

void MemmgrConfigManager::SetUnsignedIntParam(std::map<std::string, std::string> &param,
                                              std::string key, unsigned int &dst)
{
    std::map<std::string, std::string>::iterator iter = param.find(key);
    if (iter != param.end() && (iter->second).size() > 0) {
        try {
            int src = std::stoi(iter->second);
            if (src > 0) {
                dst = (unsigned int) src;
                return;
            }
        } catch (std::out_of_range&) {
            HILOGW("stoi() failed: out_of_range");
            return;
        } catch (std::invalid_argument&) {
            HILOGW("stoi() failed: invalid_argument");
            return;
        }
    }
    HILOGW("find param failed key:<%{public}s>", key.c_str());
}

bool MemmgrConfigManager::SetReclaimParam(const xmlNodePtr &currNodePtr,
    std::map<std::string, std::string> &param)
{
    std::string name = std::string(reinterpret_cast<const char *>(currNodePtr->name));
    if (name.compare("availbufferSize") == 0) {
        return SetAvailBufferConfig(param);
    }
    if (name.compare("ZswapdParam") == 0) {
        return SetZswapdParamConfig(param);
    }
    HILOGW("unknown node :<%{public}s>", name.c_str());
    return false;
}

bool MemmgrConfigManager::SetAvailBufferConfig(std::map<std::string, std::string> &param)
{
    unsigned int availBuffer = AVAIL_BUFFER;
    unsigned int minAvailBuffer = MIN_AVAIL_BUFFER;
    unsigned int highAvailBuffer = HIGH_AVAIL_BUFFER;
    unsigned int swapReserve = SWAP_RESERVE;

    SetUnsignedIntParam(param, "availBuffer", availBuffer);
    SetUnsignedIntParam(param, "minAvailBuffer", minAvailBuffer);
    SetUnsignedIntParam(param, "highAvailBuffer", highAvailBuffer);
    SetUnsignedIntParam(param, "swapReserve", swapReserve);

    MAKE_POINTER(this->availBufferSize_, shared, AvailBufferSize, "make AvailBufferSize failed", return false,
        availBuffer, minAvailBuffer, highAvailBuffer, swapReserve);
    return true;
}

bool MemmgrConfigManager::SetZswapdParamConfig(std::map<std::string, std::string> &param)
{
    std::map<std::string, std::string>::iterator iter;
    int minScore = 0;
    int maxScore = RECLAIM_PRIORITY_MAX;
    unsigned int mem2zramRatio = MEMCG_MEM_2_ZRAM_RATIO;
    unsigned int zram2ufsRatio = MEMCG_ZRAM_2_UFS_RATIO;
    unsigned int refaultThreshold = MEMCG_REFAULT_THRESHOLD;

    SetIntParam(param, "minScore", minScore);
    SetIntParam(param, "maxScore", maxScore);
    SetUnsignedIntParam(param, "mem2zramRatio", mem2zramRatio);
    SetUnsignedIntParam(param, "zram2ufsRatio", zram2ufsRatio);
    SetUnsignedIntParam(param, "refaultThreshold", refaultThreshold);

    DECLARE_SHARED_POINTER(ReclaimRatiosConfig, reclaimRatiosConfig);
    MAKE_POINTER(reclaimRatiosConfig, shared, ReclaimRatiosConfig, "make ReclaimRatiosConfig failed", return false,
        minScore, maxScore, mem2zramRatio, zram2ufsRatio, refaultThreshold);
    AddReclaimRatiosConfigToSet(reclaimRatiosConfig);
    return true;
}

bool MemmgrConfigManager::CheckNode(const xmlNodePtr &nodePtr)
{
    if (nodePtr != nullptr && nodePtr->name != nullptr &&
        (nodePtr->type == XML_ELEMENT_NODE || nodePtr->type == XML_TEXT_NODE)) {
        return true;
    }
    return false;
}

bool MemmgrConfigManager::CheckPathExist(const char *path)
{
    std::ifstream profileStream(path);
    return profileStream.good();
}

bool MemmgrConfigManager::HasChild(const xmlNodePtr &rootNodePtr)
{
    return xmlChildElementCount(rootNodePtr) > 0;
}

void MemmgrConfigManager::AddReclaimRatiosConfigToSet(std::shared_ptr<ReclaimRatiosConfig> reclaimRatiosConfig)
{
    this->reclaimRatiosConfigSet_.insert(reclaimRatiosConfig);
}

void MemmgrConfigManager::ClearReclaimRatiosConfigSet()
{
    this->reclaimRatiosConfigSet_.clear();
}

bool MemmgrConfigManager::WriteReclaimRatiosConfigToKernel()
{
    std::string path = KernelInterface::GetInstance().JoinPath(MEMCG_PATH, "memory.zswapd_memcgs_param");
    std::string content;

    unsigned int paramNum = this->reclaimRatiosConfigSet_.size();
    content = std::to_string(paramNum);
    for (auto i = this->reclaimRatiosConfigSet_.begin(); i != this->reclaimRatiosConfigSet_.end(); ++i) {
        content += " " + std::to_string((*i)->minScore);
        content += " " + std::to_string((*i)->maxScore);
        content += " " + std::to_string((*i)->mem2zramRatio);
        content += " " + std::to_string((*i)->zram2ufsRatio);
        content += " " + std::to_string((*i)->refaultThreshold);
    }

    HILOGI(" : <%{public}s>", content.c_str());
    return KernelInterface::GetInstance().WriteToFile(path, content);
}

std::shared_ptr<AvailBufferSize> MemmgrConfigManager::GetAvailBufferSize()
{
    return this->availBufferSize_;
}

const MemmgrConfigManager::ReclaimRatiosConfigSet MemmgrConfigManager::GetReclaimRatiosConfigSet()
{
    return this->reclaimRatiosConfigSet_;
}

std::shared_ptr<SystemMemoryLevelConfig> MemmgrConfigManager::GetSystemMemoryLevelConfig()
{
    return systemMemoryLevelConfig_;
}
} // namespace Memory
} // namespace OHOS
