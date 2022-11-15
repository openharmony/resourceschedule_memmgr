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

class DefaultMultiAccountStrategyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DefaultMultiAccountStrategyTest::SetUpTestCase()
{
}

void DefaultMultiAccountStrategyTest::TearDownTestCase()
{
}

void DefaultMultiAccountStrategyTest::SetUp()
{
}

void DefaultMultiAccountStrategyTest::TearDown()
{
}

/**
 * @tc.name: SetAccountPriority
 * @tc.desc: Test the value of accountInfo->GetIsActived() equals to true
 * @tc.desc: Test the value of accountInfo->priority_ equals to HIGH_PRIORITY
 * @tc.type: FUNC
 */
HWTEST_F(DefaultMultiAccountStrategyTest, SetAccountProrityTest, TestSize.Level1)
{
    std::shared_ptr<AccountPriorityInfo> accountInfo;
    bool isActived = true;
    int accountId = 0;
    std::string accountName = "admin";
    AccountSA::OsAccountType accountType = AccountSA::OsAccountType::ADMIN;
    DefaultMultiAccountStrategy mulAcc;

    // the branch of GetIsActived() equals to true
    // the branch of priority_ equals to HIGH_PRIORITY
    AccountPriorityInfo accPri1(accountId, accountName, accountType, isActived);
    MultiAccountManager::GetInstance().SetAccountPriority(accountId, accountName, accountType, isActived);
    accountInfo = MultiAccountManager::GetInstance().GetAccountPriorityInfo(accountId);
    mulAcc.SetAccountPriority(accountInfo);
    EXPECT_EQ(accountInfo->priority_, static_cast<int>(DefaultMultiAccountPriority::HIGH_PRIORITY));

    // the branch of priority_ equals to LOW_PRIORITY
    isActived = false;
    AccountPriorityInfo accPri2(accountId, accountName, accountType, isActived);
    MultiAccountManager::GetInstance().SetAccountPriority(accountId, accountName, accountType, isActived);
    accountInfo = MultiAccountManager::GetInstance().GetAccountPriorityInfo(accountId);
    mulAcc.SetAccountPriority(accountInfo);
    EXPECT_EQ(accountInfo->priority_, static_cast<int>(DefaultMultiAccountPriority::LOW_PRIORITY));

    // the branch of GetIsActived() equals to false
    accountInfo = nullptr;
    bool setAcc = mulAcc.SetAccountPriority(accountInfo);
    EXPECT_EQ(setAcc, false);
}

/**
 * @tc.name: RecalcBundlePriority
 * @tc.desc: Test the return value that not equals to RECLAIM_PRIORITY_VISIBLE
 * @tc.desc: Test the value of accountInfo equals to nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DefaultMultiAccountStrategyTest, RecalcBundlePriortiyTest, TestSize.Level1)
{
    std::shared_ptr<AccountPriorityInfo> accountInfo;
    bool isActived = true;
    int accountId = 0;
    std::string accountName = "admin";
    AccountSA::OsAccountType accountType = AccountSA::OsAccountType::ADMIN;
    DefaultMultiAccountStrategy mulAcc;
    int bundlePriority = RECLAIM_PRIORITY_VISIBLE;

    // return value not equals to RECLAIM_PRIORITY_VISIBLE
    MultiAccountManager::GetInstance().SetAccountPriority(accountId, accountName, accountType, isActived);
    accountInfo = MultiAccountManager::GetInstance().GetAccountPriorityInfo(accountId);
    mulAcc.SetAccountPriority(accountInfo);
    int recalcPriority = mulAcc.RecalcBundlePriority(accountInfo, bundlePriority);
    EXPECT_NE(recalcPriority, RECLAIM_PRIORITY_VISIBLE);

    // the branch of accountInfo equals to nullptr
    accountInfo = nullptr;
    mulAcc.SetAccountPriority(accountInfo);
    recalcPriority = mulAcc.RecalcBundlePriority(accountInfo, bundlePriority);
    EXPECT_EQ(recalcPriority, RECLAIM_PRIORITY_MAX);
}

}
}
