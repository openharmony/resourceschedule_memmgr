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

#include <regex>

#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "kernel_interface.h"
#include "avail_buffer_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "AvailBufferManager";
const std::string BUFFER_PATH = KernelInterface::MEMCG_BASE_PATH;
}
IMPLEMENT_SINGLE_INSTANCE(AvailBufferManager);

AvailBufferManager::AvailBufferManager()
{
}

AvailBufferManager::~AvailBufferManager()
{
}

bool AvailBufferManager::Init()
{
    initialized_ = GetEventHandler();
    UpdateMemTotalFromKernel();
    InitAvailBuffer();
    if (initialized_) {
        HILOGI("init successed");
    } else {
        HILOGE("init failed");
    }
    return initialized_;
}

bool AvailBufferManager::GetEventHandler()
{
    if (!handler_) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

bool AvailBufferManager::LoadAvailBufferFromConfig()
{
    std::shared_ptr<AvailBufferSize> availBuffer = MemmgrConfigManager::GetInstance().GetAvailBufferSize();
    SetAvailBuffer(availBuffer);
    return true;
}

bool AvailBufferManager::CheckAvailBuffer(std::shared_ptr<AvailBufferSize> availBuffer)
{
    if (availBuffer->availBuffer > this->memTotal_ || availBuffer->minAvailBuffer > this->memTotal_ ||
        availBuffer->highAvailBuffer > this->memTotal_ || availBuffer->swapReserve > this->memTotal_) {
        return false;
    }
    if (availBuffer->availBuffer < availBuffer->minAvailBuffer ||
        availBuffer->availBuffer > availBuffer->highAvailBuffer) {
        return false;
    }
    return true;
}

bool AvailBufferManager::SetAvailBuffer(std::shared_ptr<AvailBufferSize> availBuffer)
{
    if (!CheckAvailBuffer(availBuffer)) {
        HILOGI("size invalid, set failed");
        return false;
    }
    this->availBuffer_ = availBuffer->availBuffer;
    this->minAvailBuffer_ = availBuffer->minAvailBuffer;
    this->highAvailBuffer_ = availBuffer->highAvailBuffer;
    this->swapReserve_ = availBuffer->swapReserve;
    HILOGI("=%{public}d,minAvailBuffer = %{public}d,highAvailBuffer = %{public}d,swapReserve = %{public}d",
           this->availBuffer_, this->minAvailBuffer_, this->highAvailBuffer_, this->swapReserve_);
    return WriteAvailBufferToKernel();
}

inline std::string AvailBufferManager::NumsToString()
{
    std::string ret = std::to_string(this->availBuffer_) + " "
        + std::to_string(this->minAvailBuffer_) + " "
        + std::to_string(this->highAvailBuffer_) + " "
        + std::to_string(this->swapReserve_);
    return ret;
}

bool AvailBufferManager::WriteAvailBufferToKernel()
{
    std::string fullPath = KernelInterface::GetInstance().JoinPath(BUFFER_PATH, "memory.avail_buffers");
    HILOGI("%{public}s", NumsToString().c_str());
    return KernelInterface::GetInstance().WriteToFile(fullPath, NumsToString());
}

void AvailBufferManager::CloseZswapd()
{
    DECLARE_SHARED_POINTER(AvailBufferSize, availBuffer);
    MAKE_POINTER(availBuffer, shared, AvailBufferSize, "make shared failed", return, 0, 0, 0, 0);
    HILOGI("Zswapd close now");
    SetAvailBuffer(availBuffer);
}

void AvailBufferManager::InitAvailBuffer()
{
    UpdateZramEnableFromKernel();
    if (this->zramEnable_) {
        LoadAvailBufferFromConfig();
    } else {
        CloseZswapd();
    }
}

void AvailBufferManager::UpdateZramEnableFromKernel()
{
    std::string content;
    unsigned int swapTotal = 0;
    if (!KernelInterface::GetInstance().ReadFromFile("/proc/meminfo", content)) {
        return;
    }
    content = std::regex_replace(content, std::regex("\n+"), " "); // replace \n with space
    std::regex re("SwapTotal:[[:s:]]*([[:d:]]+) kB[[:s:]]*");
    std::smatch res;
    if (std::regex_search(content, res, re)) {
        try {
        swapTotal = (unsigned int) std::stoi(res.str(1)); // 1: swapTotal index
        } catch (std::out_of_range&) {
            HILOGE("stoi() failed: out_of_range");
        }
        HILOGI(" : %{public}d", swapTotal);
    }

    if (swapTotal != 0) {
        this->zramEnable_ = true;
    } else {
        this->zramEnable_ = false;
    }
}

void AvailBufferManager::UpdateMemTotalFromKernel()
{
    std::string content;
    if (!KernelInterface::GetInstance().ReadFromFile("/proc/meminfo", content)) {
        return;
    }
    content = std::regex_replace(content, std::regex("\n+"), " "); // replace \n with space
    std::regex re("MemTotal:[[:s:]]*([[:d:]]+) kB[[:s:]]*");
    std::smatch res;
    if (std::regex_search(content, res, re)) {
        try {
        this->memTotal_ = (unsigned int) std::stoi(res.str(1)); // 1: memTotal index
        } catch (std::out_of_range&) {
            HILOGE("stoi() failed: out_of_range");
            return;
        }
        HILOGI(" : %{public}d", this->memTotal_);
    }
}
} // namespace Memory
} // namespace OHOS
