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

namespace OHOS {
namespace Memory {
class AvailBufferManager {
DECLARE_SINGLE_INSTANCE_BASE(AvailBufferManager)
public:
    bool LoadAvailBufferConfig();
    bool WriteAvailBufferToKernel();
    bool SetAvailBuffer(int availBuffer, int minAvailBuffer, int highAvailBuffer, int swapReserve);
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
    int availBuffer = AVAIL_BUFFER; // default availBuffer 800MB
    int minAvailBuffer = MIN_AVAIL_BUFFER; // default minAvailBuffer 750MB
    int highAvailBuffer = HIGH_AVAIL_BUFFER; // default highAvailBuffer 850MB
    int swapReserve = SWAP_RESERVE; // default swapReserve 200MB
    bool zramEnable = false;
    int memTotal = 0;
    AvailBufferManager();
    ~AvailBufferManager();
    bool GetEventHandler();
    void UpdateZramEnableFromKernel();
    void UpdateMemTotalFromKernel();
};
} // namespace Memory
} // namespace OHOS
#endif
