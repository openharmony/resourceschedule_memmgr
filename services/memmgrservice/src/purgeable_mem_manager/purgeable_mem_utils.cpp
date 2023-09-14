/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "purgeable_mem_utils.h"

#include <climits>
#include <unordered_map>
#include <string>

#include "kernel_interface.h"
#include "memmgr_log.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "PurgeableMemUtils";
}

IMPLEMENT_SINGLE_INSTANCE(PurgeableMemUtils);

const std::string PurgeableMemUtils::PATH_PURGE_HEAP = "/proc/sys/kernel/purgeable";
const std::string PurgeableMemUtils::PATH_PURGEABLE_ASHMEM = "/proc/purgeable_ashmem_trigger";
const std::string PurgeableMemUtils::FILE_PURGE_MEMCG_HEAP = "memory.force_shrink_purgeable_bysize";
const std::string PurgeableMemUtils::ACTIVE_PURGEABLE_HEAP = "Active(purg):";
const std::string PurgeableMemUtils::INACTIVE_PURGEABLE_HEAP = "Inactive(purg):";
const std::string PurgeableMemUtils::PINED_PURGEABLE_HEAP = "Pined(purg):";
const std::string PurgeableMemUtils::PROC_PURGEABLE_HEAP = "PurgSum:";
const std::string PurgeableMemUtils::PROC_PINED_PURGEABLE_HEAP = "PurgPin:";
const unsigned int PurgeableMemUtils::ASHM_PARAM_SIZE_ONE_LINE = 10;
const unsigned int PurgeableMemUtils::ASHM_ID_INDEX = 6;
const unsigned int PurgeableMemUtils::ASHM_ADJ_INDEX = 2;
const unsigned int PurgeableMemUtils::ASHM_REF_COUNT_INDEX = 8;
const unsigned int PurgeableMemUtils::ASHM_PURGED_INDEX = 9;
const unsigned int PurgeableMemUtils::ASHM_SIZE_INDEX = 5;
const unsigned int PurgeableMemUtils::ASHM_TIME_INDEX = 7;
const unsigned int PurgeableMemUtils::HEAPINFO_SIZE_ONE_LINE = 3;
const unsigned int PurgeableMemUtils::ASHM_PROCESS_NAME_INDEX = 0;

bool PurgeableMemUtils::GetPurgeableHeapInfo(int &reclaimableKB)
{
    std::vector<std::string> strLines;
    if (!KernelInterface::GetInstance().ReadLinesFromFile(KernelInterface::MEMINFO_PATH, strLines)) {
        HILOGE("read file and split to lines failed : %{public}s", KernelInterface::MEMINFO_PATH.c_str());
        return false;
    }

    int activeKB = -1;
    int inactiveKB = -1;
    int pinedKB = -1;
    for (auto &it : strLines) {
        std::vector<std::string> words;
        KernelInterface::GetInstance().SplitOneLineByBlank(it, words);
        if (words.size() != HEAPINFO_SIZE_ONE_LINE) {
            continue;
        }
        try {
            if (words[0] == ACTIVE_PURGEABLE_HEAP) {
                activeKB = stoi(words[1]);
            } else if (words[0] == INACTIVE_PURGEABLE_HEAP) {
                inactiveKB = stoi(words[1]);
            } else if (words[0] == PINED_PURGEABLE_HEAP) {
                pinedKB = stoi(words[1]);
            }
        } catch (...) {
            HILOGE("stoi(%{public}s) failed", words[1].c_str());
            return false;
        }
    }

    if (activeKB < 0 || inactiveKB < 0 || pinedKB < 0) {
        return false;
    }
    if (activeKB > (INT_MAX - inactiveKB) || (activeKB + inactiveKB) < (INT_MIN + pinedKB)) {
        return false;
    }
    reclaimableKB = activeKB + inactiveKB - pinedKB;
    if (reclaimableKB >= 0) {
        return true;
    }
    return false;
}

bool PurgeableMemUtils::GetProcPurgeableHeapInfo(const int pid, int &reclaimableKB)
{
    std::string path = KernelInterface::GetInstance().JoinPath(KernelInterface::ROOT_PROC_PATH, std::to_string(pid),
                                                               KernelInterface::FILE_PROC_STATUS);
    std::vector<std::string> strLines;
    if (!KernelInterface::GetInstance().ReadLinesFromFile(path, strLines)) {
        HILOGE("read file and split to lines failed : %{public}s", path.c_str());
        return false;
    }

    int purgSumKB = -1;
    int purgPinKB = -1;
    for (auto &it : strLines) {
        std::vector<std::string> words;
        KernelInterface::GetInstance().SplitOneLineByBlank(it, words);
        if (words.size() != HEAPINFO_SIZE_ONE_LINE) {
            continue;
        }
        try {
            if (words[0] == PROC_PURGEABLE_HEAP) {
                purgSumKB = stoi(words[1]);
            } else if (words[0] == PROC_PINED_PURGEABLE_HEAP) {
                purgPinKB = stoi(words[1]);
            }
        } catch (...) {
            HILOGE("stoi(%{public}s) failed", words[1].c_str());
            return false;
        }
    }

    if (purgSumKB < 0 || purgPinKB < 0) {
        return false;
    }
    reclaimableKB = purgSumKB - purgPinKB;
    if (reclaimableKB >= 0) {
        return true;
    }
    return false;
}

bool PurgeableMemUtils::PurgeHeapAll()
{
    HILOGD("enter! Purg heap memory all");
    return KernelInterface::GetInstance().EchoToPath(PATH_PURGE_HEAP.c_str(), "1");
}

bool PurgeableMemUtils::PurgeHeapMemcg(const std::string &memcgPath, const int sizeKB)
{
    std::string path = KernelInterface::GetInstance().JoinPath(memcgPath, FILE_PURGE_MEMCG_HEAP);
    HILOGD("enter! Purg heap memory by memcg: size=%{public}d, path=%{public}s\n", sizeKB, path.c_str());
    return KernelInterface::GetInstance().EchoToPath(path.c_str(), std::to_string(sizeKB).c_str());
}

bool PurgeableMemUtils::GetPurgeableAshmInfo(int &reclaimableKB, std::vector<PurgeableAshmInfo> &ashmInfoToReclaim)
{
    std::vector<std::string> strLines;
    if (!KernelInterface::GetInstance().ReadLinesFromFile(PATH_PURGEABLE_ASHMEM, strLines)) {
        HILOGE("read file and split to lines failed : %{public}s", PATH_PURGEABLE_ASHMEM.c_str());
        return false;
    }

    std::unordered_map<std::string, PurgeableAshmInfo> ashmIdToInfoMap;
    ashmIdToInfoMap = PurgeableMemUtils::GetashmIdToInfoMap(strLines);

    reclaimableKB = 0;
    ashmInfoToReclaim.clear();
    for (const auto &[_1, value] : ashmIdToInfoMap) {
        if (value.sizeKB > 0) {
            ashmInfoToReclaim.emplace_back(value);
            reclaimableKB += value.sizeKB;
        }
    }
    HILOGD("there are %{public}dKB reclaimable purgeable [ASHM], ashmInfoVector.size()=%{public}zu", reclaimableKB,
           ashmInfoToReclaim.size());
    return true;
}

bool PurgeableMemUtils::PurgeAshmAll()
{
    HILOGD("enter! Purg ashmem memory all");
    return KernelInterface::GetInstance().EchoToPath(PATH_PURGEABLE_ASHMEM.c_str(), "0 0");
}

bool PurgeableMemUtils::PurgeAshmByIdWithTime(const std::string &idWithTime)
{
    HILOGD("enter! Purg ashmem memory: IdWithTime=%{public}s", idWithTime.c_str());
    return KernelInterface::GetInstance().EchoToPath(PATH_PURGEABLE_ASHMEM.c_str(), idWithTime.c_str());
}

PurgeableAshmInfoMap PurgeableMemUtils::GetashmIdToInfoMap(const std::vector<std::string> &strLines) const
{
    std::unordered_map<std::string, PurgeableAshmInfo> ashmIdToInfoMap;
    for (auto &it : strLines) {
        HILOGD("[ASHM]: %{public}s", it.c_str());
        std::vector<std::string> words;
        KernelInterface::GetInstance().SplitOneLineByDelim(it, ',', words);
        if (words.size() != ASHM_PARAM_SIZE_ONE_LINE || words[ASHM_REF_COUNT_INDEX] != "0" ||
            words[ASHM_PURGED_INDEX] != "0") {
            continue;
        }
        std::string curAppName;
        int minPriority;
        int sizeKB;
        try {
            curAppName = words[ASHM_PROCESS_NAME_INDEX];
            minPriority = stoi(words[ASHM_ADJ_INDEX]);
            sizeKB = stoi(words[ASHM_SIZE_INDEX]);
        } catch (...) {
            HILOGE("stoi(%{public}s) or stoi(%{public}s) or stoi(%{public}s) failed",
                words[ASHM_PROCESS_NAME_INDEX].c_str(), words[ASHM_ADJ_INDEX].c_str(), words[ASHM_SIZE_INDEX].c_str());
            continue;
        }
        std::string key = words[ASHM_ID_INDEX] + std::string(" ") + words[ASHM_TIME_INDEX];
        auto iter = ashmIdToInfoMap.find(key);
        if (iter == ashmIdToInfoMap.end()) {
            PurgeableAshmInfo info;
            info.curAppName = curAppName;
            info.minPriority = minPriority;
            info.sizeKB = sizeKB;
            info.idWithTime = key;
            ashmIdToInfoMap[key] = info;
        } else if (iter->second.minPriority > minPriority) {
            iter->second.minPriority = minPriority;
        }
    }
    return ashmIdToInfoMap;
}
} // namespace Memory
} // namespace OHOS
