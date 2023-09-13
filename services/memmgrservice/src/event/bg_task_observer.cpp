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

#include "bg_task_observer.h"
#include "memmgr_log.h"
#include "reclaim_priority_manager.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "BgTaskObserver";
}

void BgTaskObserver::OnConnected()
{
    HILOGI("called");
}

void BgTaskObserver::OnDisconnected()
{
    HILOGE("called");
}

void BgTaskObserver::OnTransientTaskStart(
    const std::shared_ptr<BackgroundTaskMgr::TransientTaskAppInfo>& ttInfo)
{
    if (ttInfo == nullptr) {
        HILOGE("called with null TransientTaskAppInfo");
        return;
    }
    std::string pkgName = ttInfo->GetPackageName();
    int uid = ttInfo->GetUid();
    int pid = ttInfo->GetPid();
    HILOGD("called, pkg=%{public}s, uid=%{public}d, pid=%{public}d", pkgName.c_str(), uid, pid);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(
        SingleRequest({pid, uid, pkgName}, AppStateUpdateReason::SUSPEND_DELAY_START));
}

void BgTaskObserver::OnTransientTaskEnd(
    const std::shared_ptr<BackgroundTaskMgr::TransientTaskAppInfo>& ttInfo)
{
    if (ttInfo == nullptr) {
        HILOGE("called with null TransientTaskAppInfo");
        return;
    }
    std::string pkgName = ttInfo->GetPackageName();
    int uid = ttInfo->GetUid();
    int pid = ttInfo->GetPid();
    HILOGD("called, pkg=%{public}s, uid=%{public}d, pid=%{public}d", pkgName.c_str(), uid, pid);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(
        SingleRequest({pid, uid, pkgName}, AppStateUpdateReason::SUSPEND_DELAY_END));
}

void BgTaskObserver::OnContinuousTaskStart(
    const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>& ctInfo)
{
    if (ctInfo == nullptr) {
        HILOGE("called with null ContinuousTaskCallbackInfo");
        return;
    }
    int32_t type = ctInfo->GetTypeId();
    uid_t uid = ctInfo->GetCreatorUid();
    pid_t pid = ctInfo->GetCreatorPid();
    std::string abilityName = ctInfo->GetAbilityName();
    HILOGD("called, abilityName=%{public}s, type=%{public}d uid=%{public}d, pid=%{public}d",
        abilityName.c_str(), type, uid, pid);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(
        SingleRequest({pid, uid, abilityName}, AppStateUpdateReason::BACKGROUND_RUNNING_START));
}

void BgTaskObserver::OnContinuousTaskStop(
    const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo>& ctInfo)
{
    if (ctInfo == nullptr) {
        HILOGE("called with null ContinuousTaskCallbackInfo");
        return;
    }
    int32_t type = ctInfo->GetTypeId();
    uid_t uid = ctInfo->GetCreatorUid();
    pid_t pid = ctInfo->GetCreatorPid();
    std::string abilityName = ctInfo->GetAbilityName();
    HILOGD("called, abilityName=%{public}s, type=%{public}d uid=%{public}d, pid=%{public}d",
        abilityName.c_str(), type, uid, pid);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(
        SingleRequest({pid, uid, abilityName}, AppStateUpdateReason::BACKGROUND_RUNNING_END));
}

void BgTaskObserver::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    HILOGE("called");
}
} // namespace Memory
} // namespace OHOS
