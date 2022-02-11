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

#include "avail_buffer_manager.h"
#include "memmgr_log.h"
#include "kernel_interface.h"
#include "memmgr_config_manager.h"

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
    CloseZswapd();
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
        handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
        if (handler_ == nullptr) {
            HILOGE("handler init failed");
            return false;
        }
    }
    return true;
}

bool AvailBufferManager::LoadAvailBufferFromConfig()
{
    AvailBufferSize *availBuffer = MemmgrConfigManager::GetInstance().GetAvailBufferSize();
    this->availBuffer = availBuffer->availBuffer;
    this->minAvailBuffer = availBuffer->minAvailBuffer;
    this->highAvailBuffer = availBuffer->highAvailBuffer;
    this->swapReserve = availBuffer->swapReserve;
    WriteAvailBufferToKernel();
    return true;
}

bool AvailBufferManager::SetAvailBuffer(int availBuffer, int minAvailBuffer, int highAvailBuffer, int swapReserve)
{
    this->availBuffer = availBuffer;
    this->minAvailBuffer = minAvailBuffer;
    this->highAvailBuffer = highAvailBuffer;
    this->swapReserve = swapReserve;
    HILOGI("=%{public}d,minAvailBuffer = %{public}d,highAvailBuffer = %{public}d,swapReserve = %{public}d",
           availBuffer, minAvailBuffer, highAvailBuffer, swapReserve);
    return WriteAvailBufferToKernel();
}

inline std::string AvailBufferManager::NumsToString()
{
    std::string ret = std::to_string(this->availBuffer) + " "
        + std::to_string(this->minAvailBuffer) + " "
        + std::to_string(this->highAvailBuffer) + " "
        + std::to_string(this->swapReserve);
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
    HILOGI("Zswapd close now");
    SetAvailBuffer(0, 0, 0, 0);
}
} // namespace Memory
} // namespace OHOS
