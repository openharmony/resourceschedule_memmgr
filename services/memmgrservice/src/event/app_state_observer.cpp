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

#include "app_state_observer.h"
#include "memmgr_log.h"
#include "reclaim_priority_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "AppStateObserver";
}

const std::map<int, AppStateUpdateReason> stateReasonMap_ = {
    { static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_READY), AppStateUpdateReason::PROCESS_READY },
    { static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND), AppStateUpdateReason::FOREGROUND },
    { static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND), AppStateUpdateReason::BACKGROUND },
};

const std::map<int, std::string> stateReasonStrMap_ = {
    { static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_READY), "PROCESS_READY" },
    { static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND), "FOREGROUND" },
    { static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND), "BACKGROUND" },
};

const std::map<int, AppStateUpdateReason> extensionStateReasonMap_ = {
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_CREATE), AppStateUpdateReason::CREATE_PROCESS },
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_READY), AppStateUpdateReason::CREATE_PROCESS },
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_CONNECTED),
      AppStateUpdateReason::FOREGROUND_BIND_EXTENSION },
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_DISCONNECTED),
      AppStateUpdateReason::NO_BIND_EXTENSION },
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_TERMINATED),
      AppStateUpdateReason::PROCESS_TERMINATED },
};

const std::map<int, std::string> extensionStateReasonStrMap_ = {
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_CREATE), "EXTENSION_STATE_CREATE" },
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_READY), "EXTENSION_STATE_READY" },
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_CONNECTED), "EXTENSION_STATE_CONNECTED" },
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_DISCONNECTED), "EXTENSION_STATE_DISCONNECTED" },
    { static_cast<int32_t>(AppExecFwk::ExtensionState::EXTENSION_STATE_TERMINATED), "EXTENSION_STATE_TERMINATED" },
};

void AppStateObserver::OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData)
{
    // no pid here !
    HILOGI("uid=%{public}d, bundleName=%{public}s, state=%{public}d, ",
        appStateData.uid, appStateData.bundleName.c_str(), appStateData.state);
}

void AppStateObserver::OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    std::string appName = abilityStateData.bundleName;
    auto stateReasonPair = stateReasonMap_.find(abilityStateData.abilityState);
    auto stateReasonStrPair = stateReasonStrMap_.find(abilityStateData.abilityState);
    if (stateReasonPair != stateReasonMap_.end() && stateReasonStrPair != stateReasonStrMap_.end()) {
        AppStateUpdateReason reason = stateReasonPair->second;
        std::string reasonStr = stateReasonStrPair->second;
        ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(
            abilityStateData.pid, abilityStateData.uid, abilityStateData.bundleName, reason);
        HILOGI("called, uid=%{public}d, pid=%{public}d, bundleName=%{public}s %{public}s",
            abilityStateData.uid, abilityStateData.pid, abilityStateData.bundleName.c_str(), reasonStr.c_str());
    } else {
        HILOGI("called, uid=%{public}d, pid=%{public}d, bundleName=%{public}s %{public}s",
            abilityStateData.uid, abilityStateData.pid, abilityStateData.bundleName.c_str(), "Skiped!");
    }
}

void AppStateObserver::OnExtensionStateChanged(const AppExecFwk::AbilityStateData &extensionStateData)
{
    HILOGI("uid=%{public}d, pid=%{public}d, bundleName=%{public}s, abilityName=%{public}s, abilityState=%{public}d",
        extensionStateData.uid, extensionStateData.pid, extensionStateData.bundleName.c_str(),
        extensionStateData.abilityName.c_str(), extensionStateData.abilityState);
}

void AppStateObserver::OnProcessCreated(const AppExecFwk::ProcessData &processData)
{
    HILOGI("uid=%{public}d, pid=%{public}d, bundleName=%{public}s",
        processData.uid, processData.pid, processData.bundleName.c_str());
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(processData.pid, processData.uid,
        processData.bundleName, AppStateUpdateReason::CREATE_PROCESS);
}

void AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    HILOGI("uid=%{public}d, pid=%{public}d, bundleName=%{public}s",
        processData.uid, processData.pid, processData.bundleName.c_str());
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(processData.pid, processData.uid,
        processData.bundleName, AppStateUpdateReason::PROCESS_TERMINATED);
}
} // namespace Memory
} // namespace OHOS
