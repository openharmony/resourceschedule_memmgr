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
#include "low_memory_killer.h"
#include "memmgr_log.h"
#include "kernel_interface.h"
#include "reclaim_priority_manager.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "LowMemoryKiller";
    const int LOW_MEM_KILL_LEVELS = 5;
    const int MAX_KILL_CNT_PER_EVENT = 3;
}

IMPLEMENT_SINGLE_INSTANCE(LowMemoryKiller);

enum class MinPrioField {
    MIN_BUFFER = 0,
    MIN_PRIO,
    MIN_PRIO_FIELD_COUNT,
};

static int g_minPrioTable[LOW_MEM_KILL_LEVELS][static_cast<int32_t>(MinPrioField::MIN_PRIO_FIELD_COUNT)] = {
    {100 * 1024, 0},   // 100M buffer, 0 priority
    {200 * 1024, 100}, // 200M buffer, 100 priority
    {300 * 1024, 200}, // 300M buffer, 200 priority
    {400 * 1024, 300}, // 400M buffer, 300 priority
    {500 * 1024, 400}  // 500M buffer, 400 priority
};

int LowMemoryKiller::KillOneBundleByPrio(int minPrio)
{
    HILOGD("called");
    int freedBuf = 0;
    std::set<BundlePriorityInfo> bundles;

    ReclaimPriorityManager::GetInstance().GetBundlePrioSet(bundles);
    HILOGD("BundlePrioSet size=%{public}d", bundles.size());

    HILOGD("iter bundles begin");
    int count = 0;
    for (auto bundle : bundles) {
        HILOGD("bundle %{public}d/%{public}d begin", count, bundles.size());
        if (bundle.priority_ < minPrio) {
            HILOGD("finish to handle all bundles with priority bigger than %{public}d, break!", minPrio);
            break;
        }
        if (ReclaimPriorityManager::GetInstance().GetBundleState(&bundle) ==
            BundleState::STATE_WAITING_FOR_KILL) {
            HILOGD("bundle <%{publics}s> is waiting to kill, skiped.", bundle.name_.c_str());
            continue;
        }

        HILOGD("iter processes of <%{publics}s> begin", bundle.name_.c_str());
        for (auto itrProcess = bundle.procs_.begin(); itrProcess != bundle.procs_.end(); itrProcess++) {
            freedBuf += KernelInterface::GetInstance().KillOneProcessByPid(itrProcess->first);
        }
        HILOGD("iter processes of <%{publics}s> end", bundle.name_.c_str());

        ReclaimPriorityManager::GetInstance().SetBundleState(bundle.accountId_, bundle.uid_,
                                                             BundleState::STATE_WAITING_FOR_KILL);
        if (freedBuf) {
            HILOGD("freedBuf = %{public}d, return", freedBuf);
            break;
        }
        HILOGD("%{public}d/%{public}d end", count, bundles.size());
        count++;
    }
    HILOGD("iter bundles end");
    return freedBuf;
}

/* Low memory killer core function */
void LowMemoryKiller::PsiHandler()
{
    HILOGD("called");
    int triBuf, availBuf, thBuf, freedBuf;
    int totalBuf = 0;
    int minPrio = RECLAIM_PRIORITY_UNKNOWN + 1;
    int killCnt = 0;

    triBuf = KernelInterface::GetInstance().GetCurrentBuffer();
    HILOGD("current buffer = %{public}d", triBuf);
    if (triBuf == MAX_BUFFER_KB) {
        HILOGE("get buffer failed, skiped!");
        return;
    }

    for (int i = 0; i < LOW_MEM_KILL_LEVELS; i++) {
        thBuf = g_minPrioTable[i][static_cast<int32_t>(MinPrioField::MIN_BUFFER)];
        if (triBuf < thBuf) {
            minPrio = g_minPrioTable[i][static_cast<int32_t>(MinPrioField::MIN_PRIO)];
            break;
        }
    }
    HILOGD("minPrio = %{public}d", minPrio);

    if (minPrio == RECLAIM_PRIORITY_UNKNOWN + 1) {
        HILOGE("no minPrio, skiped!");
        return;
    }

    // stop zswapd
    do {
        if ((freedBuf = KillOneBundleByPrio(minPrio)) == 0) {
            HILOGE("Noting to kill above score %{public}d!", minPrio);
            goto out;
        }
        totalBuf += freedBuf;
        killCnt++;
        HILOGD("killCnt = %{public}d", killCnt);

        availBuf = KernelInterface::GetInstance().GetCurrentBuffer();
        if (availBuf == MAX_BUFFER_KB) {
            HILOGE("get buffer failed, go out!");
            goto out;
        }
    } while (availBuf < MAX_BUFFER_KB && killCnt < MAX_KILL_CNT_PER_EVENT);

out:
    // resume zswapd
    if (totalBuf) {
        HILOGI("Reclaimed %dkB when current buffer %dkB below %dkB",
                totalBuf, triBuf, thBuf);
    }
}
} // namespace Memory
} // namespace OHOS
