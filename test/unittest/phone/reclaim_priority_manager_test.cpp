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

HWTEST_F(ReclaimPriorityManagerTest, InitTest, TestSize.Level1)
{
    EXPECT_EQ(ReclaimPriorityManager::GetInstance().Init(), true);
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

static inline UpdateRequest CreateUpdateRequestForExtension(int callerPid, int callerUid,
                            std::string callerBundleName, int pid, int uid,
                            std::string bundleName, AppStateUpdateReason reason)
{
    return CallerRequest({callerPid, callerUid, callerBundleName}, {pid, uid, bundleName}, reason);
}

static inline UpdateRequest CreateUpdateRequest(int pid, int uid,
                            std::string bundleName, AppStateUpdateReason reason)
{
    return SingleRequest({pid, uid, bundleName}, reason);
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

    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);

    bool isProcExist = ReclaimPriorityManager::GetInstance().IsProcExist(pid, uid, account_id);
    EXPECT_EQ(isProcExist, true);
    isProcExist = ReclaimPriorityManager::GetInstance().IsProcExist(pid+1, uid, account_id);
    EXPECT_EQ(isProcExist, false);

    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityProcessCreate, TestSize.Level1)
{
    int pid = 10002;
    int uid = 20010002;
    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    EXPECT_EQ(account_id, 100);

    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);

    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    bool hasBundle = account->HasBundle(uid);
    EXPECT_EQ(hasBundle, true);

    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityProcessTerminate, TestSize.Level1)
{
    int pid_1 = 10003;
    int pid_2 = 10004;
    int uid = 20010003;
    UpdateRequest request1 = CreateUpdateRequest(pid_1, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid_2, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request3 = CreateUpdateRequest(pid_2, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    UpdateRequest request4 = CreateUpdateRequest(pid_1, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    bool hasProc_1 = bundle->HasProc(pid_1);
    EXPECT_EQ(hasProc_1, true);
    bool hasProc_2 = bundle->HasProc(pid_2);
    EXPECT_EQ(hasProc_2, false);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request4);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityBackground, TestSize.Level1)
{
    int pid = 10006;
    int uid = 20010006;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    UpdateRequest request3 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    int priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPrioritySuspendDelayStart, TestSize.Level1)
{
    int pid = 10007;
    int uid = 20010007;
    printf("process created!");
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_suspend_delay_start", AppStateUpdateReason::SUSPEND_DELAY_START);
    UpdateRequest request3 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_suspend_delay_start", AppStateUpdateReason::BACKGROUND);
    UpdateRequest request4 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_suspend_delay_start", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    PrintReclaimPriorityList();

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    printf("process suspend delay start!");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
    PrintReclaimPriorityList();
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_SUSPEND_DELAY);

    printf("process go to background!");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);
    PrintReclaimPriorityList();
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_SUSPEND_DELAY);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request4);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPrioritySuspendDelayEnd, TestSize.Level1)
{
    int pid = 10008;
    int uid = 20010008;
    const std::string bundleName = "com.ohos.reclaim_suspend_delay_end";

    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                bundleName, AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                bundleName, AppStateUpdateReason::BACKGROUND);
    UpdateRequest request3 = CreateUpdateRequest(pid, uid,
                                bundleName, AppStateUpdateReason::SUSPEND_DELAY_START);
    UpdateRequest request4 = CreateUpdateRequest(pid, uid,
                                bundleName, AppStateUpdateReason::SUSPEND_DELAY_END);
    UpdateRequest request5 = CreateUpdateRequest(pid, uid,
                                bundleName, AppStateUpdateReason::PROCESS_TERMINATED);

    printf("process created!\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    printf("process go to background!\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
    PrintReclaimPriorityList();

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    printf("process suspend delay start!\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);
    PrintReclaimPriorityList();
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_SUSPEND_DELAY);

    printf("process suspend delay end!\n");
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request4);
    sleep(5);
    PrintReclaimPriorityList();
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request5);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityBgRunningStart, TestSize.Level1)
{
    int pid = 10009;
    int uid = 20010009;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_START);
    UpdateRequest request3 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    UpdateRequest request4 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    int priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
    priority = bundle->priority_;
    EXPECT_EQ(priority, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request4);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityBgRunningEnd, TestSize.Level1)
{
    int pid = 10010;
    int uid = 20010010;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    UpdateRequest request3 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_START);
    UpdateRequest request4 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND_RUNNING_END);
    UpdateRequest request5 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_PERCEIVED);


    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request4);
    sleep(5);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request5);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityEventStart, TestSize.Level1)
{
    int pid = 10011;
    int uid = 20010011;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_START);
    UpdateRequest request3 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    UpdateRequest request4 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request4);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityEventEnd, TestSize.Level1)
{
    int pid = 10012;
    int uid = 20010012;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_START);
    UpdateRequest request3 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::BACKGROUND);
    UpdateRequest request4 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    UpdateRequest request5 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::EVENT_END);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_PERCEIVED);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request5);
    sleep(5);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request4);
}

HWTEST_F(ReclaimPriorityManagerTest, GetBundlePrioSet, TestSize.Level1)
{
    int pid = 10015;
    int uid = 20010015;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::BunldeCopySet bundleSet;
    ReclaimPriorityManager::GetInstance().GetBundlePrioSet(bundleSet);
    bool isEmpty = bundleSet.size() == 0;
    EXPECT_EQ(isEmpty, false);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityApplicationSuspend, TestSize.Level1)
{
    int pid = 10016;
    int uid = 20010016;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::APPLICATION_SUSPEND);
    UpdateRequest request3 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);

    int account_id = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(account_id);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_SUSPEND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);
}

std::shared_ptr<BundlePriorityInfo> GetBundle(int pid1, int pid2, int bundleUid, std::string bundleName1,
    std::string bundleName2)
{
    int accountId = ReclaimPriorityManager::GetInstance().GetOsAccountLocalIdFromUid(bundleUid);
    bool isProc1Exist = ReclaimPriorityManager::GetInstance().IsProcExist(pid1, bundleUid, accountId);
    bool isProc2Exist = ReclaimPriorityManager::GetInstance().IsProcExist(pid2, bundleUid, accountId);
    if (!isProc1Exist || !isProc2Exist) {
        return nullptr;
    }
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(bundleUid);
    return bundle;
}

HWTEST_F(ReclaimPriorityManagerTest, DistDeviceCase, TestSize.Level1)
{
    // Preconditions: create one bundle with two freground processes
    int pid1 = 10017;
    int pid2 = 10018;
    int bundleUid = 20010017;
    const std::string bundleName1 = "com.ohos.reclaim_dist_device_test.process1";
    const std::string bundleName2 = "com.ohos.reclaim_dist_device_test.process2";
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid1, bundleUid,
        bundleName1, AppStateUpdateReason::CREATE_PROCESS));
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::CREATE_PROCESS));

    std::shared_ptr<BundlePriorityInfo> bundle = GetBundle(pid1, pid2, bundleUid, bundleName1, bundleName2);
    ASSERT_EQ(bundle == nullptr, false);
    ProcessPriorityInfo &proc1 = bundle->FindProcByPid(pid1);
    ProcessPriorityInfo &proc2 = bundle->FindProcByPid(pid2);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    // process#1 keep freground, process#2 go to background
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::BACKGROUND));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    // process#2 is connected to a distribute device
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::DIST_DEVICE_CONNECTED));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BG_DIST_DEVICE);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_DIST_DEVICE);

    // process#1 go to background
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid1, bundleUid,
        bundleName1, AppStateUpdateReason::BACKGROUND));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BG_DIST_DEVICE);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_DIST_DEVICE);

    // process#2 is disconnected to a distribute device
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::DIST_DEVICE_DISCONNECTED));
    sleep(5);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    // clean up the mess
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid1, bundleUid,
        bundleName1, AppStateUpdateReason::PROCESS_TERMINATED));
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::PROCESS_TERMINATED));
}

HWTEST_F(ReclaimPriorityManagerTest, ExtensionBindCase, TestSize.Level1)
{
    // Preconditions: create one bundle with two freground processes
    int pid1 = 10019;
    int pid2 = 10020;
    int bundleUid = 20010019;
    int callerPid = 99999;
    int callerUid = 20099999;
    std::string caller = "com.ohos.caller";
    const std::string bundleName1 = "com.ohos.exten_bind_test.main";
    const std::string bundleName2 = "com.ohos.exten_bind_test.extension";

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid1, bundleUid,
        bundleName1, AppStateUpdateReason::CREATE_PROCESS));
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::CREATE_PROCESS));

    std::shared_ptr<BundlePriorityInfo> bundle = GetBundle(pid1, pid2, bundleUid, bundleName1, bundleName2);
    ASSERT_EQ(bundle == nullptr, false);
    ProcessPriorityInfo &proc1 = bundle->FindProcByPid(pid1);
    ProcessPriorityInfo &proc2 = bundle->FindProcByPid(pid2);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    // process#1 keep freground, process#2 go to background
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::BACKGROUND));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    // process#2 is bind to a process
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequestForExtension(
        callerPid, callerUid, caller, pid2, bundleUid, bundleName2, AppStateUpdateReason::BIND_EXTENSION));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);

    // process#1 go to background
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid1, bundleUid,
        bundleName1, AppStateUpdateReason::BACKGROUND));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);

    // process#2 is unbind to a process
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequestForExtension(
        callerPid, callerUid, caller, pid2, bundleUid, bundleName2, AppStateUpdateReason::UNBIND_EXTENSION));
    sleep(5);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_NO_BIND_EXTENSION);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    // clean up the mess
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid1, bundleUid,
        bundleName1, AppStateUpdateReason::PROCESS_TERMINATED));
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::PROCESS_TERMINATED));
}

/**
 * @tc.name: OsAccountChanged
 * @tc.desc: Test the value of initialized_ equals to false
 * @tc.desc: Test the branch of bundle equals to nullptr
 * @tc.desc: Test the return value
 * @tc.type: FUNC
 */
HWTEST_F(ReclaimPriorityManagerTest, OsAccountChangedTest, TestSize.Level1)
{
    ReclaimPriorityManager reclPri;
    reclPri.initialized_ = false;
    int accountId = 100;

    // Test the value of initialized_ equals to false
    AccountSA::OS_ACCOUNT_SWITCH_MOD switchMod = AccountSA::OsAccountManager::GetOsAccountSwitchMod();
    bool accChan = reclPri.OsAccountChanged(accountId, switchMod);
    EXPECT_EQ(accChan, false);

    // Test the branch of bundle equals to nullptr
    reclPri.initialized_ = true;
    accountId = -1;
    accChan = ReclaimPriorityManager::GetInstance().OsAccountChanged(accountId, switchMod);
    EXPECT_EQ(accChan, false);

    // Test the return value
    accountId = 100;
    accChan = ReclaimPriorityManager::GetInstance().OsAccountChanged(accountId, switchMod);
    EXPECT_EQ(accChan, true);
}

/**
 * @tc.name: AddBundleInfoToSet
 * @tc.desc: Test the branch into "if"
 * @tc.type: FUNC
 */
HWTEST_F(ReclaimPriorityManagerTest, AddBundleInfoToSetTest, TestSize.Level1)
{
    int accountId = 100;
    std::shared_ptr<BundlePriorityInfo> bundle = std::make_shared<BundlePriorityInfo>("app",
            accountId * USER_ID_SHIFT + 1, 100);
    ProcessPriorityInfo proc1(1001, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc2(1002, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc3(1003, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc4(1004, bundle->uid_, bundle->priority_);
    bundle->AddProc(proc1);
    bundle->AddProc(proc2);
    bundle->AddProc(proc3);
    bundle->AddProc(proc4);
    ReclaimPriorityManager::GetInstance().AddBundleInfoToSet(bundle);
    ReclaimPriorityManager totBun;
    auto ret = totBun.totalBundlePrioSet_.insert(bundle);
    EXPECT_EQ(ret.second, true);
}

/**
 * @tc.name: UpdateBundlePriority
 * @tc.desc: Test Update the value of bundle
 * @tc.type: FUNC
 */
HWTEST_F(ReclaimPriorityManagerTest, UpdateBundlePriorityTest, TestSize.Level1)
{
    int accountId = 100;
    std::shared_ptr<BundlePriorityInfo> bundle = std::make_shared<BundlePriorityInfo>("app",
            accountId * USER_ID_SHIFT + 1, 100);
    ProcessPriorityInfo proc1(1001, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc2(1002, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc3(1003, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc4(1004, bundle->uid_, bundle->priority_);
    bundle->AddProc(proc1);
    bundle->AddProc(proc2);
    bundle->AddProc(proc3);
    bundle->AddProc(proc4);
    ReclaimPriorityManager::GetInstance().UpdateBundlePriority(bundle);
    ReclaimPriorityManager totBun;
    auto ret = totBun.totalBundlePrioSet_.insert(bundle);
    EXPECT_EQ(ret.second, true);
}

/**
 * @tc.name: DeleteBundleInfoFromSet
 * @tc.desc: Test Delete the value of bundle
 * @tc.type: FUNC
 */
HWTEST_F(ReclaimPriorityManagerTest, DeleteBundleInfoFromSetTest, TestSize.Level1)
{
    int accountId = 100;
    std::shared_ptr<BundlePriorityInfo> bundle1 = std::make_shared<BundlePriorityInfo>("app",
            accountId * USER_ID_SHIFT + 1, 100);
    std::shared_ptr<BundlePriorityInfo> bundle2 = std::make_shared<BundlePriorityInfo>("app",
            accountId * USER_ID_SHIFT + 1, 100);
    ReclaimPriorityManager::GetInstance().DeleteBundleInfoFromSet(bundle2);
    EXPECT_NE(bundle1, bundle2);
}

/**
 * @tc.name: GetOneKillableBundle
 * @tc.desc: Test the branch into "for"
 * @tc.desc: Test the branch into bundle->priority_ < minPrio
 * @tc.desc: Test the branch into bundle->GetState() == STATE_WAITING_FOR_KILL
 * @tc.type: FUNC
 */
HWTEST_F(ReclaimPriorityManagerTest, GetOneKillableBundleTest, TestSize.Level1)
{
    ReclaimPriorityManager tolBun1;
    ReclaimPriorityManager tolBun2;
    ReclaimPriorityManager::BunldeCopySet bundleSet;
    int accountId = 100;
    int minPrio = 200;
    std::shared_ptr<BundlePriorityInfo> bundle1 = std::make_shared<BundlePriorityInfo>("app",
            accountId * USER_ID_SHIFT + 1, 100);
    std::shared_ptr<BundlePriorityInfo> bundle2 = std::make_shared<BundlePriorityInfo>("app",
            accountId * USER_ID_SHIFT + 1, 100, 1, BundleState::STATE_WAITING_FOR_KILL);
    tolBun1.totalBundlePrioSet_.insert(bundle1);
    auto itrBundle1 = tolBun1.totalBundlePrioSet_.rbegin();

    // Test the branch into "for"
    ReclaimPriorityManager::GetInstance().GetOneKillableBundle(minPrio, bundleSet);
    EXPECT_NE(itrBundle1, tolBun1.totalBundlePrioSet_.rend());

    tolBun2.totalBundlePrioSet_.insert(bundle2);
    auto itrBundle2 = tolBun2.totalBundlePrioSet_.rbegin();
    std::shared_ptr<BundlePriorityInfo> bundle3 = *itrBundle2;

    // Test the branch into priority_ < minPrio
    ReclaimPriorityManager::GetInstance().GetOneKillableBundle(minPrio, bundleSet);
    EXPECT_EQ(bundle3->GetState(), BundleState::STATE_WAITING_FOR_KILL);

    // Test the branch into GetState() equals to STATE_WAITING_FOR_KILL
    ReclaimPriorityManager::GetInstance().GetOneKillableBundle(minPrio, bundleSet);
    EXPECT_LT(bundle3->priority_, minPrio);
}

/**
 * @tc.name: AppStateUpdateResonToString
 * @tc.desc: Test the branch into "if == true"
 * @tc.desc: Test the branch into "else"
 * @tc.type: FUNC
 */
HWTEST_F(ReclaimPriorityManagerTest, AppStateUpdateResonToStringTest, TestSize.Level1)
{
    ReclaimPriorityManager appState;
    AppStateUpdateReason reason1 = AppStateUpdateReason::CREATE_PROCESS;
    int reason2 = -1;

    // Test the branch into "if == true"
    ReclaimPriorityManager::GetInstance().AppStateUpdateResonToString(reason1);
    auto ptr = appState.updateReasonStrMapping_.find(static_cast<int32_t>(reason1));
    EXPECT_NE(ptr, appState.updateReasonStrMapping_.end());

    // Test the branch into "else"
    ReclaimPriorityManager::GetInstance().AppStateUpdateResonToString(static_cast<AppStateUpdateReason>(reason2));
    ptr = appState.updateReasonStrMapping_.find(reason2);
    EXPECT_EQ(ptr, appState.updateReasonStrMapping_.end());
}

}
}
