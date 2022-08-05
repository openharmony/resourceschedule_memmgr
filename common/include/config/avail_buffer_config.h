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

#ifndef OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_CONFIG_AVAIL_BUFFER_CONFIG_H
#define OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_CONFIG_AVAIL_BUFFER_CONFIG_H

#include <map>
#include <string>
#include <set>
#include "event_handler.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"

namespace OHOS {
namespace Memory {
class AvailBufferConfig {
public:
    unsigned int GetAvailBuffer();
    void SetAvailBuffer(unsigned int availBuffer);
    unsigned int GetMinAvailBuffer();
    void SetMinAvailBuffer(unsigned int minAvailBuffer);
    unsigned int GetHighAvailBuffer();
    void SetHighAvailBuffer(unsigned int highAvailBuffer);
    unsigned int GetSwapReserve();
    void SetSwapReserve(unsigned int swapReserve);
    AvailBufferConfig();
    AvailBufferConfig(unsigned int availBuffer, unsigned int minAvailBuffer,
                      unsigned int highAvailBuffer, unsigned int swapReserve);
    void SetDefaultConfig(unsigned int availBuffer, unsigned int minAvailBuffer,
                          unsigned int highAvailBuffer, unsigned int swapReserve);
    void ParseConfig(const xmlNodePtr &rootNodePtr);
    void SetAvailBufferConfig(std::map<std::string, std::string> &param);

private:
    unsigned int availBuffer_;
    unsigned int minAvailBuffer_;
    unsigned int highAvailBuffer_;
    unsigned int swapReserve_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_CONFIG_AVAIL_BUFFER_CONFIG_H