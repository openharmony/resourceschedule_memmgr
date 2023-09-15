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

#ifndef OHOS_MEMORY_MEMMGR_PROCESS_PRIORITY_INFO_H
#define OHOS_MEMORY_MEMMGR_PROCESS_PRIORITY_INFO_H

#include <set>
#include <string>
#include <map>

namespace OHOS {
namespace Memory {
constexpr int EXTENSION_STATUS_BIND_UNKOWN = 0;
constexpr int EXTENSION_STATUS_FG_BIND = 1;
constexpr int EXTENSION_STATUS_BG_BIND = 2;
constexpr int EXTENSION_STATUS_NO_BIND = 3;
const std::string DEFAULT_PROCESS_NAME = "";

class ProcessPriorityInfo {
public:
    explicit ProcessPriorityInfo(pid_t pid, int bundleUid, int priority, const std::string &name = DEFAULT_PROCESS_NAME);
    ProcessPriorityInfo(const ProcessPriorityInfo &copyProcess);
    ~ProcessPriorityInfo();

    int uid_;
    pid_t pid_;
    std::string name_;
    int priority_;
    bool isVisible_;
    bool isRender_;
    bool isFreground; // true means freground, false means background
    bool isBackgroundRunning;
    bool isSuspendDelay;
    bool isEventStart;
    bool isDistDeviceConnected;
    bool isExtension_;
    int extensionBindStatus; // 0: unkown, 1:fg bind, 2:bg bind, 3:no bind
    std::map<int32_t, int32_t> procsBindToMe_;
    std::map<int32_t, int32_t> procsBindFromMe_;

    void SetPriority(int targetPriority);
    int32_t ExtensionConnectorsCount();

    void ProcBindToMe(int32_t pid, int32_t uid);
    void ProcUnBindToMe(int32_t pid);
    
    void ProcBindFromMe(int32_t pid, int32_t uid);
    void ProcUnBindFromMe(int32_t pid);

    std::string ProcsBindToMe();
    std::string ProcsBindFromMe();
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_PROCESS_PRIORITY_INFO_H
