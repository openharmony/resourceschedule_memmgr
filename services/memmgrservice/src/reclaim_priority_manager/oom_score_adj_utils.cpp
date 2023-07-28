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

#include <string>
#include <sstream>
#include "memmgr_log.h"
#include "kernel_interface.h"
#include "oom_score_adj_utils.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "ReclaimPriorityManager";
}

bool OomScoreAdjUtils::WriteOomScoreAdjToKernel(std::shared_ptr<BundlePriorityInfo> bundle)
{
    HILOGD("called");
    if (bundle == nullptr) {
        return false;
    }
    for (auto i = bundle->procs_.begin(); i != bundle->procs_.end(); ++i) {
        int priority = i->second.priority_;
        pid_t pid = i->second.pid_;
        std::stringstream ss;
        ss << "/proc/" << pid << "/oom_score_adj";
        std::string path = ss.str();
        std::string content = std::to_string(priority);
        HILOGD("prepare to echo %{public}s > %{public}s", content.c_str(), path.c_str());
        KernelInterface::GetInstance().EchoToPath(path.c_str(), content.c_str());
    }
    return true;
}

bool OomScoreAdjUtils::WriteOomScoreAdjToKernel(pid_t pid, int priority)
{
    HILOGD("called");
    std::stringstream ss;
    ss << "/proc/" << pid << "/oom_score_adj";
    std::string path = ss.str();
    std::string content = std::to_string(priority);
    HILOGD("prepare to echo %{public}s > %{public}s", content.c_str(), path.c_str());
    KernelInterface::GetInstance().EchoToPath(path.c_str(), content.c_str());
    return true;
}
} // namespace Memory
} // namespace OHOS
