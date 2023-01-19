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

#include "app_state_subscriber_proxy.h"

#include "memmgr_log.h"
#include <message_parcel.h>

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "AppStateSubscriberProxy";
}
AppStateSubscriberProxy::AppStateSubscriberProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IAppStateSubscriber> (impl)
{}
AppStateSubscriberProxy::~AppStateSubscriberProxy() {}

void AppStateSubscriberProxy::OnConnected()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(AppStateSubscriberProxy::GetDescriptor())) {
        HILOGE("write interface token failed");
        return;
    }
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    int32_t ret = remote->SendRequest(ON_CONNECTED, data, reply, option);
    if (ret != ERR_NONE) {
        HILOGE("send request failed, error code: %d", ret);
    }
}

void AppStateSubscriberProxy::OnDisconnected()
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(AppStateSubscriberProxy::GetDescriptor())) {
        HILOGE("write interface token failed");
        return;
    }

    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    int32_t ret = remote->SendRequest(ON_DISCONNECTED, data, reply, option);
    if (ret != ERR_NONE) {
        HILOGE("send request failed, error code: %d", ret);
    }
}

void AppStateSubscriberProxy::OnAppStateChanged(int32_t pid, int32_t uid, int32_t state)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(AppStateSubscriberProxy::GetDescriptor())) {
        HILOGE("write interface token failed");
        return;
    }

    if (!data.WriteInt32(pid) || !data.WriteInt32(uid) || !data.WriteInt32(state)) {
        HILOGE("write app state failed");
        return;
    }
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    int32_t ret = remote->SendRequest(ON_APP_STATE_CHANGED, data, reply, option);
    if (ret != ERR_NONE) {
        HILOGE("send request failed, error code: %d", ret);
    }
}
void AppStateSubscriberProxy::OnTrim(SystemMemoryLevel level)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(AppStateSubscriberProxy::GetDescriptor())) {
        HILOGE("write interface token failed");
        return;
    }

    if (!data.WriteInt32(static_cast<int32_t>(level))) {
        HILOGE("write memorylevel of system failed");
        return;
    }

    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    int32_t ret = remote->SendRequest(FORCE_RECLAIM, data, reply, option);
    if (ret != ERR_NONE) {
        HILOGE("send request failed, error code: %d", ret);
    }
}
void AppStateSubscriberProxy::ForceReclaim(int32_t pid, int32_t uid)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOGE("remote is dead.");
        return;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(AppStateSubscriberProxy::GetDescriptor())) {
        HILOGE("write interface token failed");
        return;
    }

    if (!data.WriteInt32(pid) || !data.WriteInt32(uid)) {
        HILOGE("write pid and uid failed");
        return;
    }

    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    int32_t ret = remote->SendRequest(ON_TRIM, data, reply, option);
    if (ret != ERR_NONE) {
        HILOGE("send request failed, error code: %d", ret);
    }
}
} // namespace Memory
} // namespace OHOS

