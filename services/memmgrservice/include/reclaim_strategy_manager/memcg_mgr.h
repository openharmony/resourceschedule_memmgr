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

#ifndef OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_MEMCG_MGR_H
#define OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_MEMCG_MGR_H

#include <map>
#include <string>

#include "single_instance.h"
#include "memcg.h"

namespace OHOS {
namespace Memory {
class MemcgMgr final {
    DECLARE_SINGLE_INSTANCE_BASE(MemcgMgr);
public:
    ~MemcgMgr();

    // root memcg operations
    Memcg* GetRootMemcg() const;
    bool SetRootMemcgPara();

    // user memcg operations
    UserMemcg* GetUserMemcg(unsigned int userId);
    UserMemcg* AddUserMemcg(unsigned int userId);
    bool RemoveUserMemcg(unsigned int userId);
    bool UpdateMemcgScoreAndReclaimRatios(unsigned int userId, int score, const ReclaimRatios& ratios);
    bool AddProcToMemcg(unsigned int pid, unsigned int userId);
    bool SwapInMemcg(unsigned int userId); // load memcg data 100% to mem
    SwapInfo* GetMemcgSwapInfo(unsigned int userId);
    MemInfo* GetMemcgMemInfo(unsigned int userId);
private:
    MemcgMgr();
    Memcg* rootMemcg_;
    std::map<int, UserMemcg*> userMemcgsMap_; // map<userId, UserMemcg*>
}; // end class MemcgMgr
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_MEMCG_MGR_H
