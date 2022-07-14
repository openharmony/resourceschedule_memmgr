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

#ifndef OHOS_MEMORY_MEMMGR_INNERKITS_BUNDLE_PRIORITY_LIST_H
#define OHOS_MEMORY_MEMMGR_INNERKITS_BUNDLE_PRIORITY_LIST_H

#include "bundle_priority.h"
#include "parcel.h"

namespace OHOS {
namespace Memory {
class BundlePriorityList : public Parcelable {
public:
    /**
     * @brief Marshals a purpose into a parcel.
     *
     * @param parcel Indicates the parcel object for marshalling.
     * @return True if success, else false.
     */
    bool Marshalling(Parcel &parcel) const;
    static BundlePriorityList *Unmarshalling(Parcel &parcel);

    int32_t GetCount() const;
    void SetCount(int32_t count);
    const std::vector<BundlePriority>& GetList();
    int32_t Size() const;
    void AddBundleInfo(BundlePriority &bundleInfo);
    void Show() const;
private:
    bool ReadFromParcel(Parcel &parcel);

    int32_t count_ {0};
    std::vector<BundlePriority> list_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INNERKITS_BUNDLE_PRIORITY_LIST_H
