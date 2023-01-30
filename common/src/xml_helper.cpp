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

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "XmlHelper";
} // namespace

bool XmlHelper::CheckNode(const xmlNodePtr &nodePtr)
{
    if (nodePtr != nullptr && nodePtr->name != nullptr &&
        (nodePtr->type == XML_ELEMENT_NODE || nodePtr->type == XML_TEXT_NODE)) {
        return true;
    }
    return false;
}

bool XmlHelper::CheckPathExist(const char *path)
{
    std::ifstream profileStream(path);
    return profileStream.good();
}

bool XmlHelper::HasChild(const xmlNodePtr &rootNodePtr)
{
    return xmlChildElementCount(rootNodePtr) > 0;
}

void XmlHelper::SetIntParam(std::map<std::string, std::string> &param,
                            std::string key, int &dst, int defaultValue)
{
    dst = defaultValue;
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

void XmlHelper::SetUnsignedIntParam(std::map<std::string, std::string> &param,
                                    std::string key, unsigned int &dst, unsigned int defaultValue)
{
    dst = defaultValue;
    std::map<std::string, std::string>::iterator iter = param.find(key);
    if (iter != param.end() && (iter->second).size() > 0) {
        try {
            unsigned long src = std::stoul(iter->second);
            dst = static_cast<unsigned int>(src);
            return;
        } catch (std::out_of_range&) {
            HILOGW("stoul() failed: out_of_range");
            return;
        } catch (std::invalid_argument&) {
            HILOGW("stoul() failed: invalid_argument");
            return;
        }
    }
    HILOGW("find param failed key:<%{public}s>", key.c_str());
}

bool XmlHelper::GetModuleParam(const xmlNodePtr &rootNodePtr, std::map<std::string, std::string> &param)
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

bool XmlHelper::ParseUnsignedLongLongContent(const xmlNodePtr &rootNodePtr, unsigned long long &value)
{
    try {
        auto contentPtr = xmlNodeGetContent(rootNodePtr);
        std::string valueStr;
        if (contentPtr != nullptr) {
            valueStr = std::string(reinterpret_cast<char *>(contentPtr));
            xmlFree(contentPtr);
            value = std::strtoull(valueStr.c_str(), nullptr, 10); // 10:Decimal
            return true;
        }
    } catch (...) {
        return false;
    }
    return false;
}

void XmlHelper::SetStringParam(std::map<std::string, std::string> &param,
                                    std::string key, std::string &dst, std::string defaultValue)
{
    HILOGI("called");
    dst = defaultValue;
    std::map<std::string, std::string>::iterator iter = param.find(key);
    if (iter != param.end() && (iter->second).size() > 0) {
        dst = iter->second;
        return;
    }
    HILOGW("find param failed key:<%{public}s>", key.c_str());
}
} // namespace Memory
} // namespace OHOS
