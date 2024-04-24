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
#ifndef OHOS_MEMORY_DUMP_COMMAND_DISPATCHER_H
#define OHOS_MEMORY_DUMP_COMMAND_DISPATCHER_H

#include "memory_level_constants.h"
#include "memory_level_manager.h"
#ifdef USE_PURGEABLE_MEMORY
#include "purgeable_mem_manager.h"
#endif

namespace OHOS {
namespace Memory {
constexpr unsigned int ONTRIM_LEVEL_PARAM_SIZE = 1;
constexpr unsigned int RECLAIM_TYPE_PARAM_SIZE = 1;
constexpr unsigned int RECLAIM_HEAP_ID_PARAM_SIZE = 2;
constexpr unsigned int RECLAIM_ASHM_ID_PARAM_SIZE = 2;
constexpr unsigned int RECLAIM_SUBSCRIBER_ID_PARAM_SIZE = 1;
constexpr unsigned int FIRST_INDEX = 0;
constexpr unsigned int SECOND_INDEX = 1;
constexpr unsigned int THIRD_INDEX = 2;
constexpr unsigned int APP_STATE_PARAM_SIZE = 3;

#define CHECK_SIZE(container, len, fd, actionIfFailed) \
    do {                                               \
        if ((container).size() != (len)) {             \
            dprintf(fd, "size error\n");               \
            actionIfFailed;                            \
        }                                              \
    } while (0)

inline bool HasCommand(const std::map<std::string, std::vector<std::string>> &keyValuesMapping,
                       const std::string &command)
{
    return keyValuesMapping.find(command) != keyValuesMapping.end();
}

void ShowHelpInfo(const int fd)
{
    dprintf(fd, "Usage:\n");
    dprintf(fd, "-h                          |help for memmgrservice dumper\n");
    dprintf(fd, "-a                          |dump all info\n");
    dprintf(fd, "-e                          |dump event observer\n");
    dprintf(fd, "-r                          |dump reclaim info and adj\n");
    dprintf(fd, "-c                          |dump config\n");
    dprintf(fd, "-m                          |show malloc state\n");
#ifdef USE_PURGEABLE_MEMORY
    dprintf(fd, "-s                          |show subscriber all the pid which can be reclaimed\n");
    dprintf(fd, "-d {pid} {uid} {state}      |trigger appstate changed\n\n");
    dprintf(fd, "-t                          trigger memory onTrim:\n"
                "-t 1 ---------------------- level_purgeable\n"
                "-t 2 ---------------------- level_moderate\n"
                "-t 3 ---------------------- level_low\n"
                "-t 4 ---------------------- level_critical\n\n");
    dprintf(fd, "-f                          trigger purgeable memory Reclaim:\n"
                "-f 1 ---------------------- purg_heap all\n"
                "-f 2 ---------------------- purg_ashmem all\n"
                "-f 3 ---------------------- purg_subscriber all\n"
                "-f 4 ---------------------- purg all purgeable memory\n"
                "-f 1 -id {userId} {size} -- purg_heap by memCG and size(KB). if userId=0, reclaim system_lru\n"
                "-f 2 -id {ashmId} {time} -- purg_ashm by ashmId, which can get from /proc/purgeable_ashmem_trigger\n"
                "-f 3 -id {pid} ------------ purg_subscriber by pid. if pid=0, reclaim subscriber all\n\n");
#endif
}

#ifdef USE_PURGEABLE_MEMORY
void PrintOntrimError(const int fd)
{
    dprintf(fd, "\n error: unrecognized memory level, please input correct format as follows:\n"
                "-t 1 ---------------------- level_purgeable\n"
                "-t 2 ---------------------- level_moderate\n"
                "-t 3 ---------------------- level_low\n"
                "-t 4 ---------------------- level_critical\n");
}

void PrintReclaimError(const int fd)
{
    dprintf(fd, "\n error: trigger force reclaim failed, please input correct info as follows:\n"
                "-f 1 ---------------------- purg_heap all\n"
                "-f 2 ---------------------- purg_ashmem all\n"
                "-f 3 ---------------------- purg_subscriber all\n"
                "-f 4 ---------------------- purg all purgeable memory\n"
                "-f 1 -id {userId} {size} -- purg_heap by memCG and size(KB). if userId=0, reclaim system_lru\n"
                "-f 2 -id {ashmId} {time} -- purg_ashm by ashmId, which can get from /proc/purgeable_ashmem_trigger\n"
                "-f 3 -id {pid} ------------ purg_subscriber by pid. if pid=0, reclaim subscriber all\n");
}

void DispatchTriggerMemLevel(const int fd, std::map<std::string, std::vector<std::string>> &keyValuesMapping)
{
    std::vector<std::string> values = keyValuesMapping["-t"];
    CHECK_SIZE(values, ONTRIM_LEVEL_PARAM_SIZE, fd, return);

    int level;
    try {
        level = std::stoi(values[FIRST_INDEX]);
    } catch (...) {
        PrintOntrimError(fd);
        return;
    }

    SystemMemoryInfo info = {MemorySource::MANUAL_DUMP, SystemMemoryLevel::UNKNOWN};
    switch (level) {
        case MEMORY_LEVEL_PURGEABLE:
            info.level = SystemMemoryLevel::MEMORY_LEVEL_PURGEABLE;
            break;
        case MEMORY_LEVEL_MODERATE:
            info.level = SystemMemoryLevel::MEMORY_LEVEL_MODERATE;
            break;
        case MEMORY_LEVEL_LOW:
            info.level = SystemMemoryLevel::MEMORY_LEVEL_LOW;
            break;
        case MEMORY_LEVEL_CRITICAL:
            info.level = SystemMemoryLevel::MEMORY_LEVEL_CRITICAL;
            break;
        default:
            PrintOntrimError(fd);
            return;
    }
    MemoryLevelManager::GetInstance().TriggerMemoryLevelByDump(info);
}

void ParseForceReclaimType(const int fd, std::map<std::string, std::vector<std::string>> &keyValuesMapping,
                           DumpReclaimInfo &dumpInfo)
{
    dumpInfo.reclaimType = PurgeableMemoryType::UNKNOWN;
    dumpInfo.ifReclaimTypeAll = true;

    std::vector<std::string> values = keyValuesMapping["-f"];
    CHECK_SIZE(values, RECLAIM_TYPE_PARAM_SIZE, fd, return);

    int type;
    try {
        type = std::stoi(values[FIRST_INDEX]);
    } catch (...) {
        return;
    }
    switch (type) {
        case PURGEABLE_TYPE_HEAP:
            dumpInfo.reclaimType = PurgeableMemoryType::PURGEABLE_HEAP;
            break;
        case PURGEABLE_TYPE_ASHMEM:
            dumpInfo.reclaimType = PurgeableMemoryType::PURGEABLE_ASHMEM;
            break;
        case PURGEABLE_TYPE_SUBSCRIBER:
            dumpInfo.reclaimType = PurgeableMemoryType::PURGEABLE_SUBSCRIBER;
            break;
        case PURGEABLE_TYPE_ALL:
            dumpInfo.reclaimType = PurgeableMemoryType::PURGEABLE_ALL;
            break;
        default:
            return;
    }
}

bool ParseForceReclaimId(const int fd, std::map<std::string, std::vector<std::string>> &keyValuesMapping,
                         DumpReclaimInfo &dumpInfo)
{
    dumpInfo.ifReclaimTypeAll = false;
    std::vector<std::string> values = keyValuesMapping["-id"];
    switch (dumpInfo.reclaimType) {
        case PurgeableMemoryType::PURGEABLE_HEAP:
            CHECK_SIZE(values, RECLAIM_HEAP_ID_PARAM_SIZE, fd, return false); // {userId} {size}
            try {
                dumpInfo.memcgUserId = std::stoi(values[FIRST_INDEX]);
                dumpInfo.reclaimHeapSizeKB = std::stoi(values[SECOND_INDEX]);
            } catch (...) {
                return false;
            }
            break;
        case PurgeableMemoryType::PURGEABLE_ASHMEM:
            CHECK_SIZE(values, RECLAIM_ASHM_ID_PARAM_SIZE, fd, return false); // {ashmId} {time}
            try {
                dumpInfo.ashmId = std::stoul(values[FIRST_INDEX]);
                dumpInfo.ashmTime = std::stoul(values[SECOND_INDEX]);
            } catch (...) {
                return false;
            }
            break;
        case PurgeableMemoryType::PURGEABLE_SUBSCRIBER:
            CHECK_SIZE(values, RECLAIM_SUBSCRIBER_ID_PARAM_SIZE, fd, return false); // {pid}
            if (values[0] == std::string("0")) { // reclaim subscriber all when pid = 0
                dumpInfo.ifReclaimTypeAll = true;
                return true;
            }
            try {
                dumpInfo.subscriberPid = std::stoi(values[FIRST_INDEX]);
            } catch (...) {
                return false;
            }
            break;
        default:
            return false;
    }
    return true;
}

bool PurgeableMemoryDump(int fd, std::map<std::string, std::vector<std::string>> &keyValuesMapping)
{
    if (HasCommand(keyValuesMapping, "-s")) {
        PurgeableMemManager::GetInstance().DumpSubscribers(fd);
        return true;
    }
    if (HasCommand(keyValuesMapping, "-t")) {
        DispatchTriggerMemLevel(fd, keyValuesMapping);
        return true;
    }
    if (HasCommand(keyValuesMapping, "-f")) {
        DumpReclaimInfo dumpInfo;
        ParseForceReclaimType(fd, keyValuesMapping, dumpInfo);
        if (HasCommand(keyValuesMapping, "-id") && !ParseForceReclaimId(fd, keyValuesMapping, dumpInfo)) {
            dumpInfo.reclaimType = PurgeableMemoryType::UNKNOWN;
        }
        if (PurgeableMemManager::GetInstance().ForceReclaimByDump(dumpInfo)) {
            dprintf(fd, "trigger force reclaim success!\n");
        } else {
            PrintReclaimError(fd);
        }
        return true;
    }
    if (HasCommand(keyValuesMapping, "-d")) {
        std::vector<std::string> appState = keyValuesMapping["-d"];
        if (appState.size() < APP_STATE_PARAM_SIZE) {
            dprintf(fd, "params number is less than %{publid}d!\n", APP_STATE_PARAM_SIZE);
            return true;
        }
        int32_t pid = std::stoi(appState[FIRST_INDEX]);
        int32_t uid = std::stoi(appState[SECOND_INDEX]);
        int32_t state = std::stoi(appState[THIRD_INDEX]);
        PurgeableMemManager::GetInstance().ChangeAppState(pid, uid, state);
        return true;
    }
    return false;
}
#endif // USE_PURGEABLE_MEMORY

void DispatchDumpCommand(const int fd, std::map<std::string, std::vector<std::string>> &keyValuesMapping)
{
    if (keyValuesMapping.empty() || HasCommand(keyValuesMapping, "-h")) {
        ShowHelpInfo(fd);
        return;
    }
    if (HasCommand(keyValuesMapping, "-a")) {
        MemMgrEventCenter::GetInstance().Dump(fd);
        ReclaimPriorityManager::GetInstance().Dump(fd);
        return;
    }
    if (HasCommand(keyValuesMapping, "-e")) {
        MemMgrEventCenter::GetInstance().Dump(fd);
        return;
    }
    if (HasCommand(keyValuesMapping, "-r")) {
        ReclaimPriorityManager::GetInstance().Dump(fd);
        return;
    }
    if (HasCommand(keyValuesMapping, "-c")) {
        MemmgrConfigManager::GetInstance().Dump(fd);
        return;
    }
#ifdef USE_PURGEABLE_MEMORY
    if (PurgeableMemoryDump(fd, keyValuesMapping)) {
        return;
    }
#endif
}

} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_DUMP_COMMAND_DISPATCHER_H