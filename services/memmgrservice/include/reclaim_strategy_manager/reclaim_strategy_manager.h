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

#ifndef OHOS_MEMORY_MEMMGR_RECALIM_STRATEGY_MANAGER_H
#define OHOS_MEMORY_MEMMGR_RECALIM_STRATEGY_MANAGER_H

#include "single_instance.h"
#include "event_handler.h"
#include "memcg.h"
#include "reclaim_param.h"

namespace OHOS {
namespace Memory {
class ReclaimStrategyManager {
    DECLARE_SINGLE_INSTANCE_BASE(ReclaimStrategyManager);

public:
    bool Init();
    void NotifyAppStateChanged(std::shared_ptr<ReclaimParam> reclaimPara);
    void NotifyAccountDied(int accountId);
    void NotifyAccountPriorityChanged(int accountId, int priority);

    inline bool Initailized()
    {
        return initialized_;
    };

private:
    bool initialized_ = false;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    Memcg* rootMemcg;
    std::map<int, UserMemcg*> userMemcgsMap; // map<userId, UserMemcg*>

    ReclaimStrategyManager();
    ~ReclaimStrategyManager();
    bool GetEventHandler();

    bool HandleAppStateChanged(std::shared_ptr<ReclaimParam> reclaimPara);
    bool HandleProcessCreate(std::shared_ptr<ReclaimParam> reclaimPara);
    bool GetReclaimRatiosByScore(int score, ReclaimRatios * const ratios);
    bool SetRootMemcgPara();
    void UpdateMemcgReclaimInfo();

    UserMemcg* UserMemcgsAdd(int userId);
    UserMemcg* UserMemcgsRemove(int userId);
    UserMemcg* UserMemcgsGet(int userId);

    bool HandleAccountDied(int accountId);
    bool HandleAccountPriorityChanged(int accountId, int priority);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECALIM_STRATEGY_MANAGER_H
