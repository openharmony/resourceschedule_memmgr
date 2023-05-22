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

#ifndef OHOS_MEMORY_MEMMGR_PURGEABLE_MEM_MANAGER_H
#define OHOS_MEMORY_MEMMGR_PURGEABLE_MEM_MANAGER_H

#include <map>
#include <unordered_map>

#include "app_state_subscriber.h"
#include "event_handler.h"
#include "kernel_interface.h"
#include "memory_level_constants.h"
#include "purgeable_mem_constants.h"
#include "purgeable_mem_utils.h"
#include "remote_death_recipient.h"
#include "single_instance.h"

namespace OHOS {
namespace Memory {
struct PurgeableMemoryInfo {
    PurgeableMemoryType type;
    int reclaimableKB;
    std::vector<PurgeableAshmInfo> ashmInfoToReclaim;
};

struct DumpReclaimInfo {
    PurgeableMemoryType reclaimType;
    bool ifReclaimTypeAll;
    int memcgUserId;
    int reclaimHeapSizeKB;
    unsigned int ashmId;
    unsigned int ashmTime;
    int subscriberPid;
};

class PurgeableMemManager {
    DECLARE_SINGLE_INSTANCE_BASE(PurgeableMemManager);

public:
    void NotifyMemoryLevel(const SystemMemoryInfo &info);
    void RegisterActiveApps(int32_t pid, int32_t uid);
    void DeregisterActiveApps(int32_t pid, int32_t uid);
    void ChangeAppState(int32_t pid, int32_t uid, int32_t state);
    void AddSubscriber(const sptr<IAppStateSubscriber> &subscriber);
    void RemoveSubscriber(const sptr<IAppStateSubscriber> &subscriber);
    void OnRemoteSubscriberDied(const wptr<IRemoteObject> &object);
    void DumpSubscribers(const int fd);
    bool ForceReclaimByDump(const DumpReclaimInfo &dumpInfo);

private:
    PurgeableMemManager();
    ~PurgeableMemManager() = default;
    void NotifyMemoryLevelInner(const SystemMemoryInfo &info);
    void TriggerByManualDump(const SystemMemoryInfo &info);
    void TriggerByPsi(const SystemMemoryInfo &info);
    int PurgeByTypeAndTarget(const PurgeableMemoryType &type, const int reclaimTargetKB);
    bool GetPurgeableInfo(PurgeableMemoryInfo &info);
    bool GetMemcgPathByUserId(const int userId, std::string &memcgPath);
    bool PurgeTypeAll(const PurgeableMemoryType &type);
    bool PurgeHeap(const int userId, const int size);
    bool PurgeAshm(const unsigned int ashmId, const unsigned int time);
    void PurgHeapMemcgOneByOne(const int reclaimTargetKB, int &reclaimResultKB);
    bool PurgHeapOneMemcg(const std::vector<int> &memcgPids, const std::string &memcgPath, const int reclaimTargetKB,
                          int &reclaimResultKB);
    void PurgAshmIdOneByOne(std::vector<PurgeableAshmInfo> &ashmInfoToReclaim, const int reclaimTargetKB,
                            int &reclaimResultKB);
    bool AshmReclaimPriorityCompare(const PurgeableAshmInfo &left, const PurgeableAshmInfo &right);
    std::string PurgMemType2String(const PurgeableMemoryType &type);
    void RegisterActiveAppsInner(int32_t pid, int32_t uid);
    void DeregisterActiveAppsInner(int32_t pid, int32_t uid);
    void AddSubscriberInner(const sptr<IAppStateSubscriber> &subscriber);
    void RemoveSubscriberInner(const sptr<IAppStateSubscriber> &subscriber);
    void OnRemoteSubscriberDiedInner(const wptr<IRemoteObject> &object);
    void ChangeAppStateInner(int32_t pid, int32_t uid, int32_t state);
    void TrimAllSubscribers(const SystemMemoryLevel &level);
    void ReclaimSubscriberProc(const int32_t pid);
    void ReclaimSubscriberAll();
    bool GetEventHandler();
    bool CheckCallingToken();
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    bool initialized_ = false;
    std::map<int32_t, std::pair<int32_t, int32_t>> appList_;
    std::list<sptr<IAppStateSubscriber>> appStateSubscribers_ {};
    std::map<sptr<IRemoteObject>, sptr<RemoteDeathRecipient>> subscriberRecipients_ {};
    std::mutex mutexAppList;
    std::mutex mutexSubscribers;
    time_t lastTriggerTime = 0; // last trigger time by psi or kswapd.
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_PURGEABLE_MEM_MANAGER_H
