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

#ifndef OHOS_MEMORY_MEMMGR_ACCOUNT_PRIORITY_INFO_H
#define OHOS_MEMORY_MEMMGR_ACCOUNT_PRIORITY_INFO_H

#include <string>
#include "os_account_info.h"

namespace OHOS {
namespace Memory {
class AccountPriorityInfo {
public:
    AccountPriorityInfo(int id, std::string name, AccountSA::OsAccountType type, bool isActived);
    int GetId();
    void SetId(int id);
    std::string GetName();
    void SetName(std::string name);
    AccountSA::OsAccountType GetType();
    void SetType(AccountSA::OsAccountType type);
    bool GetIsActived();
    void SetIsActived(bool isActived);
    int GetPriority();
    void SetPriority(int priority);

private:
    int id_;
    std::string name_;
    AccountSA::OsAccountType type_;
    bool isActived_;
    int priority_;
};
} // namespace Memory
} // namespace OHOS

#endif // OHOS_MEMORY_MEMMGR_ACCOUNT_PRIORITY_H