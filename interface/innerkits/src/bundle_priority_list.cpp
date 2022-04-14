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

#include "bundle_priority_list.h"
#include "memmgr_log.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrClient";
}

int32_t BundlePriorityList::GetCount() const
{
    return count_;
}

void BundlePriorityList::SetCount(int32_t count)
{
    count_ = count;
}

const std::vector<BundlePriority>& BundlePriorityList::GetList()
{
    return list_;
}

int32_t BundlePriorityList::Size() const
{
    return list_.size();
}

void BundlePriorityList::AddBundleInfo(BundlePriority &bundleInfo)
{
    list_.push_back(bundleInfo);
}

void BundlePriorityList::Show() const
{
    HILOGI("     uid                                            name   priority   accountId");
    for (auto bi : list_) {
        HILOGI("%{public}8d\t%{public}42s\t%{public}5d\t%{public}3d",
            bi.uid_, bi.name_.c_str(), bi.priority_, bi.accountId_);
    }
    HILOGI("-------------------------------------------------------------------------------");
}

bool BundlePriorityList::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(count_)) {
        HILOGE("Failed to write count_");
        return false;
    }
    for (auto i = 0; i < count_; ++i) {
        if (!parcel.WriteInt32(list_[i].uid_)) {
            HILOGE("Failed to write uid_");
            return false;
        }
        if (!parcel.WriteString(list_[i].name_)) {
            HILOGE("Failed to write ability name");
            return false;
        }
        if (!parcel.WriteInt32(list_[i].priority_)) {
            HILOGE("Failed to write priority_");
            return false;
        }
        if (!parcel.WriteInt32(list_[i].accountId_)) {
            HILOGE("Failed to write accountId_");
            return false;
        }
    }
    return true;
}

BundlePriorityList* BundlePriorityList::Unmarshalling(Parcel &parcel)
{
    auto object = new (std::nothrow) BundlePriorityList();
    if ((object != nullptr) && !object->ReadFromParcel(parcel)) {
        delete object;
        object = nullptr;
    }

    return object;
}

bool BundlePriorityList::ReadFromParcel(Parcel &parcel)
{
    count_ = parcel.ReadInt32();
    for (auto i = 0; i < count_; ++i) {
        int32_t uid = parcel.ReadInt32();
        std::string name;
        if (!parcel.ReadString(name)) {
            HILOGE("Failed to read creator ability name");
            return false;
        }
        int32_t priority = parcel.ReadInt32();
        int32_t accountId = parcel.ReadInt32();
        list_.push_back(BundlePriority(uid, name, priority, accountId));
    }
    return true;
}
}
}

