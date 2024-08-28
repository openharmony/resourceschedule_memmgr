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

static void Sleep(int second)
{
    sleep(second);
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

struct ProcUpdateInfo {
    int pid;
    int uid;
    std::string bundleName;
};

static inline UpdateRequest CreateUpdateRequestForExtension(ProcUpdateInfo caller, ProcUpdateInfo target,
    AppStateUpdateReason reason)
{
    return CallerRequest({caller.pid, caller.uid, caller.bundleName},
        {target.pid, target.uid, target.bundleName}, reason);
}

static inline UpdateRequest CreateUpdateRequest(int pid, int uid,
    std::string bundleName, AppStateUpdateReason reason)
{
    return SingleRequest({pid, uid, "", bundleName}, reason);
}

HWTEST_F(ReclaimPriorityManagerTest, AddOsAccountInfo, TestSize.Level1)
{
    int accountId = 0;
    std::shared_ptr<AccountBundleInfo> account = std::make_shared<AccountBundleInfo>(accountId);
    ReclaimPriorityManager::GetInstance().AddOsAccountInfo(account);

    bool isAccountExist = ReclaimPriorityManager::GetInstance().IsOsAccountExist(accountId);
    EXPECT_EQ(isAccountExist, true);
}

HWTEST_F(ReclaimPriorityManagerTest, RemoveOsAccountById, TestSize.Level1)
{
    int accountId = 0;
    std::shared_ptr<AccountBundleInfo> account = std::make_shared<AccountBundleInfo>(accountId);
    ReclaimPriorityManager::GetInstance().AddOsAccountInfo(account);

    bool isAccountExist = ReclaimPriorityManager::GetInstance().IsOsAccountExist(accountId);
    EXPECT_EQ(isAccountExist, true);

    ReclaimPriorityManager::GetInstance().RemoveOsAccountById(accountId);
    isAccountExist = ReclaimPriorityManager::GetInstance().IsOsAccountExist(accountId);
    EXPECT_EQ(isAccountExist, false);
}

HWTEST_F(ReclaimPriorityManagerTest, IsProcExist, TestSize.Level1)
{
    int pid = 10001;
    int uid = 20010001;
    int accountId = GetOsAccountIdByUid(uid);
    EXPECT_EQ(accountId, 100);

    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);

    bool isProcExist = ReclaimPriorityManager::GetInstance().IsProcExist(pid, uid, accountId);
    EXPECT_EQ(isProcExist, true);
    isProcExist = ReclaimPriorityManager::GetInstance().IsProcExist(pid+1, uid, accountId);
    EXPECT_EQ(isProcExist, false);

    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityProcessCreate, TestSize.Level1)
{
    int pid = 10002;
    int uid = 20010002;
    int accountId = GetOsAccountIdByUid(uid);
    EXPECT_EQ(accountId, 100);

    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);

    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    bool hasBundle = account->HasBundle(uid);
    EXPECT_EQ(hasBundle, true);

    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);

    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateReclaimPriorityProcessTerminate, TestSize.Level1)
{
    int pid1 = 10003;
    int pid2 = 10004;
    int uid = 20010003;
    UpdateRequest request1 = CreateUpdateRequest(pid1, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(pid2, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request3 = CreateUpdateRequest(pid2, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    UpdateRequest request4 = CreateUpdateRequest(pid1, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    bool hasProc1 = bundle->HasProc(pid1);
    EXPECT_EQ(hasProc1, true);
    bool hasProc2 = bundle->HasProc(pid2);
    EXPECT_EQ(hasProc2, false);

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

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
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

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

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

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
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

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
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

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
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

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
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

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
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

HWTEST_F(ReclaimPriorityManagerTest, CheckReclaimPriorityVisible, TestSize.Level1)
{
    int pid = 10020;
    int uid = 20010020;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_visible_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);

    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_visible_test", AppStateUpdateReason::VISIBLE);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    ProcessPriorityInfo& proc = bundle->FindProcByPid(pid);
    EXPECT_EQ(proc.priority_, RECLAIM_PRIORITY_VISIBLE);
}

HWTEST_F(ReclaimPriorityManagerTest, CheckCreateProcPriorityDelay_test1, TestSize.Level1)
{
    int pid = 10023;
    int uid = 20010023;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    ProcessPriorityInfo& proc = bundle->FindProcByPid(pid);
    EXPECT_EQ(proc.priority_, RECLAIM_PRIORITY_FOREGROUND);
    UpdateRequest request2 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::UNBIND_EXTENSION);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);
    EXPECT_EQ(proc.priority_, RECLAIM_PRIORITY_NO_BIND_EXTENSION);
    UpdateRequest request3 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::PROCESS_TERMINATED);
}

HWTEST_F(ReclaimPriorityManagerTest, CheckCreateProcPriorityDelay_test2, TestSize.Level1)
{
    int pid = 10024;
    int uid = 20010024;
    UpdateRequest request1 = CreateUpdateRequest(pid, uid,
                                "com.ohos.reclaim_test", AppStateUpdateReason::CREATE_PROCESS);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    ProcessPriorityInfo& proc = bundle->FindProcByPid(pid);
    EXPECT_EQ(proc.priority_, RECLAIM_PRIORITY_FOREGROUND);
    Sleep(20);
    EXPECT_EQ(proc.priority_, RECLAIM_PRIORITY_BACKGROUND);
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

    int accountId = GetOsAccountIdByUid(uid);
    std::shared_ptr<AccountBundleInfo> account = ReclaimPriorityManager::GetInstance().FindOsAccountById(accountId);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(uid);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_SUSPEND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);
}

std::shared_ptr<BundlePriorityInfo> GetBundle(int pid1, int pid2, int bundleUid, std::string bundleName1,
    std::string bundleName2)
{
    int accountId = GetOsAccountIdByUid(bundleUid);
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
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    // process#1 keep freground, process#2 go to background
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::BACKGROUND));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    // process#2 is connected to a distribute device
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(pid2, bundleUid,
        bundleName2, AppStateUpdateReason::DIST_DEVICE_CONNECTED));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BG_DIST_DEVICE);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

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
    ProcUpdateInfo caller = {99999, 20099999, "com.ohos.caller"};
    ProcUpdateInfo targets1 = {10019, 20010019, "com.ohos.exten_bind_test.main"};
    ProcUpdateInfo targets2 = {10020, 20010019, "com.ohos.exten_bind_test.extension"};

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(targets1.pid, targets1.uid,
        targets1.bundleName, AppStateUpdateReason::CREATE_PROCESS));
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(targets2.pid, targets2.uid,
        targets2.bundleName, AppStateUpdateReason::CREATE_PROCESS));

    std::shared_ptr<BundlePriorityInfo> bundle = GetBundle(targets1.pid, targets2.pid, targets1.uid,
        targets1.bundleName, targets2.bundleName);
    ASSERT_EQ(bundle == nullptr, false);
    ProcessPriorityInfo &proc1 = bundle->FindProcByPid(targets1.pid);
    ProcessPriorityInfo &proc2 = bundle->FindProcByPid(targets2.pid);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    // process#1 keep freground, process#2 go to background
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(targets2.pid, targets2.uid,
        targets2.bundleName, AppStateUpdateReason::BACKGROUND));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    // process#2 is bind to a process
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequestForExtension(
        caller, targets2, AppStateUpdateReason::BIND_EXTENSION));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_FOREGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);

    // process#1 go to background
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(targets1.pid, targets1.uid,
        targets1.bundleName, AppStateUpdateReason::BACKGROUND));
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);

    // process#2 is unbind to a process
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequestForExtension(
        caller, targets2, AppStateUpdateReason::UNBIND_EXTENSION));
    sleep(5);
    ASSERT_EQ(proc1.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(proc2.priority_, RECLAIM_PRIORITY_BACKGROUND);
    ASSERT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);

    // clean up the mess
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(targets1.pid, targets1.uid,
        targets1.bundleName, AppStateUpdateReason::PROCESS_TERMINATED));
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequest(targets2.pid, targets2.uid,
        targets2.bundleName, AppStateUpdateReason::PROCESS_TERMINATED));
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

HWTEST_F(ReclaimPriorityManagerTest, UpdateRecalimPrioritySyncWithLockTest1, TestSize.Level1)
{
    ReclaimPriorityManager manager;
    manager.Init();
    std::string bundleName = "test1_for_ability_start_sync";
    int32_t callerPid = -1;
    int32_t callerUid = -1;
    int32_t pid = 11667;
    int32_t bundleUid = 20090001;
    AppStateUpdateReason stateReason = AppStateUpdateReason::CREATE_PROCESS;
    manager.UpdateReclaimPriorityInner(SingleRequest({pid, bundleUid, "", bundleName}, stateReason));
    manager.Dump(1);


    int accountId = GetOsAccountIdByUid(bundleUid);
    std::shared_ptr<AccountBundleInfo> account = manager.FindOsAccountById(accountId);
    bool hasBundle = account->HasBundle(bundleUid);
    EXPECT_EQ(hasBundle, true);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(bundleUid);

    stateReason = AppStateUpdateReason::BACKGROUND;
    manager.UpdateReclaimPriorityInner(SingleRequest({pid, bundleUid, "", bundleName}, stateReason));
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);
    manager.Dump(1);

    stateReason = AppStateUpdateReason::ABILITY_START;
    manager.UpdateRecalimPrioritySyncWithLock(CallerRequest({callerPid, callerUid, "", ""},
        {pid, bundleUid, "", bundleName}, stateReason));
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    manager.Dump(1);

    Sleep(15);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);
    manager.Dump(1);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdateRecalimPrioritySyncWithLockTest2, TestSize.Level1)
{
    ReclaimPriorityManager manager;
    manager.Init();
    std::string bundleName = "test2_for_ability_start_sync";
    int32_t callerPid = -1;
    int32_t callerUid = -1;
    int32_t pid = 11669;
    int32_t bundleUid = 20290001;
    AppStateUpdateReason stateReason = AppStateUpdateReason::CREATE_PROCESS;
    manager.UpdateReclaimPriorityInner(SingleRequest({pid, bundleUid, "", bundleName}, stateReason));
    manager.Dump(1);

    int accountId = GetOsAccountIdByUid(bundleUid);
    std::shared_ptr<AccountBundleInfo> account = manager.FindOsAccountById(accountId);
    bool hasBundle = account->HasBundle(bundleUid);
    EXPECT_EQ(hasBundle, true);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(bundleUid);

    stateReason = AppStateUpdateReason::SUSPEND_DELAY_START;
    manager.UpdateReclaimPriorityInner(SingleRequest({pid, bundleUid, "", bundleName}, stateReason));
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_SUSPEND_DELAY);
    manager.Dump(1);

    stateReason = AppStateUpdateReason::ABILITY_START;
    manager.UpdateRecalimPrioritySyncWithLock(CallerRequest({callerPid, callerUid, "", ""},
        {pid, bundleUid, "", bundleName}, stateReason));
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    manager.Dump(1);

    Sleep(15);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BG_SUSPEND_DELAY);
    manager.Dump(1);
}

HWTEST_F(ReclaimPriorityManagerTest, NotifyProcessStateChangedAsyncTest, TestSize.Level1)
{
    ReclaimPriorityManager manager;
    manager.Init();
    std::string bundleName = "test1_for_ability_start_async";
    int32_t callerPid = -1;
    int32_t callerUid = -1;
    int32_t pid = 11998;
    int32_t bundleUid = 20040004;
    AppStateUpdateReason stateReason = AppStateUpdateReason::CREATE_PROCESS;
    manager.UpdateReclaimPriorityInner(SingleRequest({pid, bundleUid, "", bundleName}, stateReason));
    manager.Dump(1);

    int accountId = GetOsAccountIdByUid(bundleUid);
    std::shared_ptr<AccountBundleInfo> account = manager.FindOsAccountById(accountId);
    bool hasBundle = account->HasBundle(bundleUid);
    EXPECT_EQ(hasBundle, true);
    std::shared_ptr<BundlePriorityInfo> bundle = account->FindBundleById(bundleUid);

    stateReason = AppStateUpdateReason::BACKGROUND;
    manager.UpdateReclaimPriorityInner(SingleRequest({pid, bundleUid, "", bundleName}, stateReason));
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);
    manager.Dump(1);

    stateReason = AppStateUpdateReason::ABILITY_START;
    manager.UpdateReclaimPriority(CallerRequest({callerPid, callerUid, "", ""},
        {pid, bundleUid, "", bundleName}, stateReason));
    Sleep(4);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_FOREGROUND);
    manager.Dump(1);

    Sleep(15);
    EXPECT_EQ(bundle->priority_, RECLAIM_PRIORITY_BACKGROUND);
    manager.Dump(1);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdatePriorityByProcForExtensionTest1, TestSize.Level1)
{
    ReclaimPriorityManager manager;
    manager.Init();
    ProcUpdateInfo caller = {10055, 20010055, "com.ohos.exten_test.caller"};
    ProcUpdateInfo target = {10056, 20010056, "com.ohos.exten_test.target"};


    UpdateRequest request1 = CreateUpdateRequest(
        caller.pid, caller.uid, caller.bundleName, AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(
        target.pid, target.uid, target.bundleName, AppStateUpdateReason::CREATE_PROCESS);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);

    int callerAccountId = GetOsAccountIdByUid(caller.uid);
    std::shared_ptr<AccountBundleInfo> callerAccount =
        ReclaimPriorityManager::GetInstance().FindOsAccountById(callerAccountId);
    std::shared_ptr<BundlePriorityInfo> callerBundle = callerAccount->FindBundleById(caller.uid);
    ProcessPriorityInfo &callerProc = callerBundle->FindProcByPid(caller.pid);

    int targetAccountId = GetOsAccountIdByUid(target.uid);
    std::shared_ptr<AccountBundleInfo> targetAccount =
        ReclaimPriorityManager::GetInstance().FindOsAccountById(targetAccountId);
    std::shared_ptr<BundlePriorityInfo> targetBundle = targetAccount->FindBundleById(target.uid);
    ProcessPriorityInfo &targetProc = targetBundle->FindProcByPid(target.pid);
    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_FOREGROUND);
    EXPECT_EQ(targetProc.priority_, RECLAIM_PRIORITY_FOREGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequestForExtension(
        caller, target, AppStateUpdateReason::BIND_EXTENSION));
    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_FOREGROUND);
    EXPECT_EQ(targetProc.priority_, 100);

    UpdateRequest request3 = CreateUpdateRequest(
        caller.pid, caller.uid, caller.bundleName, AppStateUpdateReason::FOREGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);
    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_FOREGROUND);
    EXPECT_EQ(targetProc.priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);

    UpdateRequest request4 = CreateUpdateRequest(
        target.pid, target.uid, target.bundleName, AppStateUpdateReason::FOREGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request4);
    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_FOREGROUND);
    EXPECT_EQ(targetProc.priority_, RECLAIM_PRIORITY_FOREGROUND);

    UpdateRequest request5 = CreateUpdateRequest(
        caller.pid, caller.uid, caller.bundleName, AppStateUpdateReason::BACKGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request5);
    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_BACKGROUND);
    EXPECT_EQ(targetProc.priority_, RECLAIM_PRIORITY_FOREGROUND);

    UpdateRequest request6 = CreateUpdateRequest(
        caller.pid, caller.uid, caller.bundleName, AppStateUpdateReason::PROCESS_TERMINATED);
    UpdateRequest request7 = CreateUpdateRequest(
        target.pid, target.uid, target.bundleName, AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request6);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request7);
}

HWTEST_F(ReclaimPriorityManagerTest, UpdatePriorityByProcForExtensionTest2, TestSize.Level1)
{
    ProcUpdateInfo caller = {10021, 20010021, "com.ohos.exten_test.caller"};
    ProcUpdateInfo target = {10022, 20010022, "com.ohos.exten_test.target"};

    UpdateRequest request1 = CreateUpdateRequest(
        caller.pid, caller.uid, caller.bundleName, AppStateUpdateReason::CREATE_PROCESS);
    UpdateRequest request2 = CreateUpdateRequest(
        target.pid, target.uid, target.bundleName, AppStateUpdateReason::CREATE_PROCESS);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request1);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request2);

    int callerAccountId = GetOsAccountIdByUid(caller.uid);
    std::shared_ptr<AccountBundleInfo> callerAccount =
        ReclaimPriorityManager::GetInstance().FindOsAccountById(callerAccountId);
    std::shared_ptr<BundlePriorityInfo> callerBundle = callerAccount->FindBundleById(caller.uid);
    ProcessPriorityInfo &callerProc = callerBundle->FindProcByPid(caller.pid);

    int targetAccountId = GetOsAccountIdByUid(target.uid);
    std::shared_ptr<AccountBundleInfo> targetAccount =
        ReclaimPriorityManager::GetInstance().FindOsAccountById(targetAccountId);
    std::shared_ptr<BundlePriorityInfo> targetBundle = targetAccount->FindBundleById(target.uid);
    ProcessPriorityInfo &targetProc = targetBundle->FindProcByPid(target.pid);

    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_FOREGROUND);
    EXPECT_EQ(targetProc.priority_, RECLAIM_PRIORITY_FOREGROUND);

    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(CreateUpdateRequestForExtension(
        caller, target, AppStateUpdateReason::BIND_EXTENSION));
    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_FOREGROUND);
    EXPECT_EQ(targetProc.priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);

    UpdateRequest request3 = CreateUpdateRequest(
        caller.pid, caller.uid, caller.bundleName, AppStateUpdateReason::FOREGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request3);

    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_FOREGROUND);
    EXPECT_EQ(targetProc.priority_, RECLAIM_PRIORITY_FG_BIND_EXTENSION);

    UpdateRequest request4 = CreateUpdateRequest(
        caller.pid, caller.uid, caller.bundleName, AppStateUpdateReason::BACKGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request4);

    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_BACKGROUND);
    EXPECT_EQ(targetProc.priority_, RECLAIM_PRIORITY_NO_BIND_EXTENSION);

    UpdateRequest request5 = CreateUpdateRequest(
        target.pid, target.uid, target.bundleName, AppStateUpdateReason::BACKGROUND);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request5);
 
    EXPECT_EQ(callerProc.priority_, RECLAIM_PRIORITY_BACKGROUND);
    EXPECT_EQ(targetProc.priority_, RECLAIM_PRIORITY_BACKGROUND);

    UpdateRequest request6 = CreateUpdateRequest(
        caller.pid, caller.uid, caller.bundleName, AppStateUpdateReason::PROCESS_TERMINATED);
    UpdateRequest request7 = CreateUpdateRequest(
        target.pid, target.uid, target.bundleName, AppStateUpdateReason::PROCESS_TERMINATED);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request6);
    ReclaimPriorityManager::GetInstance().UpdateReclaimPriorityInner(request7);
}
}
}