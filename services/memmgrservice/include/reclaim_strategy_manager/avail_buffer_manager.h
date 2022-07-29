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

#ifndef OHOS_MEMORY_MEMCG_AVAIL_BUFFER_MANAGER_H
#define OHOS_MEMORY_MEMCG_AVAIL_BUFFER_MANAGER_H

#include "single_instance.h"
#include "event_handler.h"
#include "reclaim_strategy_constants.h"
#include "memmgr_config_manager.h"

namespace OHOS {
namespace Memory {
class AvailBufferManager {
DECLARE_SINGLE_INSTANCE_BASE(AvailBufferManager)
public:
    bool LoadAvailBufferConfig();
    bool WriteAvailBufferToKernel();
    bool SetAvailBuffer(std::shared_ptr<AvailBufferConfig> availBuffer);
    bool LoadAvailBufferFromConfig();
    void CloseZswapd();
    void InitAvailBuffer();
    bool Init();
    std::string NumsToString();

    inline bool initialized()
    {
        return initialized_;
    };

private:
    bool initialized_ = false;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    unsigned int availBuffer_ = AVAIL_BUFFER; // default availBuffer 800MB
    unsigned int minAvailBuffer_ = MIN_AVAIL_BUFFER; // default minAvailBuffer 750MB
    unsigned int highAvailBuffer_ = HIGH_AVAIL_BUFFER; // default highAvailBuffer 850MB
    unsigned int swapReserve_ = SWAP_RESERVE; // default swapReserve 200MB
    bool zramEnable_ = false;
    unsigned int memTotal_ = 0;
    AvailBufferManager();
    ~AvailBufferManager();
    bool GetEventHandler();
    void UpdateZramEnableFromKernel();
    bool CheckAvailBuffer(std::shared_ptr<AvailBufferConfig> availBuffer);
    void UpdateMemTotalFromKernel();
};
} // namespace Memory
} // namespace OHOS
#endif
