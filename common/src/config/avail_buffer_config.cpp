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
#include "memmgr_log.h"
#include "xml_helper.h"
#include "reclaim_strategy_constants.h"
#include "avail_buffer_config.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "AvailBufferConfig";
}

AvailBufferConfig::AvailBufferConfig()
{
}

AvailBufferConfig::AvailBufferConfig(unsigned int availBuffer, unsigned int minAvailBuffer,
                                     unsigned int highAvailBuffer, unsigned int swapReserve)
    : availBuffer_(availBuffer),
      minAvailBuffer_(minAvailBuffer),
      highAvailBuffer_(highAvailBuffer),
      swapReserve_(highAvailBuffer)
{
}

void AvailBufferConfig::SetDefaultConfig(unsigned int availBuffer, unsigned int minAvailBuffer,
                                         unsigned int highAvailBuffer, unsigned int swapReserve)
{
    SetAvailBuffer(availBuffer);
    SetMinAvailBuffer(minAvailBuffer);
    SetHighAvailBuffer(highAvailBuffer);
    SetSwapReserve(swapReserve);
}

void AvailBufferConfig::ParseConfig(const xmlNodePtr &rootNodePtr)
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
    SetAvailBufferConfig(param);
}

void AvailBufferConfig::SetAvailBufferConfig(std::map<std::string, std::string> &param)
{
    XmlHelper::SetUnsignedIntParam(param, "availBuffer", availBuffer_, AVAIL_BUFFER);
    XmlHelper::SetUnsignedIntParam(param, "minAvailBuffer", minAvailBuffer_, MIN_AVAIL_BUFFER);
    XmlHelper::SetUnsignedIntParam(param, "highAvailBuffer", highAvailBuffer_, HIGH_AVAIL_BUFFER);
    XmlHelper::SetUnsignedIntParam(param, "swapReserve", swapReserve_, SWAP_RESERVE);
}

unsigned int AvailBufferConfig::GetAvailBuffer()
{
    return availBuffer_;
}

void AvailBufferConfig::SetAvailBuffer(unsigned int availBuffer)
{
    availBuffer_ = availBuffer;
}

unsigned int AvailBufferConfig::GetMinAvailBuffer()
{
    return minAvailBuffer_;
}

void AvailBufferConfig::SetMinAvailBuffer(unsigned int minAvailBuffer)
{
    minAvailBuffer_ = minAvailBuffer;
}

unsigned int AvailBufferConfig::GetHighAvailBuffer()
{
    return highAvailBuffer_;
}

void AvailBufferConfig::SetHighAvailBuffer(unsigned int highAvailBuffer)
{
    highAvailBuffer_ = highAvailBuffer;
}

unsigned int AvailBufferConfig::GetSwapReserve()
{
    return swapReserve_;
}

void AvailBufferConfig::SetSwapReserve(unsigned int swapReserve)
{
    swapReserve_ = swapReserve;
}

void AvailBufferConfig::Dump(int fd)
{
    dprintf(fd, "AvailBufferConfig:   \n");
    dprintf(fd, "                      availBuffer: %u\n", availBuffer_);
    dprintf(fd, "                   minAvailBuffer: %u\n", minAvailBuffer_);
    dprintf(fd, "                  highAvailBuffer: %u\n", highAvailBuffer_);
    dprintf(fd, "                      swapReserve: %u\n", swapReserve_);
}
} // namespace Memory
} // namespace OHOS
