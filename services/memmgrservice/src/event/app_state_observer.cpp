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
#include <sstream>
#include "kernel_interface.h"
#include "mem_mgr_event_center.h"
#include "memmgr_log.h"
#include "reclaim_priority_manager.h"

#ifdef USE_PURGEABLE_MEMORY
#include "purgeable_mem_manager.h"
#endif

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "AppStateObserver";
const std::string SLASH = "/";
constexpr int RSS_THRESHOLD_RATIO = 80;
}

void AppStateObserver::OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData)
{
    // no pid here !
    HILOGI("uid=%{public}d, bundleName=%{public}s, state=%{public}d, ",
        appStateData.uid, appStateData.bundleName.c_str(), appStateData.state);
#ifdef USE_PURGEABLE_MEMORY
    PurgeableMemManager::GetInstance().ChangeAppState(appStateData.pid, appStateData.uid, appStateData.state);
#endif
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
    int32_t renderUid = processData.renderUid;
    int uid = -1;
    std::stringstream ss;

    ss << KernelInterface::ROOT_PROC_PATH << SLASH << processData.pid << KernelInterface::RSS_THRESHOLD_PATH;
    std::string path = ss.str();
    int32_t rssThresoldMax = KernelInterface::GetInstance().GetTotalBuffer() * RSS_THRESHOLD_RATIO / 100;
    std::string content = std::to_string(rssThresoldMax);
    KernelInterface::GetInstance().EchoToPath(path.c_str(), content.c_str());
    if (renderUid != -1) {
        uid = renderUid;
    } else {
        uid = processData.uid;
    }
    HILOGI("uid=%{public}d, pid=%{public}d, bundle=%{public}s", uid, processData.pid, processData.bundleName.c_str());
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(
        SingleRequest(processData.pid, uid, processData.bundleName, AppStateUpdateReason::CREATE_PROCESS));
}

void AppStateObserver::OnProcessDied(const AppExecFwk::ProcessData &processData)
{
    int32_t renderUid = processData.renderUid;
    int uid = -1;

    if (renderUid != -1) {
        uid = renderUid;
    } else {
        uid = processData.uid;
    }
    HILOGD("uid=%{public}d, pid=%{public}d, bundle=%{public}s", uid, processData.pid, processData.bundleName.c_str());
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(
        SingleRequest(processData.pid, uid, processData.bundleName, AppStateUpdateReason::PROCESS_TERMINATED));
    MemMgrEventCenter::GetInstance().OnProcessDied(processData.pid);
}
} // namespace Memory
} // namespace OHOS
