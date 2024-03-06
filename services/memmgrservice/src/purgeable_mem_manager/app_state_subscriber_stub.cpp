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

#include "memmgr_log.h"
#include "app_state_subscriber_stub.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "AppStateSubscriberStub";
}

AppStateSubscriberStub::AppStateSubscriberStub() {}
AppStateSubscriberStub::~AppStateSubscriberStub() {}

int32_t AppStateSubscriberStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                                MessageOption &option)
{
    std::u16string descriptor = AppStateSubscriberStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("local descriptor not match remote");
        return -1;
    }
    HILOGI("get remote request code: %{public}d", code);
    return OnRemoteRequestInner(code, data, reply, option);
}

int32_t AppStateSubscriberStub::OnRemoteRequestInner(uint32_t code, MessageParcel &data, MessageParcel &reply,
                                                     MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AppStateSubscriberInterfaceCode::ON_CONNECTED): {
            return HandleOnConnected();
        }
        case static_cast<uint32_t>(AppStateSubscriberInterfaceCode::ON_DISCONNECTED): {
            return HandleOnDisconnected();
        }
        case static_cast<uint32_t>(AppStateSubscriberInterfaceCode::ON_APP_STATE_CHANGED): {
            return HandleOnAppStateChanged(data);
        }
        case static_cast<uint32_t>(AppStateSubscriberInterfaceCode::ON_TRIM): {
            return HandleOnTrim(data);
        }
        case static_cast<uint32_t>(AppStateSubscriberInterfaceCode::FORCE_RECLAIM): {
            return HandleForceReclaim(data);
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

int32_t AppStateSubscriberStub::HandleOnConnected()
{
    OnConnected();
    return 0;
}

int32_t AppStateSubscriberStub::HandleOnDisconnected()
{
    OnDisconnected();
    return 0;
}

int32_t AppStateSubscriberStub::HandleOnAppStateChanged(MessageParcel &data)
{
    int32_t pid;
    int32_t uid;
    int32_t state;
    if (!data.ReadInt32(pid) || !data.ReadInt32(uid) || !data.ReadInt32(state)) {
        HILOGE("read pid, uid or new app state failed");
        return -1;
    }
    OnAppStateChanged(pid, uid, state);
    return 0;
}

int32_t AppStateSubscriberStub::HandleOnTrim(MessageParcel &data)
{
    int32_t levelTemp;
    if (!data.ReadInt32(levelTemp)) {
        HILOGE("read memory level of system failed");
        return -1;
    }
    SystemMemoryLevel level = static_cast<SystemMemoryLevel>(levelTemp);
    OnTrim(level);
    return 0;
}

int32_t AppStateSubscriberStub::HandleForceReclaim(MessageParcel &data)
{
    int32_t pid;
    int32_t uid;
    if (!data.ReadInt32(pid) || !data.ReadInt32(uid)) {
        HILOGE("read pid or uid failed");
        return -1;
    }
    ForceReclaim(pid, uid);
    return 0;
}
} // namespace Memory
} // namespace OHOS