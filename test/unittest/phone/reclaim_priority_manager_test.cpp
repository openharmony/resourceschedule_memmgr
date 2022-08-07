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

class ReclaimPriorityManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ReclaimPriorityManagerTest::SetUpTestCase()
{
}

void ReclaimPriorityManagerTest::TearDownTestCase()
{
}

void ReclaimPriorityManagerTest::SetUp()
{
}

void ReclaimPriorityManagerTest::TearDown()
{
}

static void PrintReclaimPriorityList()
{
    ReclaimPriorityManager::BunldeCopySet bundleSet;
    ReclaimPriorityManager::GetInstance().GetBundlePrioSet(bundleSet);
    printf("begin print reclaim priority list. \n");
    printf("     uid                                            name   priority   accountId\n");
    for (auto bi : bundleSet) {
        printf("%3d\t%42s\t%5d\t%3d\n", bi.uid_, bi.name_.c_str(), bi.priority_, bi.accountId_);
        for (auto piPair : bi.procs_) {
            ProcessPriorityInfo &pi = piPair.second;
            printf("\tuid_=%3d, pid_=%5d, priority_=%5d, isFg=%d, isBgTsk=%d, isSusDelay=%d, isDistDevConn=%d, "
                "extensionBindStatus=%d\n",
                pi.uid_, pi.pid_, pi.priority_, pi.isFreground, pi.isBackgroundRunning, pi.isSuspendDelay,
                pi.isDistDeviceConnected, pi.extensionBindStatus);
        }
    }
    printf("-------------------------------------------------------------------------------\n");
}

HWTEST_F(ReclaimPriorityManagerTest, AddOsAccountInfo, TestSize.Level1)
{
    int account_id = 0;
    std::shared_ptr<AccountBundleInfo> account = std::make_shared<AccountBundleInfo>(account_id);
    ReclaimPriorityManager::GetInstance().AddOsAccountInfo(account);

    bool isAccountExist = ReclaimPriorityManager::GetInstance().IsOsAccountExist(account_id);
    EXPECT_EQ(isAccountExist, true);
}

HWTEST_F(ReclaimPriorityManagerTest, RemoveOsAccountById, TestSize.Level1)
{
    int account_id = 0;
    std::shared_ptr<AccountBundleInfo> account = std::make_shared<AccountBundleInfo>(account_id);
    ReclaimPriorityManager::GetInstance().AddOsAccountInfo(account);

    bool isAccountExist = ReclaimPriorityManager::GetInstance().IsOsAccountExist(account_id);
    EXPECT_EQ(isAccountExist, true);

    ReclaimPriorityManager::GetInstance().RemoveOsAccountById(account_id);
    isAccountExist = ReclaimPriorityManager::GetInstance().IsOsAccountExist(account_id);
    EXPECT_EQ(isAccountExist, false);
}

HWTEST_F(ReclaimPriorityManagerTest, IsProcExist, TestSize.Level1)
{
    int pid = 10001;
    int uid = 20010001;
    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    EXPECT_EQ(account_id, 100);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);

    bool isProcExist = ReclaimPriorityManager::GetInstance().IsProcExist(pid, uid, account_id);
    EXPECT_EQ(isProcExist, true);
    isProcExist = ReclaimPriorityManager::GetInstance().IsProcExist(pid+1, uid, account_id);
    EXPECT_EQ(isProcExist, false);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityProcessCreate, TestSize.Level1)
{
    int pid = 10002;
    int uid = 20010002;
    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    EXPECT_EQ(account_id, 100);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);

    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    bool hasBundle = account->HasBundle(uid);
    EXPECT_EQ(hasBundle, true);

    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityProcessTerminate, TestSize.Level1)
{
    int pid_1 = 10003;
    int pid_2 = 10004;
    int uid = 20010003;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid_1, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid_2, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid_2, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    bool hasProc_1 = bundle->HasProc(pid_1);
    EXPECT_EQ(hasProc_1, true);
    bool hasProc_2 = bundle->HasProc(pid_2);
    EXPECT_EQ(hasProc_2, false);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid_1, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityBackground, TestSize.Level1)
{
    int pid = 10006;
    int uid = 20010006;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    int priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPrioritySuspendDelayStart, TestSize.Level1)
{
    int pid = 10007;
    int uid = 20010007;
    printf("process created!");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_suspend_delay_start", AppStateUpdateReason::CREATE_PROCESS);
    PrintReclaimPriorityList();

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    printf("process suspend delay start!");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_suspend_delay_start", AppStateUpdateReason::SUSPEND_DELAY_START);
    PrintReclaimPriorityList();
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    printf("process go to background!");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_suspend_delay_start", AppStateUpdateReason::BACKGROUND);
    PrintReclaimPriorityList();
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_SUSPEND_DELAY);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_suspend_delay_start", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPrioritySuspendDelayEnd, TestSize.Level1)
{
    int pid = 10008;
    int uid = 20010008;
    const std::string bundleName = "com.ohos.reclaim_suspend_delay_end";

    printf("process created!\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid, bundleName,
        AppStateUpdateReason::CREATE_PROCESS);
    printf("process go to background!\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid, bundleName,
        AppStateUpdateReason::BACKGROUND);
    PrintReclaimPriorityList();

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    printf("process suspend delay start!\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid, bundleName,
        AppStateUpdateReason::SUSPEND_DELAY_START);
    PrintReclaimPriorityList();
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_SUSPEND_DELAY);

    printf("process suspend delay end!\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid, bundleName,
        AppStateUpdateReason::SUSPEND_DELAY_END);
    sleep(5);
    PrintReclaimPriorityList();
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid, bundleName,
        AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityBgRunningStart, TestSize.Level1)
{
    int pid = 10009;
    int uid = 20010009;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_START);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
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

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityBgRunningEnd, TestSize.Level1)
{
    int pid = 10010;
    int uid = 20010010;

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_START);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_PERCEIVED);


    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_END);
    sleep(5);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityEventStart, TestSize.Level1)
{
    int pid = 10011;
    int uid = 20010011;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_START);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_START);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityEventEnd, TestSize.Level1)
{
    int pid = 10012;
    int uid = 20010012;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_START);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_END);
    sleep(5);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityDataAbilityStart, TestSize.Level1)
{
    int pid = 10013;
    int uid = 20010013;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::DATA_ABILITY_START);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
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

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityDataAbilityEnd, TestSize.Level1)
{
    int pid = 10014;
    int uid = 20010014;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::DATA_ABILITY_START);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_PERCEIVED);


    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::DATA_ABILITY_END);
    sleep(5);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, GetBundlePrioSet, TestSize.Level1)
{
    int pid = 10015;
    int uid = 20010015;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::BunldeCopySet bundleSet;
    ReclaimPriorityManager::GetInstance().GetBundlePrioSet(bundleSet);
    bool isEmpty = bundleSet.size() == 0;
    EXPECT_EQ(isEmpty, false);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid, "com.ohos.reclaim_test",
        AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityApplicationSuspend, TestSize.Level1)
{
    int pid = 10016;
    int uid = 20010016;
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(IGNORE_PID, uid,
                "com.ohos.reclaim_test", AppStateUpdateReason::APPLICATION_SUSPEND);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_SUSPEND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid, uid, "com.ohos.reclaim_test",
        AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, DistDeviceCase, TestSize.Level1)
{
    // Preconditions: create one bundle with two freground processes
    printf("Preconditions: create one bundle with two freground processes\n");
    int pid1 = 10017;
    int pid2 = 10018;
    int bundleUid = 20010017;
    const std::string bundleName1 = "com.ohos.reclaim_dist_device_test.process1";
    const std::string bundleName2 = "com.ohos.reclaim_dist_device_test.process2";
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid1, bundleUid, bundleName1,
        AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::CREATE_PROCESS);

    int accountId = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(bundleUid);
    bool isProc1Exist = ReclaimPriorityManager::GetInstance().IsProcExist(pid1, bundleUid, accountId);
    ASSERT_EQ(isProc1Exist, true);
    bool isProc2Exist = ReclaimPriorityManager::GetInstance().IsProcExist(pid2, bundleUid, accountId);
    ASSERT_EQ(isProc2Exist, true);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(bundleUid);
    ProcessPriorityInfo &proc1 = bundle->FindProcByPid(pid1);
    ProcessPriorityInfo &proc2 = bundle->FindProcByPid(pid2);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    PrintReclaimPriorityList();

    // process#1 keep freground, process#2 go to background
    printf("process#1 keep freground, process#2 go to background\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::BACKGROUND);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    PrintReclaimPriorityList();

    // process#2 is connected to a distribute device
    printf("process#1 is connected to a distribute device\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::DIST_DEVICE_CONNECTED);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BG_DIST_DEVICE);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    PrintReclaimPriorityList();

    // process#1 go to background
    printf("process#1 go to background\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid1, bundleUid, bundleName1,
        AppStateUpdateReason::BACKGROUND);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BG_DIST_DEVICE);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_DIST_DEVICE);
    PrintReclaimPriorityList();

    // process#2 is disconnected to a distribute device
    printf("process#2 is disconnected to a distribute device\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::DIST_DEVICE_DISCONNECTED);
    sleep(5);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);
    PrintReclaimPriorityList();

    // clean up the mess
    printf("clean up the mess\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid1, bundleUid, bundleName1,
        AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::PROCESS_TERMINATED);
    PrintReclaimPriorityList();
}

HWTEST_F(ReclaimPriorityManagerTest, ExtensionBindCase, TestSize.Level1)
{
    // Preconditions: create one bundle with two freground processes
    printf("Preconditions: create one bundle with two freground processes\n");
    int pid1 = 10019;
    int pid2 = 10020;
    int bundleUid = 20010019;
    const std::string bundleName1 = "com.ohos.exten_bind_test.main";
    const std::string bundleName2 = "com.ohos.exten_bind_test.extension";
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid1, bundleUid, bundleName1,
        AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::CREATE_PROCESS);

    int accountId = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(bundleUid);
    bool isProc1Exist = ReclaimPriorityManager::GetInstance().IsProcExist(pid1, bundleUid, accountId);
    ASSERT_EQ(isProc1Exist, true);
    bool isProc2Exist = ReclaimPriorityManager::GetInstance().IsProcExist(pid2, bundleUid, accountId);
    ASSERT_EQ(isProc2Exist, true);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(bundleUid);
    ProcessPriorityInfo &proc1 = bundle->FindProcByPid(pid1);
    ProcessPriorityInfo &proc2 = bundle->FindProcByPid(pid2);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    PrintReclaimPriorityList();

    // process#1 keep freground, process#2 go to background
    printf("process#1 keep freground, process#2 go to background\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::BACKGROUND);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    PrintReclaimPriorityList();

    // process#2 is bind to a process
    printf("process#2 is bind to a process\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::BIND_EXTENSION);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    PrintReclaimPriorityList();

    // process#1 go to background
    printf("process#1 go to background\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid1, bundleUid, bundleName1,
        AppStateUpdateReason::BACKGROUND);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);
    PrintReclaimPriorityList();

    // process#2 is unbind to a process
    printf("process#2 is no bind to any process\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::UNBIND_EXTENSION);
    sleep(5);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_NO_BIND_EXTENSION);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_NO_BIND_EXTENSION);
    PrintReclaimPriorityList();

    // clean up the mess
    printf("clean up the mess\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid1, bundleUid, bundleName1,
        AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(pid2, bundleUid, bundleName2,
        AppStateUpdateReason::PROCESS_TERMINATED);
    PrintReclaimPriorityList();
}
}
}
