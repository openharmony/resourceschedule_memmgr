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
}

IMPLEMENT_SINGLE_INSTANCE(LowMemoryKiller);

#define LOW_MEM_KILL_LEVELS 5
#define MAX_KILL_CNT_PER_EVENT 3
#define MAX_TARGET_BUFFER (500 * 1024) // the same with upper bound of minPrioTable

typedef enum {
    MIN_BUFFER = 0,
    MIN_PRIO,
    MIN_PRIO_FIELD_COUNT,
} MinPrioField;

static int g_minPrioTable[LOW_MEM_KILL_LEVELS][MIN_PRIO_FIELD_COUNT] = {
    {100 * 1024, 0},   // 100M buffer, 0 priority
    {200 * 1024, 100}, // 200M buffer, 100 priority
    {300 * 1024, 200}, // 300M buffer, 200 priority
    {400 * 1024, 300}, // 400M buffer, 300 priority
    {500 * 1024, 400}  // 500M buffer, 400 priority
};

int LowMemoryKiller::KillOneBundleByPrio(int minPrio)
{
    int prio;
    int freedBuf = 0;
    ReclaimPriorityManager::BundlePrioSet bundles = ReclaimPriorityManager::GetInstance().GetBundlePrioSet();

    auto itrBundle = bundles.rbegin();
    prio = (*itrBundle)->priority_;
    while (prio >= minPrio && itrBundle != bundles.rend()) {
        BundlePriorityInfo *bundle = *itrBundle;
        if (ReclaimPriorityManager::GetInstance().GetBundleState(bundle) == BundleState::STATE_WAITING_FOR_KILL) {
            continue;
        }

        for (auto itrProcess = bundle->procs_.begin(); itrProcess != bundle->procs_.end(); itrProcess++) {
            freedBuf += KernelInterface::GetInstance().KillOneProcessByPid(itrProcess->first);
        }

        ReclaimPriorityManager::GetInstance().SetBundleState(bundle, BundleState::STATE_WAITING_FOR_KILL);
        if (freedBuf) {
            break;
        }

        itrBundle++;
        prio = (*itrBundle)->priority_;
    }

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
    if (triBuf == 0) { // max
        return;
    }

    for (int i = 0; i < LOW_MEM_KILL_LEVELS; i++) {
        thBuf = g_minPrioTable[i][MIN_BUFFER];
        if (triBuf < thBuf) {
            minPrio = g_minPrioTable[i][MIN_PRIO];
            break;
        }
    }
    HILOGD("minPrio = %{public}d", minPrio);

    if (minPrio == RECLAIM_PRIORITY_UNKNOWN + 1) {
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

        availBuf = KernelInterface::GetInstance().GetCurrentBuffer();
        if (availBuf == 0) { // max
            goto out;
        }
    } while (availBuf < MAX_TARGET_BUFFER && killCnt < MAX_KILL_CNT_PER_EVENT);

out:
    // resume zswapd
    if (totalBuf) {
        HILOGI("Reclaimed %dkB when current buffer %dkB below %dkB",
                totalBuf, triBuf, thBuf);
    }
}
} // namespace Memory
} // namespace OHOS
