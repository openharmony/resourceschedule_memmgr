/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "mem_mgr_process_state_info.h"
#include "memmgr_log.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrProcessStateInfo";
}

bool MemMgrProcessStateInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteInt32(callerPid_) && parcel.WriteInt32(callerUid_) &&
           parcel.WriteInt32(pid_) && parcel.WriteInt32(uid_) &&
           parcel.WriteUint32(static_cast<uint32_t>(reason_));
}

MemMgrProcessStateInfo* MemMgrProcessStateInfo::Unmarshalling(Parcel &parcel)
{
    auto memMgrProcessStateInfo = new (std::nothrow) MemMgrProcessStateInfo();
    if (memMgrProcessStateInfo == nullptr) {
        HILOGE("process state info is nullptr.");
        return nullptr;
    }
    uint32_t reason = static_cast<uint32_t>(ProcPriorityUpdateReason::UNKNOWN);
    bool res = parcel.ReadInt32(memMgrProcessStateInfo->callerPid_) &&
        parcel.ReadInt32(memMgrProcessStateInfo->callerUid_) &&
        parcel.ReadInt32(memMgrProcessStateInfo->pid_) &&
        parcel.ReadInt32(memMgrProcessStateInfo->uid_) &&
        parcel.ReadUint32(reason);
    if (!res) {
        delete memMgrProcessStateInfo;
        return nullptr;
    }
    switch (reason) {
        case static_cast<uint32_t>(ProcPriorityUpdateReason::START_ABILITY):
            memMgrProcessStateInfo->reason_ = static_cast<ProcPriorityUpdateReason>(reason);
            break;
        default:
            break;
    }
    return memMgrProcessStateInfo;
}
}
}

