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

#include "purgeable_mem_manager.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "app_mem_info.h"
#include "app_mgr_constants.h"
#include "ipc_skeleton.h"
#include "accesstoken_kit.h"
 
namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "PurgeableMemManager";
}

IMPLEMENT_SINGLE_INSTANCE(PurgeableMemManager);

PurgeableMemManager::PurgeableMemManager()
{
    initialized_ = GetEventHandler();
    if (initialized_) {
        HILOGI("init succeeded");
    } else {
        HILOGE("init failed");
    }
    appList_.clear();
}

bool PurgeableMemManager::GetEventHandler()
{
    if (!handler_) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

void PurgeableMemManager::AddSubscriberInner(const sptr<IAppStateSubscriber> &subscriber)
{
    auto remoteObj = subscriber->AsObject();
    auto findSubscriber = [&remoteObj](const auto &target) { return remoteObj == target->AsObject(); };
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers);
    auto subscriberIter = std::find_if(appStateSubscribers_.begin(), appStateSubscribers_.end(), findSubscriber);
    if (subscriberIter != appStateSubscribers_.end()) {
        HILOGE("target subscriber already exist");
        return;
    }

    if (appStateSubscribers_.size() >= PURGEABLE_SUBSCRIBER_MAX_NUM) {
        HILOGE("the number of registered subscribers has reach the upper limit");
        return;
    }

    appStateSubscribers_.emplace_back(subscriber);

    if (subscriber->AsObject() == nullptr) {
        HILOGE("subscriber is null");
        return;
    }

    if (subscriberRecipients_.find(subscriber->AsObject()) != subscriberRecipients_.end()) {
        HILOGE("subscriberRecipients_ don't find subscriber");
        return;
    }
    sptr<RemoteDeathRecipient> deathRecipient = new (std::nothrow)
        RemoteDeathRecipient([this] (const wptr<IRemoteObject> &remote) { this->OnRemoteSubscriberDied(remote); });
    if (!deathRecipient) {
        HILOGE("create death recipient failed");
        return;
    }

    subscriber->AsObject()->AddDeathRecipient(deathRecipient);
    subscriberRecipients_.emplace(subscriber->AsObject(), deathRecipient);
    subscriber->OnConnected();
    HILOGI("add app state subscriber succeed, subscriber list size is: %{public}d",
        static_cast<int>(appStateSubscribers_.size()));
}

void PurgeableMemManager::AddSubscriber(const sptr<IAppStateSubscriber> &subscriber)
{
    if (!CheckCallingToken()) {
        HILOGW("AddSubscriber not allowed");
        return;
    }
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    if (subscriber == nullptr) {
        HILOGE("subscriber is null");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::AddSubscriberInner, this, subscriber);
    handler_->PostImmediateTask(func);
}

void PurgeableMemManager::RemoveSubscriberInner(const sptr<IAppStateSubscriber> &subscriber)
{
    auto remote = subscriber->AsObject();
    if (remote == nullptr) {
        HILOGE("subscriber object is null");
        return;
    }
    auto findSubscriber = [&remote] (const auto &targetSubscriber) { return remote == targetSubscriber->AsObject(); };

    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers);
    auto subscriberIter = find_if(appStateSubscribers_.begin(), appStateSubscribers_.end(), findSubscriber);
    if (subscriberIter == appStateSubscribers_.end()) {
        HILOGE("subscriber to remove is not exists");
        return;
    }

    auto iter = subscriberRecipients_.find(remote);
    if (iter != subscriberRecipients_.end()) {
        iter->first->RemoveDeathRecipient(iter->second);
        subscriberRecipients_.erase(iter);
    }
    subscriber->OnDisconnected();
    appStateSubscribers_.erase(subscriberIter);
    HILOGI("remove subscriber succeed, subscriber list size is: %{public}d",
        static_cast<int>(appStateSubscribers_.size()));
}

void PurgeableMemManager::RemoveSubscriber(const sptr<IAppStateSubscriber> &subscriber)
{
    if (!CheckCallingToken()) {
        HILOGW("RemoveSubscriber not allowed");
        return;
    }
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    if (subscriber == nullptr) {
        HILOGE("subscriber is null");
        return;
    }
    RemoveSubscriberInner(subscriber);
}

void PurgeableMemManager::OnRemoteSubscriberDiedInner(const wptr<IRemoteObject> &object)
{
    sptr<IRemoteObject> objectProxy = object.promote();
    if (!objectProxy) {
        HILOGE("get remote object failed");
        return;
    }
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers);
    auto iter = appStateSubscribers_.begin();
    while (iter != appStateSubscribers_.end()) {
        if ((*iter)->AsObject() == objectProxy) {
            iter = appStateSubscribers_.erase(iter);
            HILOGI("remove the subscriber");
        } else {
            iter++;
        }
    }
    HILOGI("recipients remove the subscriber, subscriber list size is : %{public}d",
        static_cast<int>(appStateSubscribers_.size()));
    subscriberRecipients_.erase(objectProxy);
}

void PurgeableMemManager::OnRemoteSubscriberDied(const wptr<IRemoteObject> &object)
{
    if (object == nullptr) {
        HILOGE("remote object is null");
        return;
    }

    handler_->PostSyncTask([this, &object]() { this->OnRemoteSubscriberDiedInner(object); });
}

void PurgeableMemManager::RegisterActiveAppsInner(int32_t pid, int32_t uid)
{
    std::lock_guard<std::mutex> lockAppList(mutexAppList);
    if (appList_.size() >= PURGEABLE_APPSTATE_MAX_NUM) {
        HILOGE("the number of registered apps has reached the upper limit");
        return;
    }

    if (appList_.find(pid) != appList_.end()) {
        HILOGE("the app has already registered");
        return;
    }
    int32_t state = static_cast<int32_t>(AppExecFwk::ApplicationState::APP_STATE_FOREGROUND);
    std::pair<int32_t, int32_t> appinfo = std::make_pair(uid, state);
    appList_[pid] = appinfo;
    HILOGI("the app is registered, pid is: %{public}d, uid is %{public}d", pid, uid);
}

void PurgeableMemManager::RegisterActiveApps(int32_t pid, int32_t uid)
{
    if (!CheckCallingToken()) {
        HILOGW("AddSubscriber not allowed");
        return;
    }
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::RegisterActiveAppsInner, this, pid, uid);
    handler_->PostImmediateTask(func);
}

void PurgeableMemManager::DeregisterActiveAppsInner(int32_t pid, int32_t uid)
{
    std::lock_guard<std::mutex> lockAppList(mutexAppList);
    if (appList_.find(pid) == appList_.end()) {
        HILOGE("the app is not registered");
        return;
    }
    std::pair<int32_t, int32_t> appinfo = appList_[pid];
    if (appinfo.first != uid) {
        HILOGE("uid don't match the pid");
        return;
    }
    appList_.erase(pid);
    HILOGI("the app is deregistered, pid is: %{public}d, uid is %{public}d", pid, uid);
}

void PurgeableMemManager::DeregisterActiveApps(int32_t pid, int32_t uid)
{
    if (!CheckCallingToken()) {
        HILOGW("AddSubscriber not allowed");
        return;
    }
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::DeregisterActiveAppsInner, this, pid, uid);
    handler_->PostImmediateTask(func);
}

void PurgeableMemManager::ChangeAppStateInner(int32_t pid, int32_t uid, int32_t state)
{
    {
        std::lock_guard<std::mutex> lockAppList(mutexAppList);
        if (appList_.find(pid) == appList_.end()) {
            HILOGE("the app is not registered");
            return;
        }
        std::pair<int32_t, int32_t> appinfo = appList_[pid];
        if (appinfo.first != uid) {
            HILOGE("uid don't match the pid");
            return;
        }
        int32_t oldState = appList_[pid].second;
        appList_[pid].second = state;
        HILOGI("state is changed, old state: %{public}d, new state: %{public}d pid: %{public}d, uid: %{public}d",
            oldState, appList_[pid].second, pid, uid);
    }

    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers);
    auto iter = appStateSubscribers_.begin();
    while (iter != appStateSubscribers_.end()) {
        HILOGI("do OnAppStateChanged");
        (*iter)->OnAppStateChanged(pid, uid, state);
        iter++;
    }
}

void PurgeableMemManager::ChangeAppState(int32_t pid, int32_t uid, int32_t state)
{
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::ChangeAppStateInner, this, pid, uid, state);
    handler_->PostImmediateTask(func);
}

void PurgeableMemManager::NotifyMemoryLevelInner(SystemMemoryLevel level)
{
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers);
    auto iter = appStateSubscribers_.begin();
    while (iter != appStateSubscribers_.end()) {
        (*iter)->OnTrim(level);
        iter++;
    }
}

void PurgeableMemManager::NotifyMemoryLevel(SystemMemoryLevel level)
{
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::NotifyMemoryLevelInner, this, level);
    handler_->PostImmediateTask(func);
}

bool PurgeableMemManager::CheckCallingToken()
{
    Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenFlag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenFlag == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        tokenFlag == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        return true;
    }
    return false;
}

void PurgeableMemManager::ReclaimAllInner()
{
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers);
    std::lock_guard<std::mutex> lockAppList(mutexAppList);
    auto subscriberIter = appStateSubscribers_.begin();
    int pid = -1, uid = -1;
    while (subscriberIter != appStateSubscribers_.end()) {
        HILOGI("do ForceReclaim");
        auto appListIter = appList_.begin();
        while (appListIter != appList_.end()) {
            pid = appListIter->first;
            std::pair<int32_t, int32_t> appinfo = appListIter->second;
            uid = appinfo.first;
            (*subscriberIter)->ForceReclaim(pid, uid);
            appListIter++;
        }
        subscriberIter++;
    }
}

void PurgeableMemManager::ReclaimAll()
{
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::ReclaimAllInner, this);
    handler_->PostImmediateTask(func);
}

void PurgeableMemManager::ReclaimInner(int32_t pid)
{
    int32_t uid = -1;
    {
        std::lock_guard<std::mutex> lockAppList(mutexAppList);
        if (appList_.find(pid) == appList_.end()) {
            HILOGE("the app is not registered");
            return;
        }

        std::pair<int32_t, int32_t> appinfo = appList_[pid];
        uid = appinfo.first;
    }
    std::lock_guard<std::mutex> lockSubscriber(mutexSubscribers);
    auto iter = appStateSubscribers_.begin();
    while (iter != appStateSubscribers_.end()) {
        HILOGI("do ForceReclaim");
        (*iter)->ForceReclaim(pid, uid);
        iter++;
    }
}

void PurgeableMemManager::Reclaim(int32_t pid)
{
    if (!initialized_) {
        HILOGE("is not initialized");
        return;
    }
    std::function<void()> func = std::bind(&PurgeableMemManager::ReclaimInner, this, pid);
    handler_->PostImmediateTask(func);
}

bool PurgeableMemManager::isNumeric(std::string const &str)
{
    return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

void PurgeableMemManager::Test(int fd, std::vector<std::string> &params)
{
    if (params.size() == PARAM_SIZE_ONTRIM && params[FIRST_ARG_INDEX] == "-t") {
        if (params[SYSTEM_MEMORY_LEVEL_INDEX].length() != 1) {
            return;
        }
        int32_t level = params[SYSTEM_MEMORY_LEVEL_INDEX][0] - '0';
        switch (level) {
            case MEMORY_LEVEL_MODERATE:
                NotifyMemoryLevel(SystemMemoryLevel::MEMORY_LEVEL_MODERATE);
                break;
            case MEMORY_LEVEL_LOW:
                NotifyMemoryLevel(SystemMemoryLevel::MEMORY_LEVEL_LOW);
                break;
            case MEMORY_LEVEL_CRITICAL:
                NotifyMemoryLevel(SystemMemoryLevel::MEMORY_LEVEL_CRITICAL);
                break;
            default:
                return;
        }
    } else if (params.size() > 0 && params[FIRST_ARG_INDEX] == "-f") {
        if (params.size() == PARAM_SIZE_RECLAIMALL) {
            ReclaimAll();
        }
        if (params.size() == PARAM_SIZE_RECLAIM_BY_PID && params[SECOND_ARG_INDEX] == "-p" &&
            isNumeric(params[PID_INDEX])) {
            Reclaim(std::stoi(params[PID_INDEX]));
        }
    } else if (params.size() == PARAM_SIZE_SHOW_APPS && params[FIRST_ARG_INDEX] == "-s") {
        ShowRegistedApps(fd);
    }
}

void PurgeableMemManager::ShowRegistedApps(int fd)
{
    std::lock_guard<std::mutex> lockAppList(mutexAppList);
    int32_t pid, uid, state;
    auto appListIter = appList_.begin();
    while (appListIter != appList_.end()) {
        pid = appListIter->first;
        std::pair<int32_t, int32_t> appinfo = appListIter->second;
        uid = appinfo.first;
        state = appinfo.second;
        dprintf(fd, "pid:%d, uid:%d state:%s\n", pid, uid, (state == APP_STATE_FOREGROUND) ?
            "Foreground" : "Background");
        appListIter++;
    }
}

} // namespace Memory
} // namespace OHOS