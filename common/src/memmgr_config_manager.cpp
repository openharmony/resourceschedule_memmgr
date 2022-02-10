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
#include "memmgr_log.h"
#include "memmgr_config_manager.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "MemmgrConfigManager";
    const std::string XML_PATH = "/etc/xml/";
    const std::string MEMCG_PATH = KernelInterface::MEMCG_BASE_PATH;
} // namespace
IMPLEMENT_SINGLE_INSTANCE(MemmgrConfigManager);

bool MemmgrConfigManager::Init()
{
    ReadParamFromXml();
    WriteReclaimRatiosConfigToKernel();
    return this->XmlLoaded;
}

MemmgrConfigManager::MemmgrConfigManager()
{
}

MemmgrConfigManager::~MemmgrConfigManager()
{
}

AvailBufferSize::AvailBufferSize(int availBuffer, int minAvailBuffer, int highAvailBuffer, int swapReserve)
    : availBuffer(availBuffer),
      minAvailBuffer(minAvailBuffer),
      highAvailBuffer(highAvailBuffer),
      swapReserve(swapReserve) {};
ReclaimRatiosConfig::ReclaimRatiosConfig(int minScore, int maxScore, int mem2zramRatio, int zran2ufsRation,
    int refaultThreshold) : minScore(minScore), maxScore(maxScore), mem2zramRatio(mem2zramRatio),
    zran2ufsRation(zran2ufsRation), refaultThreshold(refaultThreshold)
{
}

bool MemmgrConfigManager::GetXmlLoaded()
{
    return this->XmlLoaded;
}

void MemmgrConfigManager::ClearExistConfig()
{
    ClearReclaimRatiosConfigSet();
}

bool MemmgrConfigManager::ReadParamFromXml()
{
    std::string path = KernelInterface::GetInstance().JoinPath(XML_PATH, "memmgr_config.xml");
    HILOGI(":%{public}s", path.c_str());
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
    XmlLoaded = ParseXmlRootNode(rootNodePtr);
    return XmlLoaded;
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
        HILOGW("unknow node :<%{public}s>", name.c_str());
        return false;
    }
    return true;
}

bool MemmgrConfigManager::ParseKillConfig(const xmlNodePtr &rootNodePtr)
{
    HILOGI("Todo:parseKillConfig");
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

void MemmgrConfigManager::SetParam(std::map<std::string, std::string> &param, std::string key, int* dst)
{
    std::map<std::string, std::string>::iterator iter = param.find(key);
    if (iter != param.end()) {
        *dst = stoi(iter->second);
        return;
    }
    HILOGW("find param failed key:<%{public}s>", key.c_str());
}

bool MemmgrConfigManager::SetReclaimParam(const xmlNodePtr &currNodePtr,
    std::map<std::string, std::string> &param)
{
    std::string name = std::string(reinterpret_cast<const char *>(currNodePtr->name));
    if (name.compare("availbuffer") == 0) {
        return SetAvailBufferConfig(param);
    }
    if (name.compare("ZswapdParam") == 0) {
        return SetZswapdParamConfig(param);
    }
    HILOGW("unknow node :<%{public}s>", name.c_str());
    return false;
}

bool MemmgrConfigManager::SetAvailBufferConfig(std::map<std::string, std::string> &param)
{
    int availBuffer = 800; // default availBuffer 800MB
    int minAvailBuffer = 750; // default minAvailBuffer 750MB
    int highAvailBuffer = 850; // default highAvailBuffer 850MB
    int swapReserve = 200; // default swapReserve 200MB

    SetParam(param, "availBuffer", &availBuffer);
    SetParam(param, "minAvailBuffer", &minAvailBuffer);
    SetParam(param, "highAvailBuffer", &highAvailBuffer);
    SetParam(param, "swapReserve", &swapReserve);
    delete this->availBufferSize;
    this->availBufferSize = new AvailBufferSize(availBuffer, minAvailBuffer, highAvailBuffer, swapReserve);
    return true;
}

bool MemmgrConfigManager::SetZswapdParamConfig(std::map<std::string, std::string> &param)
{
    std::map<std::string, std::string>::iterator iter;
    int minScore = 0;
    int maxScore = 1000; // default maxAppscore 1000
    int mem2zramRatio = 40; // default mem2zramRatio 40%
    int zran2ufsRation = 0;
    int refaultThreshold = 0;

    SetParam(param, "minScore", &minScore);
    SetParam(param, "maxScore", &maxScore);
    SetParam(param, "mem2zramRatio", &mem2zramRatio);
    SetParam(param, "zran2ufsRation", &zran2ufsRation);
    SetParam(param, "refaultThreshold", &refaultThreshold);

    ReclaimRatiosConfig *reclaimRatiosConfig =
        new ReclaimRatiosConfig(minScore, maxScore, mem2zramRatio, zran2ufsRation, refaultThreshold);
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

void MemmgrConfigManager::AddReclaimRatiosConfigToSet(ReclaimRatiosConfig *reclaimRatiosConfig)
{
    this->reclaimRatiosConfigSet.insert(reclaimRatiosConfig);
}

void MemmgrConfigManager::ClearReclaimRatiosConfigSet()
{
    for (auto i = reclaimRatiosConfigSet.begin(); i != reclaimRatiosConfigSet.end(); ++i) {
        delete *i;
    }
    this->reclaimRatiosConfigSet.clear();
}

bool MemmgrConfigManager::WriteReclaimRatiosConfigToKernel()
{
    std::string path = KernelInterface::GetInstance().JoinPath(MEMCG_PATH, "memory.zswapd_memcgs_param");
    std::string content;
    int paramNum = this->reclaimRatiosConfigSet.size();
    content += std::to_string(paramNum);
    for (auto i = reclaimRatiosConfigSet.begin(); i != reclaimRatiosConfigSet.end(); ++i) {
        content += " " + std::to_string((*i)->minScore);
        content += " " + std::to_string((*i)->maxScore);
        content += " " + std::to_string((*i)->mem2zramRatio);
        content += " " + std::to_string((*i)->zran2ufsRation);
        content += " " + std::to_string((*i)->refaultThreshold);
    }
    return KernelInterface::GetInstance().WriteToFile(path, content, false);
}

AvailBufferSize *MemmgrConfigManager::GetAvailBufferSize()
{
    return this->availBufferSize;
}

const MemmgrConfigManager::ReclaimRatiosConfigSet MemmgrConfigManager::GetReclaimRatiosConfigSet()
{
    return this->reclaimRatiosConfigSet;
}
} // namespace Memory
} // namespace OHOS
