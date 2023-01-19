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

#include "app_state_subscriber.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "memmgr_log.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "AppStateSubscriber";
}
AppStateSubscriber::AppStateSubscriber()
{
#ifdef USE_PURGEABLE_MEMORY
    impl_ = new (std::nothrow) AppStateSubscriberImpl(*this);
#endif
}

AppStateSubscriber::~AppStateSubscriber()
{
#ifdef USE_PURGEABLE_MEMORY
    impl_->OnListenerDied();
#endif
}

void AppStateSubscriber::OnConnected() {}
void AppStateSubscriber::OnDisconnected() {}
void AppStateSubscriber::OnAppStateChanged(int32_t pid, int32_t uid, int32_t state) {}
void AppStateSubscriber::OnTrim(SystemMemoryLevel level) {}
void AppStateSubscriber::ForceReclaim(int32_t pid, int32_t uid) {}
void AppStateSubscriber::OnRemoteDied(const wptr<IRemoteObject> &object) {}

#ifdef USE_PURGEABLE_MEMORY
const sptr<AppStateSubscriber::AppStateSubscriberImpl> AppStateSubscriber::GetImpl() const
{
    return impl_;
}

AppStateSubscriber::AppStateSubscriberImpl::AppStateSubscriberImpl(AppStateSubscriber &subscriber)
    : subscriber_(subscriber)
{
    recipient_ = new (std::nothrow) DeathRecipient(*this);
}

void AppStateSubscriber::AppStateSubscriberImpl::OnConnected()
{
    if (GetMemMgrProxy() && recipient_ != nullptr) {
        proxy_->AsObject()->AddDeathRecipient(recipient_);
    }
    HILOGI("CALLED");
    std::lock_guard<std::mutex> lock(mutex_alive);
    if (!isListenerAlive) {
        HILOGE("Listener already died");
        return;
    }
    subscriber_.OnConnected();
}

void AppStateSubscriber::AppStateSubscriberImpl::OnDisconnected()
{
    if (GetMemMgrProxy() && recipient_ != nullptr) {
        proxy_->AsObject()->RemoveDeathRecipient(recipient_);
    }
    HILOGI("CALLED");
    std::lock_guard<std::mutex> lock(mutex_alive);
    if (!isListenerAlive) {
        HILOGE("Listener already died");
        return;
    }
    subscriber_.OnDisconnected();
}

void AppStateSubscriber::AppStateSubscriberImpl::OnAppStateChanged(int32_t pid, int32_t uid, int32_t state)
{
    HILOGI("CALLED");
    std::lock_guard<std::mutex> lock(mutex_alive);
    if (!isListenerAlive) {
        HILOGE("Listener already died");
        return;
    }
    subscriber_.OnAppStateChanged(pid, uid, state);
}

void AppStateSubscriber::AppStateSubscriberImpl::ForceReclaim(int32_t pid, int32_t uid)
{
    HILOGI("CALLED");
    std::lock_guard<std::mutex> lock(mutex_alive);
    if (!isListenerAlive) {
        HILOGE("Listener already died");
        return;
    }
    subscriber_.ForceReclaim(pid, uid);
}

void AppStateSubscriber::AppStateSubscriberImpl::OnTrim(SystemMemoryLevel level)
{
    HILOGI("CALLED");
    std::lock_guard<std::mutex> lock(mutex_alive);
    if (!isListenerAlive) {
        HILOGE("Listener already died");
        return;
    }
    subscriber_.OnTrim(level);
}

void AppStateSubscriber::AppStateSubscriberImpl::OnListenerDied()
{
    HILOGI("CALLED");
    std::lock_guard<std::mutex> lock(mutex_alive);
    isListenerAlive = false;
}

bool AppStateSubscriber::AppStateSubscriberImpl::GetMemMgrProxy()
{
    if (proxy_) {
        return true;
    }
    std::lock_guard<std::mutex> lock(mutex_proxy);
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        return false;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(MEMORY_MANAGER_SA_ID);
    if (!remoteObject) {
        return false;
    }

    proxy_ = iface_cast<IMemMgr>(remoteObject);
    if ((!proxy_) || (proxy_->AsObject() == nullptr)) {
        return false;
    }
    return true;
}

AppStateSubscriber::AppStateSubscriberImpl::DeathRecipient::DeathRecipient(AppStateSubscriberImpl &subscriberImpl)
    : subscriberImpl_(subscriberImpl) {}

AppStateSubscriber::AppStateSubscriberImpl::DeathRecipient::~DeathRecipient() {}

void AppStateSubscriber::AppStateSubscriberImpl::DeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    subscriberImpl_.proxy_ = nullptr;
    subscriberImpl_.subscriber_.OnRemoteDied(object);
}
#endif // USE_PURGEABLE_MEMORY
}
}
