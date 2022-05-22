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

#include "process_priority_info.h"
#include "memmgr_log.h"
#include "reclaim_priority_constants.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "ProcessPriorityInfo";
} // namespace

ProcessPriorityInfo::ProcessPriorityInfo(pid_t pid, int bundleUid, int priority)
{
    this->uid_ = bundleUid;
    this->pid_ = pid;
    this->priority_ = priority;
    this->isFreground = (priority == RECLAIM_PRIORITY_FOREGROUND) ? true : false;
    this->isBackgroundRunning = false;
    this->isSuspendDelay = false;
    this->isEventStart = false;
    this->isDataAbilityStart = false;
    this->isDistDeviceConnected = false;
    this->extensionBindStatus = EXTENSION_STATUS_BIND_UNKOWN;
}

ProcessPriorityInfo::ProcessPriorityInfo(const ProcessPriorityInfo &copyProcess)
{
    this->uid_ = copyProcess.uid_;
    this->pid_ = copyProcess.pid_;
    this->priority_ = copyProcess.priority_;
    this->isFreground = copyProcess.isFreground;
    this->isBackgroundRunning = copyProcess.isBackgroundRunning;
    this->isSuspendDelay = copyProcess.isSuspendDelay;
    this->isEventStart = copyProcess.isEventStart;
    this->isDataAbilityStart = copyProcess.isDataAbilityStart;
    this->isDistDeviceConnected = copyProcess.isDistDeviceConnected;
    this->extensionBindStatus = copyProcess.extensionBindStatus;
}

void ProcessPriorityInfo::SetPriority(int targetPriority)
{
    priority_ = targetPriority;
    HILOGD("set process[%{public}d] priority to %{public}d", pid_, priority_);
}
} // namespace Memory
} // namespace OHOS
