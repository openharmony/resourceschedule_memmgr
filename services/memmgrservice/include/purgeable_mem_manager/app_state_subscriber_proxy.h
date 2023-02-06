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

#ifndef OHOS_MEMORY_MEMMGR_APP_STATE_SUBSCRIBER_PROXY_H
#define OHOS_MEMORY_MEMMGR_APP_STATE_SUBSCRIBER_PROXY_H

#include <iremote_proxy.h>

#include "iapp_state_subscriber.h"

namespace OHOS {
namespace Memory {
class AppStateSubscriberProxy : public IRemoteProxy<IAppStateSubscriber> {
public:
    AppStateSubscriberProxy() = delete;
    explicit AppStateSubscriberProxy(const sptr<IRemoteObject> &impl);
    ~AppStateSubscriberProxy() override;
    DISALLOW_COPY_AND_MOVE(AppStateSubscriberProxy);

    /* *
     * @brief Called back when the subscriber is connected to Memory Manager Service.
     */
    void OnConnected() override;

    /* *
     * @brief Called back when the subscriber is disconnected to Memory Manager Service.
     */
    void OnDisconnected() override;

    /* *
     * @brief Called back when app state change.
     *
     * @param pid pid of the process whose state is changed.
     * @param uid uid of the process whose state is changed.
     * @param state new state of the app.
     */
    void OnAppStateChanged(int32_t pid, int32_t uid, int32_t state) override;

    /* *
     * @brief Called back when need to reclaim memory.
     *
     * @param pid pid of the process which need to reclaim.
     * @param uid uid of the process which need to reclaim.
     */
    void ForceReclaim(int32_t pid, int32_t uid) override;

    /* *
     * @brief Called back when get systemMemoryLevel message.
     *
     * @param level current memory level.
     */
    void OnTrim(SystemMemoryLevel level) override;

private:
    static inline BrokerDelegator<AppStateSubscriberProxy> delegator_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_APP_STATE_SUBSCRIBER_PROXY_H
