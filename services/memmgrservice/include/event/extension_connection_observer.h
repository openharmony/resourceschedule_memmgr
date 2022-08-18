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

#ifndef OHOS_MEMORY_MEMMGR_CONNECTION_OBSERVER_H
#define OHOS_MEMORY_MEMMGR_CONNECTION_OBSERVER_H

#include "connection_observer.h"

namespace OHOS {
namespace Memory {
class ExtensionConnectionObserver : public AbilityRuntime::ConnectionObserver {
public:
    /**
     * called when extension was connected.
     *
     * @param data connection relationship data.
     */
    virtual void OnExtensionConnected(const AbilityRuntime::ConnectionData& data) override;

    /**
     * called when extension was disconnected.
     *
     * @param data connection relationship data.
     */
    virtual void OnExtensionDisconnected(const AbilityRuntime::ConnectionData& data) override;

    /**
     * called when dlp ability was started.
     *
     * @param data dlp state data.
     */
    virtual void OnDlpAbilityOpened(const AbilityRuntime::DlpStateData& data) override;

    /**
     * called when dlp ability was terminated.
     *
     * @param data dlp state data.
     */
    virtual void OnDlpAbilityClosed(const AbilityRuntime::DlpStateData& data) override;

    /**
     * called when service was died.
     *
     */
    virtual void OnServiceDied() override;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_CONNECTION_OBSERVER_H
