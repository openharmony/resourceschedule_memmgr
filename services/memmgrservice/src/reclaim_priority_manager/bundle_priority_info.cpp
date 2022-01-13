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
}

BundlePriorityInfo::BundlePriorityInfo(std::string name, int bundleUid, int priority):name_(name),
    uid_(bundleUid), priority_(priority)
{
}

BundlePriorityInfo::~BundlePriorityInfo()
{
    delete this;
}

bool BundlePriorityInfo::ProcessExistInBundle(pid_t pid)
{
    if (processes_.count(pid) == 0) {
        return false;
    }
    return true;
}

void BundlePriorityInfo::AddProcess(ProcessPriorityInfo &newProcess)
{
    processes_.insert(std::make_pair(newProcess.pid_, newProcess));
}

void BundlePriorityInfo::RemoveProcessById(pid_t pid)
{
    processes_.erase(pid);
}

int BundlePriorityInfo::GetProcessCount()
{
    return processes_.size();
}

ProcessPriorityInfo& BundlePriorityInfo::FindProcessInfoById(pid_t pid)
{
    return processes_.at(pid);
}

int BundlePriorityInfo::GetMinProcessPriority()
{
    int min_priority = RECLAIM_PRIORITY_UNKNOWN;
    for (auto i = processes_.begin(); i != processes_.end(); ++i) {
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
    int targetPriority = GetMinProcessPriority();
    SetPriority(targetPriority);
    HILOGI("bundleName=%{public}s, priority=%{public}d", name_.c_str(), priority_);
}


} // namespace Memory
} // namespace OHOS
