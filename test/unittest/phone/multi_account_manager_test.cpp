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
#include "default_multi_account_strategy.h"
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

HWTEST_F(MultiAccountManagerTest, InitTest, TestSize.Level1)
{
    MultiAccountManager::GetInstance().Init();
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
    EXPECT_EQ(info->GetPriority(), static_cast<int>(DefaultMultiAccountPriority::HIGH_PRIORITY));
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
    std::shared_ptr<BundlePriorityInfo> bundle = std::make_shared<BundlePriorityInfo>("app",
            accountId * USER_ID_SHIFT + 1, 100);
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
    std::shared_ptr<BundlePriorityInfo> bundle = std::make_shared<BundlePriorityInfo>("app",
            accountId * USER_ID_SHIFT + 1, 100);
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

/**
 * @tc.name: AddAccountPriorityInfo
 * @tc.desc: Test add value include id_  name_  type_  isActived_
 * @tc.type: FUNC
 */
HWTEST_F(MultiAccountManagerTest, AddAccountPriorityInfoTest, TestSize.Level1)
{
    int accountId = 3;
    std::string accountName = "admin";
    AccountSA::OsAccountType accountType = AccountSA::OsAccountType::ADMIN;
    bool isActived = true;

    MultiAccountManager::GetInstance().SetAccountPriority(accountId, accountName, accountType, isActived);
    std::shared_ptr<AccountPriorityInfo> accountInfo =
        MultiAccountManager::GetInstance().GetAccountPriorityInfo(accountId);
    MultiAccountManager::GetInstance().AddAccountPriorityInfo(accountInfo);
    EXPECT_EQ(accountInfo->GetId(), accountId);
    EXPECT_STREQ(accountInfo->GetName().c_str(), accountName.c_str());
    EXPECT_EQ(accountInfo->GetType(), accountType);
    EXPECT_EQ(accountInfo->GetIsActived(), isActived);
}

/**
 * @tc.name: SetMultiAccountStrategy and GetMultiAccountStratgy
 * @tc.desc: Test set value of strategy_ equals to nullptr
 * @tc.desc: Test get value of strategy_
 * @tc.type: FUNC
 */
HWTEST_F(MultiAccountManagerTest, SetMultiAccountStrategyTest, TestSize.Level1)
{
    // strategy_ equals to nullptr
    std::shared_ptr<MultiAccountStrategy> strategy = nullptr;
    bool retMul = MultiAccountManager::GetInstance().SetMultiAccountStrategy(strategy);
    EXPECT_EQ(retMul, false);

    // set and get value of strategy_
    strategy = MultiAccountManager::GetInstance().GetMultiAccountStratgy();
    retMul = MultiAccountManager::GetInstance().SetMultiAccountStrategy(strategy);
    EXPECT_EQ(retMul, true);
}

/**
 * @tc.name: GetSwitchedAccountIds
 * @tc.desc: test GetSwitchedAccountIds into for and if branch
 * @tc.type: FUNC
 */
HWTEST_F(MultiAccountManagerTest, GetSwitchedAccountIdsTest, TestSize.Level1)
{
    MultiAccountManager oldAct;
    oldAct.oldActiveAccountIds_ = {1, 3, 5, 7, 9};
    std::vector<int> accountIds;
    std::vector<int> accountIds1 = {1, 3, 5, 7, 9};
    oldAct.GetSwitchedAccountIds(accountIds);
    EXPECT_EQ(accountIds, accountIds1);
}

/**
 * @tc.name: UpdateAccountPriorityInfo
 * @tc.desc: test UpdateAccountPriorityInfoTest into for and if branch
 * @tc.type: FUNC
 */
HWTEST_F(MultiAccountManagerTest, UpdateAccountPriorityInfoTest, TestSize.Level1)
{
    std::vector<int> accountIds;
    EXPECT_EQ(MultiAccountManager::GetInstance().UpdateAccountPriorityInfo(accountIds), true);
    std::vector<int> accountIds1 = {1, 3, 5, 7, 9};
    EXPECT_EQ(MultiAccountManager::GetInstance().UpdateAccountPriorityInfo(accountIds1), false);
}

/**
 * @tc.name: HandleOsAccountsChanged
 * @tc.desc: test initialized_ == false
 * @tc.desc: test the branch of UpdateAccountPriorityInfo(updatedAccountIds) equals to false
 * @tc.desc: test the branch of switchMod equals to AccountSA::COLD_SWITCH
 * @tc.desc: test the branch of switchMod equals to AccountSA::HOT_SWITCH
 * @tc.type: FUNC
 */
HWTEST_F(MultiAccountManagerTest, HandleOsAccountsChangedTest, TestSize.Level1)
{
    MultiAccountManager mulAcc;
    mulAcc.initialized_ = false;
    int accountId = 100;
    AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod1 = AccountSA::OsAccountManager::GetOsAccountSwitchMod();
    AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod2 = AccountSA::COLD_SWITCH;
    AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod3 = AccountSA::HOT_SWITCH;
    std::map<int, std::shared_ptr<AccountBundleInfo>> osAccountsInfoMap;

    // the branch of test initialized_ equals to false
    bool ret = mulAcc.HandleOsAccountsChanged(accountId, switchMod1, osAccountsInfoMap);
    EXPECT_EQ(ret, false);

    // the branch of UpdateAccountPriorityInfo(updatedAccountIds) equals to false
    ret = mulAcc.HandleOsAccountsChanged(accountId, switchMod1, osAccountsInfoMap);
    EXPECT_EQ(ret, false);

    // the branch of switchMod equals to AccountSA::COLD_SWITCH
    ret = MultiAccountManager::GetInstance().HandleOsAccountsChanged(accountId, switchMod2, osAccountsInfoMap);
    EXPECT_EQ(ret, true);

    // the branch of switchMod equals to AccountSA::HOT_SWITCH
    ret = MultiAccountManager::GetInstance().HandleOsAccountsChanged(accountId, switchMod3, osAccountsInfoMap);
    EXPECT_EQ(ret, true);
}

}
}
