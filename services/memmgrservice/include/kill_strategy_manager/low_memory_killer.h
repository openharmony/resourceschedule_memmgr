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

#ifndef OHOS_MEMORY_MEMMGR_LOW_MEMORY_KILLER_H
#define OHOS_MEMORY_MEMMGR_LOW_MEMORY_KILLER_H

#include "event_handler.h"
#include "single_instance.h"

namespace OHOS {
namespace Memory {
class  LowMemoryKiller {
    DECLARE_SINGLE_INSTANCE_BASE(LowMemoryKiller);

public:
    void PsiHandler();
    int32_t GetKillLevel();
private:
    LowMemoryKiller();
    ~LowMemoryKiller() = default;
    std::pair<unsigned int, int> QueryKillMemoryPriorityPair(unsigned int currBufferKB,
            unsigned int &targetBufKB, int &killLevel);
    void PsiHandlerInner();
    int KillOneBundleByPrio(int minPrio);
    bool GetEventHandler();
    std::shared_ptr<AppExecFwk::EventHandler> handler_;

    bool initialized_ = false;
    long calledCount = 0;
    int32_t killLevel_ = 0;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_LOW_MEMORY_KILLER_H
