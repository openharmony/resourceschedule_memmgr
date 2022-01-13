/*
 * Copyright (c) 2021 XXXX.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this->file except in compliance with the License.
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

#include "os_account_priority_info.h"
#include "memmgr_log.h"


namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "OsAccountPriorityInfo";
const int OS_ACCOUNT_PRIORITY_SHIFT_DEFAULT = 50;
} // namespace

OsAccountPriorityInfo::OsAccountPriorityInfo(int accountId, bool isCurrent):id_(accountId), isCurOsAccount_(isCurrent)
{
    isPreOsAccount_ = false;
    priorityShift_ = OS_ACCOUNT_PRIORITY_SHIFT_DEFAULT;
}

bool OsAccountPriorityInfo::BundleExist(int bundleUId)
{
    if (bundleIdInfoMapping_.count(bundleUId) == 0) {
        return false;
    }
    return true;
}

BundlePriorityInfo* OsAccountPriorityInfo::FindBundleInfoById(int bundleUId)
{
    return bundleIdInfoMapping_.at(bundleUId);
}

void OsAccountPriorityInfo::AddBundleToUser(BundlePriorityInfo* bundle)
{
    bundleIdInfoMapping_.insert(std::make_pair(bundle->uid_, bundle));
}

void OsAccountPriorityInfo::RemoveBundleById(int bundleUid)
{
    bundleIdInfoMapping_.erase(bundleUid);
}

int OsAccountPriorityInfo::BundleCount()
{
    return bundleIdInfoMapping_.size();
}

void OsAccountPriorityInfo::PromoteAllBundlePriority(int shift)
{
    AdjustAllBundlePriority(-shift);
}

void OsAccountPriorityInfo::ReduceAllBundlePriority(int shift)
{
    AdjustAllBundlePriority(shift);
}

void OsAccountPriorityInfo::AdjustAllBundlePriority(int shift)
{
    for (auto i = bundleIdInfoMapping_.begin(); i != bundleIdInfoMapping_.end(); ++i) {
        BundlePriorityInfo *bundleInfo = i->second;
        int targetPriority = bundleInfo->priority_ + shift;
        bundleInfo->SetPriority(targetPriority);
        
    }
}



} // namespace Memory
} // namespace OHOS
