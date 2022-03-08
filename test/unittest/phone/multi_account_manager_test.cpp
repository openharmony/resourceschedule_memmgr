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

#include "gtest/gtest.h"

#include "utils.h"

#define private public
#define protected public
#include "reclaim_priority_constants.h"
#include "multi_account_manager.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class MultiAccountManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MultiAccountManagerTest::SetUpTestCase()
{
}

void MultiAccountManagerTest::TearDownTestCase()
{
}

void MultiAccountManagerTest::SetUp()
{
}

void MultiAccountManagerTest::TearDown()
{
}

HWTEST_F(MultiAccountManagerTest, SetAccountPrority, TestSize.Level1)
{
    int accountId = 2;
    std::string accountName = "admin";
    AccountSA::OsAccountType accountType = AccountSA::OsAccountType::ADMIN;
    bool isActived = true;

    MultiAccountManager::GetInstance().SetAccountPriority(accountId, accountName, accountType, isActived);
    std::shared_ptr<AccountPriorityInfo> info = MultiAccountManager::GetInstance().GetAccountPriorityInfo(accountId);

    EXPECT_EQ(info->GetId(), accountId);
    EXPECT_STREQ(info->GetName().c_str(), accountName.c_str());
    EXPECT_EQ(info->GetType(), accountType);
    EXPECT_EQ(info->GetIsActived(), isActived);
    EXPECT_EQ(info->GetPriority(), 0);
}

HWTEST_F(MultiAccountManagerTest, RecalcBundlePriortiy, TestSize.Level1)
{
    int accountId = 2;
    std::string accountName = "admin";
    AccountSA::OsAccountType accountType = AccountSA::OsAccountType::ADMIN;
    bool isActived = false;
    int bundlePriority = RECLAIM_PRIORITY_FOREGROUND;

    MultiAccountManager::GetInstance().SetAccountPriority(accountId, accountName, accountType, isActived);
    int recalcPriority = MultiAccountManager::GetInstance().RecalcBundlePriority(accountId, bundlePriority);

    EXPECT_EQ(recalcPriority, RECLAIM_PRIORITY_FOREGROUND + 50);
}

HWTEST_F(MultiAccountManagerTest, AccountColdSwitch, TestSize.Level1)
{
    int accountId = 100;
    std::shared_ptr<AccountBundleInfo> account = std::make_shared<AccountBundleInfo>(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = std::make_shared<BundlePriorityInfo>("app", accountId * USER_ID_SHIFT + 1, 100);
    ProcessPriorityInfo proc1(1001, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc2(1002, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc3(1003, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc4(1004, bundle->uid_, bundle->priority_);

    std::map<int, std::shared_ptr<AccountBundleInfo>> osAccountsInfoMap;
    bundle->AddProc(proc1);
    bundle->AddProc(proc2);
    bundle->AddProc(proc3);
    bundle->AddProc(proc4);
    account->AddBundleToOsAccount(bundle);
    osAccountsInfoMap.insert(std::make_pair(account->id_, account));

    std::vector<int> switchedIds { accountId };
    MultiAccountManager::GetInstance().HandleAccountColdSwitch(switchedIds, osAccountsInfoMap);
}

HWTEST_F(MultiAccountManagerTest, AccountHotSwitch, TestSize.Level1)
{
    int accountId = 100;
    std::shared_ptr<AccountBundleInfo> account = std::make_shared<AccountBundleInfo>(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = std::make_shared<BundlePriorityInfo>("app", accountId * USER_ID_SHIFT + 1, 100);
    ProcessPriorityInfo proc1(1001, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc2(1002, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc3(1003, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc4(1004, bundle->uid_, bundle->priority_);

    std::map<int, std::shared_ptr<AccountBundleInfo>> osAccountsInfoMap;
    bundle->AddProc(proc1);
    bundle->AddProc(proc2);
    bundle->AddProc(proc3);
    bundle->AddProc(proc4);
    account->AddBundleToOsAccount(bundle);
    osAccountsInfoMap.insert(std::make_pair(account->id_, account));

    std::string accountName = "admin";
    AccountSA::OsAccountType accountType = AccountSA::OsAccountType::ADMIN;
    bool isActived = false;
    MultiAccountManager::GetInstance().SetAccountPriority(accountId, accountName, accountType, isActived);

    std::vector<int> switchedIds { accountId };
    MultiAccountManager::GetInstance().HandleAccountHotSwitch(switchedIds, osAccountsInfoMap);

    EXPECT_EQ(bundle->priority_, 150);
}
}
}
