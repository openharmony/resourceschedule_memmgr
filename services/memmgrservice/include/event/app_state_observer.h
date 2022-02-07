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

#ifndef OHOS_MEMORY_MEMMGR_APP_STATE_OBSERVER_H
#define OHOS_MEMORY_MEMMGR_APP_STATE_OBSERVER_H

#include "application_state_observer_stub.h"

namespace OHOS {
namespace Memory {
class AppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    /**
     * Application foreground state changed callback.
     *
     * @param appStateData Application Process data.
     */
    virtual void OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData) override;

    /**
     * Will be called when the ability state changes.
     *
     * @param abilityStateData Ability state data.
     */
    virtual void OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override;

    /**
     * Will be called when the extension state changes.
     *
     * @param abilityStateData Extension state data.
     */
    virtual void OnExtensionStateChanged(const AppExecFwk::AbilityStateData &abilityStateData) override;

    /**
     * Will be called when the process start.
     *
     * @param processData Process data.
     */
    virtual void OnProcessCreated(const AppExecFwk::ProcessData &processData) override;

    /**
     * Will be called when the process die.
     *
     * @param processData Process data.
     */
    virtual void OnProcessDied(const AppExecFwk::ProcessData &processData) override;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_APP_STATE_OBSERVER_H
