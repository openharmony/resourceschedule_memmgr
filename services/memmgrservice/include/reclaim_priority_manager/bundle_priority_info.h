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

#ifndef OHOS_MEMORY_MEMMGR_BUNDLE_PRIORITY_INFO_H
#define OHOS_MEMORY_MEMMGR_BUNDLE_PRIORITY_INFO_H

#include <string>
#include <list>
#include <map>
#include <mutex>

#include "reclaim_priority_constants.h"
#include "process_priority_info.h"

namespace OHOS {
namespace Memory {
class BundlePriorityInfo {
public:
    using ProcessesInfoMap = std::map<pid_t, ProcessPriorityInfo>;
    explicit BundlePriorityInfo(std::string name, int bundleUid);
    explicit BundlePriorityInfo(std::string name, int bundleUid, int priority);
    explicit BundlePriorityInfo(std::string name, int bundleUid, int priority, int accountId, BundleState state);
    BundlePriorityInfo(const BundlePriorityInfo &copyBundle);
    inline bool operator<(const BundlePriorityInfo &tmp) const
    {
        return priority_ < tmp.priority_;
    }
    std::string name_;
    int uid_;
    ProcessesInfoMap procs_;
    int priority_;
    int accountId_;
    BundleState state_;
    std::mutex bundleLock_;

    bool HasProc(pid_t pid);
    void AddProc(ProcessPriorityInfo &newProcess);
    void RemoveProcByPid(pid_t pid);
    ProcessPriorityInfo& FindProcByPid(pid_t pid);
    int GetProcsCount();
    int GetMinProcPriority();
    void SetPriority(int targetPriority);
    void UpdatePriority();
    BundleState GetState();
    void SetState(BundleState state);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_BUNDLE_PRIORITY_INFO_H
