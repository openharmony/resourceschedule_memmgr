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

#include "memmgr_log.h"
#include "kernel_interface.h"
#include "multi_account_kill.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MultiAccountKill";
}

IMPLEMENT_SINGLE_INSTANCE(MultiAccountKill);

MultiAccountKill::MultiAccountKill()
{
}

MultiAccountKill::~MultiAccountKill()
{
}

void MultiAccountKill::KillAccountProccesses(std::vector<pid_t> &pidList)
{
    for (unsigned int i = 0; i < pidList.size(); i++) {
        pid_t pid = pidList.at(i);
        int ret = KernelInterface::GetInstance().KillOneProcessByPid(pid);
        if (!ret) {
            HILOGI("Kill account one process fail, pid = %{public}d.", pid);
        } else {
            HILOGI("Kill account one process success, pid = %{public}d.", pid);
        }
    }
}
} // namespace Memory
} // namespace OHOS