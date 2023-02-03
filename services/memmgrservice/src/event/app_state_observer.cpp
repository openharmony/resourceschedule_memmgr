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

void AppStateObserver::OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData)
{
    // no pid here !
    HILOGI("uid=%{public}d, bundleName=%{public}s, state=%{public}d, ",
        appStateData.uid, appStateData.bundleName.c_str(), appStateData.state);
}

void AppStateObserver::OnAbilityStateChanged(const AppExecFwk::AbilityStateData &abilityStateData)
{
    auto stateReasonPair = stateReasonMap_.find(abilityStateData.abilityState);
    auto stateReasonStrPair = stateReasonStrMap_.find(abilityStateData.abilityState);
    if (stateReasonPair != stateReasonMap_.end() && stateReasonStrPair != stateReasonStrMap_.end()) {
        AppStateUpdateReason reason = stateReasonPair->second;
        std::string reasonStr = stateReasonStrPair->second;
        ReclaimHandleRequest request;
        request.pid = abilityStateData.pid;
        request.uid = abilityStateData.uid;
        request.bundleName = abilityStateData.bundleName;
        request.reason = reason;
        ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(request);
        HILOGI("called, uid=%{public}d, pid=%{public}d, bundleName=%{public}s %{public}s",
            abilityStateData.uid, abilityStateData.pid, abilityStateData.bundleName.c_str(), reasonStr.c_str());
    } else {
        HILOGI("called, uid=%{public}d, pid=%{public}d, bundleName=%{public}s %{public}s",
            abilityStateData.uid, abilityStateData.pid, abilityStateData.bundleName.c_str(), "Skiped!");
    }
}

void AppStateObserver::OnExtensionStateChanged(const AppExecFwk::AbilityStateData &extensionStateData)
{
}

void AppStateObserver::OnProcessCreated(const AppExecFwk::ProcessData &processData)
{
    HILOGI("uid=%{public}d, pid=%{public}d, bundleName=%{public}s",
        processData.uid, processData.pid, processData.bundleName.c_str());

    ReclaimHandleRequest request;
    request.pid = processData.pid;
    request.uid = processData.uid;
    request.bundleName = processData.bundleName;
    request.reason = AppStateUpdateReason::CREATE_PROCESS;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(request);
}

void AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    HILOGI("uid=%{public}d, pid=%{public}d, bundleName=%{public}s",
        processData.uid, processData.pid, processData.bundleName.c_str());

    ReclaimHandleRequest request;
    request.pid = processData.pid;
    request.uid = processData.uid;
    request.bundleName = processData.bundleName;
    request.reason = AppStateUpdateReason::PROCESS_TERMINATED;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(request);
}
} // namespace Memory
} // namespace OHOS
