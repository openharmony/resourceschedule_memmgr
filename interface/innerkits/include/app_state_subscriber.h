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

#ifndef OHOS_MEMORY_MEMMGR_APP_STATE_SUBSCRIBER_H
#define OHOS_MEMORY_MEMMGR_APP_STATE_SUBSCRIBER_H

#include <iremote_broker.h>

#include "i_mem_mgr.h"
#ifdef USE_PURGEABLE_MEMORY
#include "app_state_subscriber_stub.h"
#else
#include "memory_level_constants.h"
#endif

namespace OHOS {
namespace Memory {
class AppStateSubscriber {
public:
    /* *
     * Default constructor used to create a instance.
     */
    AppStateSubscriber();

    /* *
     * Default destructor.
     */
    virtual ~AppStateSubscriber();

    /* *
     * Called back when the subscriber is connected to Memory Manager Service.
     */
    virtual void OnConnected();

    /* *
     * Called back when the subscriber is disconnected to Memory Manager Service.
     */
    virtual void OnDisconnected();

    /* *
     * @brief Called back when app state change.
     *
     * @param pid pid of the process whose state is changed.
     * @param uid uid of the process whose state is changed.
     * @param state new state of the app.
     */
    virtual void OnAppStateChanged(int32_t pid, int32_t uid, int32_t state);

    /* *
     * @brief Called back when need to reclaim memory.
     *
     * @param pid pid of the process which need to reclaim.
     * @param uid uid of the process which need to reclaim.
     */
    virtual void ForceReclaim(int32_t pid, int32_t uid);

    /* *
     * @brief Called back when get systemMemoryLevel message.
     *
     * @param level current memory level.
     */
    virtual void OnTrim(SystemMemoryLevel level);

    /* *
     * @brief Called back when the Memory Manager Service has died.
     */
    virtual void OnRemoteDied(const wptr<IRemoteObject> &object);

#ifdef USE_PURGEABLE_MEMORY
private:
    class AppStateSubscriberImpl final : public AppStateSubscriberStub {
    public:
        class DeathRecipient final : public IRemoteObject::DeathRecipient {
        public:
            DeathRecipient(AppStateSubscriberImpl &subscriberImpl);

            ~DeathRecipient();

            /* *
             * @brief Called back when remote object has died.
             *
             * @param object Object which has died.
             */
            void OnRemoteDied(const wptr<IRemoteObject> &object) override;

        private:
            AppStateSubscriberImpl &subscriberImpl_;
        };

    public:
        AppStateSubscriberImpl(AppStateSubscriber &subscriber);
        ~AppStateSubscriberImpl() {}

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

        /* *
         * @brief Get managed proxy of memory manager.
         *
         * @return True if success, else false.
         */
        bool GetMemMgrProxy();

        void OnListenerDied();

    public:
        AppStateSubscriber &subscriber_;
        sptr<DeathRecipient> recipient_ { nullptr };
        sptr<IMemMgr> proxy_ { nullptr };
        std::mutex mutex_proxy {};
        std::mutex mutex_alive {};

    private:
        bool isListenerAlive = true;
    };
private:
    const sptr<AppStateSubscriberImpl> GetImpl() const;

private:
    sptr<AppStateSubscriberImpl> impl_ { nullptr };

    friend class MemMgrClient;
#endif // USE_PURGEABLE_MEMORY
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_APP_STATE_SUBSCRIBER_H
