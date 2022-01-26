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
#include "reclaim_priority_manager.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class MemMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MemMgrTest::SetUpTestCase()
{
}

void MemMgrTest::TearDownTestCase()
{
}

void MemMgrTest::SetUp()
{
}

void MemMgrTest::TearDown()
{
}


HWTEST_F(MemMgrTest, ReclaimPriorityManagerInit, TestSize.Level1)
{
    bool ret = ReclaimPriorityManager::GetInstance().Init();
    EXPECT_EQ(ret, true);
}

HWTEST_F(MemMgrTest, ChangeOsAccountIdToValid, TestSize.Level1)
{
    ReclaimPriorityManager::GetInstance().CurrentOsAccountChanged(0);
    EXPECT_EQ(ReclaimPriorityManager::GetInstance().curOsAccountId_, 0);
}

HWTEST_F(MemMgrTest, ChangeOsAccountIdToInValid, TestSize.Level1)
{
    bool ret = ReclaimPriorityManager::GetInstance().CurrentOsAccountChanged(-1);
    EXPECT_EQ(ret, false);
}

HWTEST_F(MemMgrTest, AddOsAccountInfo, TestSize.Level1)
{
    int account_id = 0;
    OsAccountPriorityInfo account(account_id, true);
    ReclaimPriorityManager::GetInstance().AddOsAccountInfo(account);

    bool isAccountExist = ReclaimPriorityManager::GetInstance().IsOsAccountExist(account_id);
    EXPECT_EQ(isAccountExist, true);
}

HWTEST_F(MemMgrTest, RemoveOsAccountById, TestSize.Level1)
{
    int account_id = 0;
    OsAccountPriorityInfo account(account_id, true);
    ReclaimPriorityManager::GetInstance().AddOsAccountInfo(account);

    bool isAccountExist = ReclaimPriorityManager::GetInstance().IsOsAccountExist(account_id);
    EXPECT_EQ(isAccountExist, true);

    ReclaimPriorityManager::GetInstance().RemoveOsAccountById(account_id);
    isAccountExist = ReclaimPriorityManager::GetInstance().IsOsAccountExist(account_id);
    EXPECT_EQ(isAccountExist, false);
}

HWTEST_F(MemMgrTest, IsProcExist, TestSize.Level1)
{
    int pid = 1000;
    int uid = 100;
    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    EXPECT_EQ(account_id, 0);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);

    bool isProcExist = ReclaimPriorityManager::GetInstance().IsProcExist(pid, uid, account_id);
    EXPECT_EQ(isProcExist, true);

    pid = 1001;
    isProcExist = ReclaimPriorityManager::GetInstance().IsProcExist(pid, uid, account_id);
    EXPECT_EQ(isProcExist, false);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, IsSystemApp, TestSize.Level1)
{
    BundlePriorityInfo* bundle_1 = new BundlePriorityInfo("test", 101, 100);
    bool isSystem = ReclaimPriorityManager::GetInstance().IsSystemApp(bundle_1);
    EXPECT_EQ(isSystem, false);
    delete bundle_1;

    BundlePriorityInfo* bundle_2 = new BundlePriorityInfo("com.ohos.systemui", 102, 100);
    isSystem = ReclaimPriorityManager::GetInstance().IsSystemApp(bundle_2);
    EXPECT_EQ(isSystem, true);
    delete bundle_2;

    BundlePriorityInfo* bundle_3 = new BundlePriorityInfo("com.ohos.launcher", 103, 100);
    isSystem = ReclaimPriorityManager::GetInstance().IsSystemApp(bundle_3);
    EXPECT_EQ(isSystem, true);
    delete bundle_3;
}

HWTEST_F(MemMgrTest, UpdateReclaimPriorityProcessCreate, TestSize.Level1)
{
    int pid = 1002;
    int uid = 104;
    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    EXPECT_EQ(account_id, 0);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);

    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    bool hasBundle = account->HasBundle(uid);
    EXPECT_EQ(hasBundle, true);

    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPriorityProcessTerminate, TestSize.Level1)
{
    int pid_1 = 1003;
    int pid_2 = 1004;
    int uid = 105;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid_1, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid_2, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid_2, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    bool hasProc_1 = bundle->HasProc(pid_1);
    EXPECT_EQ(hasProc_1, true);
    bool hasProc_2 = bundle->HasProc(pid_2);
    EXPECT_EQ(hasProc_2, false);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid_1, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPrioritySystemProcess, TestSize.Level1)
{
    int pid = 1005;
    int uid = 106;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.systemui", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.systemui", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    bool isSystem = ReclaimPriorityManager::GetInstance().IsSystemApp(bundle);
    EXPECT_EQ(isSystem, true);

    int priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_SYSTEM);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPriorityBackground, TestSize.Level1)
{
    int pid = 1006;
    int uid = 107;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    int priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPrioritySuspendDelayStart, TestSize.Level1)
{
    int pid = 1007;
    int uid = 108;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::SUSPEND_DELAY_START);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    int priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_FOREGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::SUSPEND_DELAY_START);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_SUSPEND_DELAY);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPrioritySuspendDelayEnd, TestSize.Level1)
{
    int pid = 1008;
    int uid = 109;
    int priority;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::SUSPEND_DELAY_START);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_SUSPEND_DELAY);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::SUSPEND_DELAY_END);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPriorityBgRunningStart, TestSize.Level1)
{
    int pid = 1009;
    int uid = 110;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_START);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    int priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_FOREGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_START);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPriorityBgRunningEnd, TestSize.Level1)
{
    int pid = 1010;
    int uid = 111;
    int priority;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_START);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_PERCEIVED);


    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_END);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPriorityEventStart, TestSize.Level1)
{
    int pid = 1011;
    int uid = 112;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_START);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    int priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_FOREGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_START);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPriorityEventEnd, TestSize.Level1)
{
    int pid = 1012;
    int uid = 113;
    int priority;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_START);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_END);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPriorityDataAbilityStart, TestSize.Level1)
{
    int pid = 1013;
    int uid = 114;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::DATA_ABILITY_START);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    int priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_FOREGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::DATA_ABILITY_START);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, UpdateReclaimPriorityDataAbilityEnd, TestSize.Level1)
{
    int pid = 1014;
    int uid = 115;
    int priority;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    OsAccountPriorityInfo *account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    BundlePriorityInfo* bundle = account->FindBundleById(uid);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::DATA_ABILITY_START);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_PERCEIVED);


    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::DATA_ABILITY_END);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(MemMgrTest, GetBundlePrioSet, TestSize.Level1)
{
    int pid = 1015;
    int uid = 116;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    const ReclaimPriorityManager::BundlePrioSet bundleSet = ReclaimPriorityManager::GetInstance().GetBundlePrioSet();
    bool isEmpty = bundleSet.size() == 0;
    EXPECT_EQ(isEmpty, false);
}
}
}