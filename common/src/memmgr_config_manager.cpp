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
#include <string>

#include "xml_helper.h"
#include "memmgr_config_manager.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "MemmgrConfigManager";
    const std::string XML_PATH = "/etc/memmgr/";
    const std::string MEMCG_PATH = KernelInterface::MEMCG_BASE_PATH;
} // namespace
IMPLEMENT_SINGLE_INSTANCE(MemmgrConfigManager);

bool MemmgrConfigManager::Init()
{
    killConfig_.ClearKillLevelsMap();
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
    ClearReclaimConfigSet();
    killConfig_.ClearKillLevelsMap();
}

void MemmgrConfigManager::InitDefaultConfig()
{
    availBufferConfig_.SetDefaultConfig(AVAIL_BUFFER, MIN_AVAIL_BUFFER, HIGH_AVAIL_BUFFER, SWAP_RESERVE);
    reclaimConfig_.SetDefaultConfig(RECLAIM_PRIORITY_MIN, RECLAIM_PRIORITY_MAX,
                                    MEMCG_MEM_2_ZRAM_RATIO, MEMCG_ZRAM_2_UFS_RATIO, MEMCG_REFAULT_THRESHOLD);
}

bool MemmgrConfigManager::GetXmlLoaded()
{
    return xmlLoaded_;
}

void MemmgrConfigManager::ClearExistConfig()
{
    ClearReclaimConfigSet();
}

bool MemmgrConfigManager::ReadParamFromXml()
{
    std::string path = KernelInterface::GetInstance().JoinPath(XML_PATH, "memmgr_config.xml");
    char absPatch[PATH_MAX] = {0};
    HILOGI(":%{public}s", path.c_str());
    if (path.length() > PATH_MAX || realpath(path.c_str(), absPatch) == NULL) {
            return false;
    }
    if (!XmlHelper::CheckPathExist(path.c_str())) {
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
        if (name.compare("availBufferConfig") == 0) {
            availBufferConfig_.ParseConfig(currNode);
            continue;
        }

        if (name.compare("reclaimConfig") == 0) {
            reclaimConfig_.ParseConfig(currNode);
            continue;
        }

        if (name.compare("killConfig") == 0) {
            killConfig_.ParseConfig(currNode);
            continue;
        }
        if (name.compare("systemMemoryLevelConfig") == 0) {
            systemMemoryLevelConfig_.ParseConfig(currNode);
            continue;
        }
        if (name.compare("reclaimPriorityConfig") == 0) {
            reclaimPriorityConfig_.ParseConfig(currNode);
            continue;
        }
        if (name.compare("nandLifeConfig") == 0) {
            nandLifeConfig_.ParseConfig(currNode);
            continue;
        }
        if (name.compare("switchConfig") == 0) {
            switchConfig_.ParseConfig(currNode);
            continue;
        }
        HILOGW("unknown node :<%{public}s>", name.c_str());
        return false;
    }
    return true;
}

void MemmgrConfigManager::ClearReclaimConfigSet()
{
    reclaimConfig_.ClearReclaimConfigSet();
}

const NandLifeConfig &MemmgrConfigManager::GetNandLifeConfig()
{
    return nandLifeConfig_;
}

const SwitchConfig &MemmgrConfigManager::GetSwitchConfig()
{
    return switchConfig_;
}

const ReclaimPriorityConfig& MemmgrConfigManager::GetReclaimPriorityConfig()
{
    return reclaimPriorityConfig_;
}

const KillConfig::KillLevelsMap& MemmgrConfigManager::GetKillLevelsMap()
{
    return killConfig_.GetKillLevelsMap();
}

bool MemmgrConfigManager::WriteReclaimRatiosConfigToKernel()
{
    std::string path = KernelInterface::GetInstance().JoinPath(MEMCG_PATH, "memory.zswapd_memcgs_param");
    std::string content;

    unsigned int paramNum = reclaimConfig_.GetReclaimConfigSet().size();
    content = std::to_string(paramNum);
    for (auto i = reclaimConfig_.GetReclaimConfigSet().begin(); i != reclaimConfig_.GetReclaimConfigSet().end(); ++i) {
        content += " " + std::to_string((*i)->GetMinScore());
        content += " " + std::to_string((*i)->GetMaxScore());
        content += " " + std::to_string((*i)->GetMem2zramRatio());
        content += " " + std::to_string((*i)->GetZram2ufsRatio());
        content += " " + std::to_string((*i)->GetRefaultThreshold());
    }

    HILOGI("Write to kernel: <%{public}s>", content.c_str());
    return KernelInterface::GetInstance().WriteToFile(path, content);
}

AvailBufferConfig MemmgrConfigManager::GetAvailBufferConfig()
{
    return availBufferConfig_;
}

const ReclaimConfig::ReclaimConfigSet& MemmgrConfigManager::GetReclaimConfigSet()
{
    return reclaimConfig_.GetReclaimConfigSet();
}

const KillConfig& MemmgrConfigManager::GetKillConfig()
{
    return killConfig_;
}

SystemMemoryLevelConfig MemmgrConfigManager::GetSystemMemoryLevelConfig()
{
    return systemMemoryLevelConfig_;
}

void MemmgrConfigManager::Dump(int fd)
{
    availBufferConfig_.Dump(fd);
    killConfig_.Dump(fd);
    reclaimConfig_.Dump(fd);
    nandLifeConfig_.Dump(fd);
    systemMemoryLevelConfig_.Dump(fd);
    switchConfig_.Dump(fd);
    reclaimPriorityConfig_.Dump(fd);
}
} // namespace Memory
} // namespace OHOS
