/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "mem_mgr_window_info.h"
#include "memmgr_log.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrWindowInfo";
}

bool MemMgrWindowInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteUint32(windowId_) && parcel.WriteInt32(pid_) &&
           parcel.WriteInt32(uid_) && parcel.WriteBool(isVisible_);
}

MemMgrWindowInfo* MemMgrWindowInfo::Unmarshalling(Parcel &parcel)
{
    auto memMgrWindowInfo = new (std::nothrow) MemMgrWindowInfo();
    if (memMgrWindowInfo == nullptr) {
        HILOGE("window visibility info is nullptr.");
        return nullptr;
    }
    bool res = parcel.ReadUint32(memMgrWindowInfo->windowId_) && parcel.ReadInt32(memMgrWindowInfo->pid_) &&
        parcel.ReadInt32(memMgrWindowInfo->uid_) && parcel.ReadBool(memMgrWindowInfo->isVisible_);
    if (!res) {
        delete memMgrWindowInfo;
        return nullptr;
    }
    return memMgrWindowInfo;
}
}
}

