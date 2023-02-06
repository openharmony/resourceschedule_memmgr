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

#ifndef OHOS_MEMORY_MEMMGR_APP_STATE_SUBSCRIBER_STUB_H
#define OHOS_MEMORY_MEMMGR_APP_STATE_SUBSCRIBER_STUB_H

#include <iremote_stub.h>

#include "iapp_state_subscriber.h"
namespace OHOS {
namespace Memory {
class AppStateSubscriberStub : public IRemoteStub<IAppStateSubscriber> {
public:
    AppStateSubscriberStub();
    ~AppStateSubscriberStub() override;
    DISALLOW_COPY_AND_MOVE(AppStateSubscriberStub);

    /* *
     * @brief Request service code and service data.
     *
     * @param code Service request code.
     * @param data MessageParcel object.
     * @param reply Local service response.
     * @param option Point out async or sync.
     * @return ERR_OK if success, else fail.
     */
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleOnConnected();
    int32_t HandleOnDisconnected();
    int32_t HandleOnAppStateChanged(MessageParcel &data);
    int32_t HandleOnTrim(MessageParcel &data);
    int32_t HandleForceReclaim(MessageParcel &data);
    int32_t OnRemoteRequestInner(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_APP_STATE_SUBSCRIBER_STUB_H