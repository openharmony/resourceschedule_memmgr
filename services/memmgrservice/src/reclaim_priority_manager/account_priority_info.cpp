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

#include "account_priority_info.h"

namespace OHOS {
namespace Memory {
AccountPriorityInfo::AccountPriorityInfo(int id, std::string name, AccountType type, bool isActived)
    : id_(id), name_(name), type_(type), isActived_(isActived)
{
}

int AccountPriorityInfo::GetId()
{
    return id_;
}

void AccountPriorityInfo::SetId(int id)
{
    id_ = id;
}

std::string AccountPriorityInfo::GetName()
{
    return name_;
}

void AccountPriorityInfo::SetName(std::string name)
{
    name_ = name;
}

AccountType AccountPriorityInfo::GetType()
{
    return type_;
}

void AccountPriorityInfo::SetType(AccountType type)
{
    type_ = type;
}

bool AccountPriorityInfo::GetIsActived()
{
    return isActived_;
}

void AccountPriorityInfo::SetIsActived(bool isActived)
{
    isActived_ = isActived;
}

int AccountPriorityInfo::GetPriority()
{
    return priority_;
}

void AccountPriorityInfo::SetPriority(int priority)
{
    priority_ = priority;
}
} // namespace Memory
} // namespace OHOS