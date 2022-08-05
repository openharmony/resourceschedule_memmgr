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

#ifndef OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_XML_HELPER_H
#define OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_XML_HELPER_H

#include <map>
#include <string>
#include <set>
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "kernel_interface.h"
#include "single_instance.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"

namespace OHOS {
namespace Memory {
class XmlHelper {
public:
static bool CheckNode(const xmlNodePtr &nodePtr);
static bool CheckPathExist(const char *path);
static bool HasChild(const xmlNodePtr &rootNodePtr);
static void SetIntParam(std::map<std::string, std::string> &param,
                        std::string key, int &dst, int defaultValue);
static void SetUnsignedIntParam(std::map<std::string, std::string> &param,
                                std::string key, unsigned int &dst, unsigned int defaultValue);
static bool GetModuleParam(const xmlNodePtr &rootNodePtr, std::map<std::string, std::string> &param);
static bool ParseUnsignedLongLongContent(const xmlNodePtr &rootNodePtr, unsigned long long &value);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_XML_HELPER_H
