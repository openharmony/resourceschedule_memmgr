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

#ifndef OHOS_MEMORY_MEMMGR_INNERKITS_MEM_MGR_PROCESS_STATE_INFO_H
#define OHOS_MEMORY_MEMMGR_INNERKITS_MEM_MGR_PROCESS_STATE_INFO_H

#include <parcel.h>
namespace OHOS {
namespace Memory {
enum class ProcPriorityUpdateReason : uint32_t {
    START_ABILITY = 0,
    UNKNOWN,
};

/**
 * @class MemMgrProcessStateInfo
 *
 * @brief State info of process.
 */
class MemMgrProcessStateInfo : public Parcelable {
public:
    /**
     * @brief Default construct of MemMgrProcessStateInfo.
     */
    MemMgrProcessStateInfo() = default;
    /**
     * @brief Construct of MemMgrProcessStateInfo.
     *
     * @param callerPid Process id of caller process.
     * @param callerUid User id of caller process.
     * @param pid Process id.
     * @param uid User id.
     * @param reason Reason of update process priority.
     */
    MemMgrProcessStateInfo(int32_t callerPid, int32_t callerUid, int32_t pid, int32_t uid,
        ProcPriorityUpdateReason reason)
        : callerPid_(callerPid), callerUid_(callerUid), pid_(pid), uid_(uid), reason_(reason) {};
    /**
     * @brief Deconstruct of MemMgrProcessStateInfo.
     */
    ~MemMgrProcessStateInfo() = default;

    /**
     * @brief Marshalling MemMgrProcessStateInfo.
     *
     * @param parcel Package of MemMgrProcessStateInfo.
     * @return True means marshall success, false means marshall failed.
     */
    virtual bool Marshalling(Parcel &parcel) const override;
    /**
     * @brief Unmarshalling MemMgrProcessStateInfo.
     *
     * @param parcel Package of MemMgrProcessStateInfo.
     * @return MemMgrProcessStateInfo object.
     */
    static MemMgrProcessStateInfo* Unmarshalling(Parcel &parcel);

    int32_t callerPid_ {0};
    int32_t callerUid_ {0};
    int32_t pid_ {0};
    int32_t uid_ {0};
    ProcPriorityUpdateReason reason_ = ProcPriorityUpdateReason::UNKNOWN;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INNERKITS_MEM_MGR_PROCESS_STATE_INFO_H