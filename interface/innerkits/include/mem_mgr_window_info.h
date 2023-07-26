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

#ifndef OHOS_MEMORY_MEMMGR_INNERKITS_MEM_MGR_WINDOW_INFO_H
#define OHOS_MEMORY_MEMMGR_INNERKITS_MEM_MGR_WINDOW_INFO_H

#include <parcel.h>
namespace OHOS {
namespace Memory {
constexpr int INVALID_WINDOW_ID = 0;
/**
 * @class MemMgrWindowInfo
 *
 * @brief Visibility info of window.
 */
class MemMgrWindowInfo : public Parcelable {
public:
    /**
     * @brief Default construct of MemMgrWindowInfo.
     */
    MemMgrWindowInfo() = default;
    /**
     * @brief Construct of MemMgrWindowInfo.
     *
     * @param winId Window id.
     * @param pid Process id.
     * @param uid User id.
     * @param visibility True means window is visible, false means the opposite.
     */
    MemMgrWindowInfo(uint32_t winId, int32_t pid, int32_t uid, bool visibility)
        : windowId_(winId), pid_(pid), uid_(uid), isVisible_(visibility) {};
    /**
     * @brief Deconstruct of MemMgrWindowInfo.
     */
    ~MemMgrWindowInfo() = default;

    /**
     * @brief Marshalling MemMgrWindowInfo.
     *
     * @param parcel Package of MemMgrWindowInfo.
     * @return True means marshall success, false means marshall failed.
     */
    virtual bool Marshalling(Parcel &parcel) const override;
    /**
     * @brief Unmarshalling MemMgrWindowInfo.
     *
     * @param parcel Package of MemMgrWindowInfo.
     * @return MemMgrWindowInfo object.
     */
    static MemMgrWindowInfo* Unmarshalling(Parcel &parcel);

    uint32_t windowId_ {INVALID_WINDOW_ID};
    int32_t pid_ {0};
    int32_t uid_ {0};
    bool isVisible_ {false};
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INNERKITS_MEM_MGR_WINDOW_INFO_H