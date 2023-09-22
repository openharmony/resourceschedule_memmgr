/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "process_priority_info.h"
#include "memmgr_log.h"
#include "reclaim_priority_constants.h"

#include <sstream>

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "ProcessPriorityInfo";
} // namespace

ProcessPriorityInfo::ProcessPriorityInfo(pid_t pid, int bundleUid, int priority, bool isImportant)
{
    this->uid_ = bundleUid;
    this->pid_ = pid;
    this->priority_ = priority;
    this->priorityIfImportant_ = priority;
    this->isImportant_ = isImportant;
    this->isVisible_ = false;
    this->isRender_ = false;
    this->isFreground = (priority == RECLAIM_PRIORITY_FOREGROUND) ? true : false;
    this->isBackgroundRunning = false;
    this->isSuspendDelay = false;
    this->isEventStart = false;
    this->isDistDeviceConnected = false;
    this->isExtension_ = false;
    this->extensionBindStatus = EXTENSION_STATUS_BIND_UNKOWN;
}

ProcessPriorityInfo::ProcessPriorityInfo(const ProcessPriorityInfo &copyProcess)
{
    this->uid_ = copyProcess.uid_;
    this->pid_ = copyProcess.pid_;
    this->priority_ = copyProcess.priority_;
    this->priorityIfImportant_ = copyProcess.priorityIfImportant_;
    this->isImportant_ = copyProcess.isImportant_;
    this->isVisible_ = copyProcess.isVisible_;
    this->isRender_ = copyProcess.isRender_;
    this->isFreground = copyProcess.isFreground;
    this->isExtension_ = copyProcess.isExtension_;
    this->isBackgroundRunning = copyProcess.isBackgroundRunning;
    this->isSuspendDelay = copyProcess.isSuspendDelay;
    this->isEventStart = copyProcess.isEventStart;
    this->isDistDeviceConnected = copyProcess.isDistDeviceConnected;
    this->extensionBindStatus = copyProcess.extensionBindStatus;
    for (auto &pair : copyProcess.procsBindToMe_) {
        this->procsBindToMe_.insert(std::make_pair(pair.first, pair.second));
    }
    for (auto &pair : copyProcess.procsBindFromMe_) {
        this->procsBindFromMe_.insert(std::make_pair(pair.first, pair.second));
    }
}

ProcessPriorityInfo::~ProcessPriorityInfo()
{
    procsBindToMe_.clear();
    procsBindFromMe_.clear();
}

void ProcessPriorityInfo::SetPriority(int targetPriority)
{
    priority_ = targetPriority;
    HILOGD("set process[%{public}d] priority to %{public}d", pid_, priority_);
}

int32_t ProcessPriorityInfo::ExtensionConnectorsCount()
{
    return procsBindToMe_.size();
}

void ProcessPriorityInfo::ProcBindToMe(int32_t pid, int32_t uid)
{
    procsBindToMe_[pid] = uid;
    HILOGE("insert process[pid=%{public}d, uid=%{public}d] to procsBindToMe_, map size = %{public}zu",
        pid, uid, procsBindToMe_.size());
}

void ProcessPriorityInfo::ProcUnBindToMe(int32_t pid)
{
    procsBindToMe_.erase(pid);
    HILOGE("remove process[pid=%{public}d] to procsBindToMe_, map size = %{public}zu",
        pid, procsBindToMe_.size());
}

void ProcessPriorityInfo::ProcBindFromMe(int32_t pid, int32_t uid)
{
    procsBindFromMe_[pid] = uid;
    HILOGE("insert process[pid=%{public}d, uid=%{public}d] to procsBindFromMe_, map size = %{public}zu",
        pid, uid, procsBindFromMe_.size());
}

void ProcessPriorityInfo::ProcUnBindFromMe(int32_t pid)
{
    procsBindFromMe_.erase(pid);
    HILOGE("remove process[pid=%{public}d] to procsBindFromMe_, map size = %{public}zu",
        pid, procsBindFromMe_.size());
}

std::string ProcessPriorityInfo::ProcsBindToMe()
{
    std::stringstream ss;
    ss << "[";
    for (auto &pair : procsBindToMe_) {
        ss << "(pid=" << pair.first << ", uid=" << pair.second << ") ";
    }
    ss << "]";
    return ss.str();
}

std::string ProcessPriorityInfo::ProcsBindFromMe()
{
    std::stringstream ss;
    ss << "[";
    for (auto &pair : procsBindFromMe_) {
        ss << "(pid=" << pair.first << ", uid=" << pair.second << ") ";
    }
    ss << "]";
    return ss.str();
}
} // namespace Memory
} // namespace OHOS
