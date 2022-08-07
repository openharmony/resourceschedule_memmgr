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

#include "extension_connection_observer.h"
#include "memmgr_log.h"
#include "reclaim_priority_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "ExtConnObs";
}

void ExtensionConnectionObserver::OnExtensionConnected(const AbilityRuntime::ConnectionData& data)
{
    HILOGI("%{public}s(%{public}d,%{public}d) --> [%{public}s|%{public}s|%{public}s](%{public}d,%{public}d) "
        "with type %{public}d", data.callerName.c_str(), data.callerPid, data.callerUid,
        data.extensionBundleName.c_str(), data.extensionModuleName.c_str(), data.extensionName.c_str(),
        data.extensionPid, data.extensionUid, static_cast<int32_t>(data.extensionType));

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(data.extensionPid, data.extensionUid,
        data.extensionBundleName, AppStateUpdateReason::BIND_EXTENSION);
}

void ExtensionConnectionObserver::OnExtensionDisconnected(const AbilityRuntime::ConnectionData& data)
{
    HILOGI("%{public}s(%{public}d,%{public}d) -/-> [%{public}s|%{public}s|%{public}s](%{public}d,%{public}d) "
        "with type %{public}d", data.callerName.c_str(), data.callerPid, data.callerUid,
        data.extensionBundleName.c_str(), data.extensionModuleName.c_str(), data.extensionName.c_str(),
        data.extensionPid, data.extensionUid, static_cast<int32_t>(data.extensionType));
}

void ExtensionConnectionObserver::OnDlpAbilityOpened(const AbilityRuntime::DlpStateData& data)
{
    HILOGI("%{public}s(%{public}d,%{public}d) --> [%{public}s|%{public}s|%{public}s](%{public}d,%{public}d)",
        data.callerName.c_str(), data.callerPid, data.callerUid, data.targetBundleName.c_str(),
        data.targetModuleName.c_str(), data.targetAbilityName.c_str(), data.targetPid, data.targetUid);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(data.targetPid, data.targetUid,
        data.targetBundleName, AppStateUpdateReason::BIND_EXTENSION);
}

void ExtensionConnectionObserver::OnDlpAbilityClosed(const AbilityRuntime::DlpStateData& data)
{
    HILOGI("%{public}s(%{public}d,%{public}d) -/-> [%{public}s|%{public}s|%{public}s](%{public}d,%{public}d)",
        data.callerName.c_str(), data.callerPid, data.callerUid, data.targetBundleName.c_str(),
        data.targetModuleName.c_str(), data.targetAbilityName.c_str(), data.targetPid, data.targetUid);
}

void ExtensionConnectionObserver::OnServiceDied()
{
    HILOGE("called");
}
} // namespace Memory
} // namespace OHOS
