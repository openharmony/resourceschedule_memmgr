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
#include "memmgr_config_manager.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "kernel_interface.h"
#include "reclaim_priority_manager.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "LowMemoryKiller";
    const int LOW_MEM_KILL_LEVELS = 5;
    const int MAX_KILL_CNT_PER_EVENT = 3;
    const int NOT_TO_KILL_DURING = 3;
    /*
     * LMKD_DBG_TRIGGER_FILE_PATH:
     * print process meminfo when write 0/1 to the file,
     * 0: print all info anyway. 1: print limited by interval.
     * It is used before killing one bundle.
     */
    const std::string LMKD_DBG_TRIGGER_FILE_PATH = "/proc/lmkd_dbg_trigger";
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

LowMemoryKiller::LowMemoryKiller()
{
    initialized_ = GetEventHandler();
    if (initialized_) {
        HILOGI("init successed");
    } else {
        HILOGE("init failed");
    }
}

bool LowMemoryKiller::GetEventHandler()
{
    if (!handler_) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

int32_t LowMemoryKiller::GetKillLevel()
{
    return killLevel_;
}

int LowMemoryKiller::KillOneBundleByPrio(int minPrio)
{
    HILOGE("called. minPrio=%{public}d", minPrio);
    int freedBuf = 0;
    ReclaimPriorityManager::BunldeCopySet bundles;

    ReclaimPriorityManager::GetInstance().GetOneKillableBundle(minPrio, bundles);
    HILOGD("get BundlePrioSet size=%{public}zu", bundles.size());

    int count = 0;
    for (auto bundle : bundles) {
        HILOGI("iter bundle %{public}d/%{public}zu, uid=%{public}d, name=%{public}s, priority=%{public}d",
               count, bundles.size(), bundle.uid_, bundle.name_.c_str(), bundle.priority_);
        if (bundle.priority_ < minPrio) {
            HILOGD("finish to handle all bundles with priority bigger than %{public}d, break!", minPrio);
            break;
        }
        if (KernelInterface::GetInstance().GetSystemCurTime() - bundle.GetCreateTime() < NOT_TO_KILL_DURING) {
            HILOGD("bundle uid<%{public}d> <%{public}s> is protected, skiped.",
                bundle.uid_, bundle.name_.c_str());
            count++;
            continue;
        }
        if (bundle.GetState() == BundleState::STATE_WAITING_FOR_KILL) {
            HILOGD("bundle uid<%{public}d> <%{public}s> is waiting to kill, skiped.",
                bundle.uid_, bundle.name_.c_str());
            count++;
            continue;
        }

        for (auto itrProcess = bundle.procs_.begin(); itrProcess != bundle.procs_.end(); itrProcess++) {
            HILOGI("killing pid<%{public}d> with uid<%{public}d> of bundle<%{public}s>",
                itrProcess->first, bundle.uid_, bundle.name_.c_str());
            freedBuf += KernelInterface::GetInstance().KillOneProcessByPid(itrProcess->first);
        }

        ReclaimPriorityManager::GetInstance().SetBundleState(bundle.accountId_, bundle.uid_,
                                                             BundleState::STATE_WAITING_FOR_KILL);
        if (freedBuf) {
            HILOGD("freedBuf = %{public}d, break iter", freedBuf);
            break;
        }
        count++;
    }
    HILOGD("iter bundles end");
    return freedBuf;
}

std::pair<unsigned int, int> LowMemoryKiller::QueryKillMemoryPriorityPair(unsigned int currBufferKB,
    unsigned int &targetBufKB, int &killLevel)
{
    unsigned int minBufKB = 0;
    int minPrio = RECLAIM_PRIORITY_UNKNOWN + 1;
    int tempKillLevel = 0;

    targetBufKB = 0; /* default val */
    static const KillConfig::KillLevelsMap levelMap = MemmgrConfigManager::GetInstance().GetKillLevelsMap();
    if (levelMap.empty()) { /* xml not config, using default table */
        for (int i = 0; i < LOW_MEM_KILL_LEVELS; i++) {
            int minBufInTable = g_minPrioTable[i][static_cast<int32_t>(MinPrioField::MIN_BUFFER)];
            if (minBufInTable < 0) {
                HILOGE("error: negative value(%{public}d) of mem in g_minPrioTable", minBufInTable);
                continue;
            }
            tempKillLevel++;
            if (currBufferKB < (unsigned int)minBufInTable) {
                minBufKB = (unsigned int)minBufInTable;
                minPrio = g_minPrioTable[i][static_cast<int32_t>(MinPrioField::MIN_PRIO)];
                break;
            }
        }
        /* set targetBufKB = max mem val in g_minPrioTable */
        int maxMemInTable = g_minPrioTable[LOW_MEM_KILL_LEVELS - 1][static_cast<int32_t>(MinPrioField::MIN_BUFFER)];
        targetBufKB = (maxMemInTable > 0 ? (unsigned int)maxMemInTable : 0);
        killLevel = tempKillLevel;
        return std::make_pair(minBufKB, minPrio);
    }
    /* query from xml */
    for (auto it = levelMap.begin(); it != levelMap.end(); it++) {
        tempKillLevel++;
        if (currBufferKB < it->first) {
            minBufKB = it->first;
            minPrio = it->second;
            break;
        }
    }
    /* set targetBufKB = max mem val in levelMap */
    targetBufKB = levelMap.rbegin()->first;
    killLevel = tempKillLevel;
    HILOGD("(%{public}u) return from xml mem:%{public}u prio:%{public}d target:%{public}u",
        currBufferKB, minBufKB, minPrio, targetBufKB);
    return std::make_pair(minBufKB, minPrio);
}

/* Low memory killer core function */
void LowMemoryKiller::PsiHandlerInner()
{
    HILOGD("[%{public}ld] called", ++calledCount);
    int triBuf, availBuf, freedBuf;
    unsigned int minBuf = 0, targetBuf = 0, targetKillKb = 0, currKillKb = 0;
    int minPrio;
    int killCnt = 0;

    triBuf = KernelInterface::GetInstance().GetCurrentBuffer();
    HILOGE("[%{public}ld] current buffer = %{public}d KB", calledCount, triBuf);
    if (triBuf == MAX_BUFFER_KB) {
        HILOGD("[%{public}ld] get buffer failed, skiped!", calledCount);
        return;
    }

    std::pair<unsigned int, int> memPrioPair = QueryKillMemoryPriorityPair(triBuf, targetBuf, killLevel_);
    minBuf = memPrioPair.first;
    minPrio = memPrioPair.second;
    if (triBuf > 0 && targetBuf > (unsigned int)triBuf) {
        targetKillKb = (unsigned int)(targetBuf - triBuf);
    }

    HILOGE("[%{public}ld] minPrio = %{public}d", calledCount, minPrio);

    if (minPrio == RECLAIM_PRIORITY_UNKNOWN + 1) {
        HILOGD("[%{public}ld] no minPrio, skiped!", calledCount);
        return;
    }

    do {
        /* print process mem info in dmesg, 1 means it is limited by print interval. Ignore return val   */
        KernelInterface::GetInstance().EchoToPath(LMKD_DBG_TRIGGER_FILE_PATH.c_str(), "1");
        if ((freedBuf = KillOneBundleByPrio(minPrio)) <= 0) {
            HILOGD("[%{public}ld] Noting to kill above score %{public}d!", calledCount, minPrio);
            goto out;
        }
        currKillKb += (unsigned int)freedBuf;
        killCnt++;
        HILOGD("[%{public}ld] killCnt = %{public}d", calledCount, killCnt);

        availBuf = KernelInterface::GetInstance().GetCurrentBuffer();
        if (availBuf < 0 || availBuf >= MAX_BUFFER_KB) {
            HILOGE("[%{public}ld] get buffer failed, go out!", calledCount);
            goto out;
        }
        if ((unsigned int)availBuf >= targetBuf) {
            killLevel_ = 0;
            goto out;
        }
    } while (currKillKb < targetKillKb && killCnt < MAX_KILL_CNT_PER_EVENT);

out:
    if (currKillKb > 0) {
        HILOGI("[%{public}ld] Reclaimed %{public}uK when currBuff %{public}dK below %{public}uK target %{public}uK",
            calledCount, currKillKb, triBuf, minBuf, targetBuf);
    }
}

void LowMemoryKiller::PsiHandler()
{
    if (!initialized_) {
        HILOGE("is not initialized, return!");
        return;
    }
    std::function<void()> func = std::bind(&LowMemoryKiller::PsiHandlerInner, this);
    handler_->PostImmediateTask(func);
}
} // namespace Memory
} // namespace OHOS
