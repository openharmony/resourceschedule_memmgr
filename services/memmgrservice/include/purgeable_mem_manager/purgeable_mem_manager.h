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

#include "event_handler.h"
#include "single_instance.h"
#include "app_state_subscriber.h"
#include "remote_death_recipient.h"
#include "memory_level_constants.h"
#include "purgeable_mem_constants.h"

namespace OHOS {
namespace Memory {
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
    void Test(int fd, std::vector<std::string> &params);

private:
    PurgeableMemManager();
    ~PurgeableMemManager() = default;
    void NotifyMemoryLevelInner(const SystemMemoryInfo &info);
    void TriggerByPsi(const SystemMemoryInfo &info);
    void RegisterActiveAppsInner(int32_t pid, int32_t uid);
    void DeregisterActiveAppsInner(int32_t pid, int32_t uid);
    void AddSubscriberInner(const sptr<IAppStateSubscriber> &subscriber);
    void RemoveSubscriberInner(const sptr<IAppStateSubscriber> &subscriber);
    void OnRemoteSubscriberDiedInner(const wptr<IRemoteObject> &object);
    void ChangeAppStateInner(int32_t pid, int32_t uid, int32_t state);
    void TrimAllSubscribers(const SystemMemoryLevel &level);
    void ReclaimInner(int32_t pid);
    void ReclaimAllInner();
    void Reclaim(int32_t pid);
    void ReclaimAll();
    void ShowRegistedApps(int fd);
    bool GetEventHandler();
    bool CheckCallingToken();
    bool isNumeric(std::string const &str);
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    bool initialized_ = false;
    std::map<int32_t, std::pair<int32_t, int32_t>> appList_;
    std::list<sptr<IAppStateSubscriber>> appStateSubscribers_ {};
    std::map<sptr<IRemoteObject>, sptr<RemoteDeathRecipient>> subscriberRecipients_ {};
    std::mutex mutexAppList;
    std::mutex mutexSubscribers;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_PURGEABLE_MEM_MANAGER_H
