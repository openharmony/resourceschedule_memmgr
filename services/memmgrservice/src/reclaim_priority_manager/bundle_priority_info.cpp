/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this->file except in compliance with the License.
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

#include "bundle_priority_info.h"
#include "memmgr_log.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "BundlePriorityInfo";
} // namespace

BundlePriorityInfo::BundlePriorityInfo(std::string name, int bundleUid):name_(name), uid_(bundleUid)
{
    this->priority_ = RECLAIM_PRIORITY_UNKNOWN;
    this->accountId_ = bundleUid / USER_ID_SHIFT;
    this->state_ = BundleState::STATE_DEFAULT;
}

BundlePriorityInfo::BundlePriorityInfo(std::string name, int bundleUid, int priority):name_(name),
    uid_(bundleUid), priority_(priority)
{
    this->accountId_ = bundleUid / USER_ID_SHIFT;
    this->state_ = BundleState::STATE_DEFAULT;
}

bool BundlePriorityInfo::HasProc(pid_t pid)
{
    if (procs_.count(pid) == 0) {
        return false;
    }
    return true;
}

void BundlePriorityInfo::AddProc(ProcessPriorityInfo &newProcess)
{
    procs_.insert(std::make_pair(newProcess.pid_, newProcess));
}

void BundlePriorityInfo::RemoveProcByPid(pid_t pid)
{
    procs_.erase(pid);
}

int BundlePriorityInfo::GetProcsCount()
{
    return procs_.size();
}

ProcessPriorityInfo& BundlePriorityInfo::FindProcByPid(pid_t pid)
{
    return procs_.at(pid);
}

int BundlePriorityInfo::GetMinProcPriority()
{
    int min_priority = RECLAIM_PRIORITY_UNKNOWN;
    for (auto i = procs_.begin(); i != procs_.end(); ++i) {
        if (i->second.priority_ < min_priority) {
            min_priority = i->second.priority_;
        }
    }
    return min_priority;
}

void BundlePriorityInfo::SetPriority(int targetPriority)
{
    priority_  = targetPriority;
}

void BundlePriorityInfo::UpdatePriority()
{
    int targetPriority = GetMinProcPriority();
    if (targetPriority >= RECLAIM_PRIORITY_MAX) {
        targetPriority = RECLAIM_PRIORITY_MAX;
    } else if (targetPriority <= RECLAIM_PRIORITY_MIN) {
        targetPriority = RECLAIM_PRIORITY_MIN;
    }
    SetPriority(targetPriority);
    HILOGI("bundleName=%{public}s, priority=%{public}d", name_.c_str(), priority_);
}
} // namespace Memory
} // namespace OHOS
