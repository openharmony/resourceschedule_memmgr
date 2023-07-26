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

ProcessPriorityInfo::ProcessPriorityInfo(pid_t pid, int bundleUid, int priority)
{
    this->uid_ = bundleUid;
    this->pid_ = pid;
    this->priority_ = priority;
    this->isVisible_ = false;
    this->isFreground = (priority == RECLAIM_PRIORITY_FOREGROUND) ? true : false;
    this->isBackgroundRunning = false;
    this->isSuspendDelay = false;
    this->isEventStart = false;
    this->isDistDeviceConnected = false;
    this->isExtension = false;
    this->extensionBindStatus = EXTENSION_STATUS_BIND_UNKOWN;
}

ProcessPriorityInfo::ProcessPriorityInfo(const ProcessPriorityInfo &copyProcess)
{
    this->uid_ = copyProcess.uid_;
    this->pid_ = copyProcess.pid_;
    this->priority_ = copyProcess.priority_;
    this->isVisible_ = copyProcess.isVisible_;
    this->isFreground = copyProcess.isFreground;
    this->isExtension = copyProcess.isExtension;
    this->isBackgroundRunning = copyProcess.isBackgroundRunning;
    this->isSuspendDelay = copyProcess.isSuspendDelay;
    this->isEventStart = copyProcess.isEventStart;
    this->isDistDeviceConnected = copyProcess.isDistDeviceConnected;
    this->extensionBindStatus = copyProcess.extensionBindStatus;
    for (auto connectors : copyProcess.extensionConnectors_) {
        this->AddExtensionConnector(connectors);
    }
    for (auto callerUid : copyProcess.extensionProcessUids_) {
        this->AddExtensionProcessUid(callerUid);
    }
    for (auto connectorUid : copyProcess.extensionConnectorUids_) {
        this->AddExtensionConnectorUid(connectorUid);
    }
}

ProcessPriorityInfo::~ProcessPriorityInfo()
{
    extensionConnectors_.clear();
    extensionProcessUids_.clear();
    extensionConnectorUids_.clear();
}

void ProcessPriorityInfo::SetPriority(int targetPriority)
{
    priority_ = targetPriority;
    HILOGD("set process[%{public}d] priority to %{public}d", pid_, priority_);
}

int32_t ProcessPriorityInfo::ExtensionConnectorsCount()
{
    return extensionConnectors_.size();
}

void ProcessPriorityInfo::AddExtensionConnector(int32_t pid)
{
    extensionConnectors_.insert(pid);
}

void ProcessPriorityInfo::RemoveExtensionConnector(int32_t pid)
{
    extensionConnectors_.erase(pid);
}

void ProcessPriorityInfo::AddExtensionProcessUid(int32_t uid)
{
    extensionProcessUids_.insert(uid);
}

void ProcessPriorityInfo::RemoveExtensionProcessUid(int32_t uid)
{
    extensionProcessUids_.erase(uid);
}

void ProcessPriorityInfo::AddExtensionConnectorUid(int32_t uid)
{
    extensionConnectorUids_.insert(uid);
}

void ProcessPriorityInfo::RemoveExtensionConnectorUid(int32_t uid)
{
    extensionConnectorUids_.erase(uid);
}

bool ProcessPriorityInfo::ContainsConnector(int32_t pid)
{
    return extensionConnectors_.count(pid) != 0;
}

std::string ProcessPriorityInfo::ConnectorsToString()
{
    std::stringstream ss;
    ss << "[";
    for (auto connector : extensionConnectors_) {
        ss << connector << " ";
    }
    ss << "]";
    return ss.str();
}

std::string ProcessPriorityInfo::ExtensionProcessUidToString()
{
    std::stringstream ss;
    ss << "[";
    for (auto callerUid : extensionProcessUids_) {
        ss << callerUid << " ";
    }
    ss << "]";
    return ss.str();
}

std::string ProcessPriorityInfo::ExtensionConnectorsUidToString()
{
    std::stringstream ss;
    ss << "[";
    for (auto connectorUid : extensionConnectorUids_) {
        ss << connectorUid << " ";
    }
    ss << "]";
    return ss.str();
}
} // namespace Memory
} // namespace OHOS
