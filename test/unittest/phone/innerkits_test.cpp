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
#include "mem_mgr_client.h"
#include "bundle_priority_list.h"
#include "mem_mgr_constant.h"
#include "app_state_subscriber.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class AppStateSubscriberTest : public AppStateSubscriber {
public:
    void OnConnected() override;
    void OnDisconnected() override;
    void OnAppStateChanged(int32_t pid, int32_t uid, int32_t state) override;
    void ForceReclaim(int32_t pid, int32_t uid) override;
    void OnTrim(SystemMemoryLevel level) override;
    void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};

void AppStateSubscriberTest::OnConnected()
{
}

void AppStateSubscriberTest::OnDisconnected()
{
}

void AppStateSubscriberTest::OnAppStateChanged(int32_t pid, int32_t uid, int32_t state)
{
}

void AppStateSubscriberTest::ForceReclaim(int32_t pid, int32_t uid)
{
}

void AppStateSubscriberTest::OnTrim(SystemMemoryLevel level)
{
}

void AppStateSubscriberTest::OnRemoteDied(const wptr<IRemoteObject> &object)
{
}

class InnerkitsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static std::shared_ptr<MemMgrClient> memMgrClient_;
    static std::vector<std::shared_ptr<AppStateSubscriberTest>> appStateSubscriberTests;
};
std::vector<std::shared_ptr<AppStateSubscriberTest>> InnerkitsTest::appStateSubscriberTests =
    std::vector<std::shared_ptr<AppStateSubscriberTest>>();

void InnerkitsTest::SetUpTestCase()
{
}

void InnerkitsTest::TearDownTestCase()
{
}

void InnerkitsTest::SetUp()
{
}

void InnerkitsTest::TearDown()
{
    for (auto appStateSubscriberTest_ : appStateSubscriberTests) {
        MemMgrClient::GetInstance().UnsubscribeAppState(*appStateSubscriberTest_);
    }
    appStateSubscriberTests.clear();
}

HWTEST_F(InnerkitsTest, GetBundlePriorityList_Test, TestSize.Level1)
{
    BundlePriorityList bundlePrioList;
    int32_t ret = MemMgrClient::GetInstance().GetBundlePriorityList(bundlePrioList);
    EXPECT_EQ(ret, 0);
    bundlePrioList.Show();
}

HWTEST_F(InnerkitsTest, GetPriorityDescTest, TestSize.Level1)
{
    auto ptr = ReclaimPriorityMapping.find(RECLAIM_PRIORITY_SYSTEM);
    EXPECT_EQ(ptr->second, "System");
}

HWTEST_F(InnerkitsTest, NotifyDistDevStatus_Test, TestSize.Level1)
{
    int32_t ret = MemMgrClient::GetInstance().NotifyDistDevStatus(123, 456, "dist_dev_test", true);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(InnerkitsTest, GetKillLevelOfLmkd_Test, TestSize.Level1)
{
    int32_t killLevel;
    int32_t ret = MemMgrClient::GetInstance().GetKillLevelOfLmkd(killLevel);
    EXPECT_EQ(ret, 0);
    printf("lmkd kill level: %d\n", killLevel);
}

#ifdef USE_PURGEABLE_MEMORY

HWTEST_F(InnerkitsTest, RegisterActiveApps_Test, TestSize.Level1)
{
    int32_t pid = 1234;
    int32_t uid = 20012001;
    int32_t ret = MemMgrClient::GetInstance().RegisterActiveApps(pid, uid);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(InnerkitsTest, DeregisterActiveApps_Test, TestSize.Level1)
{
    int32_t pid = 1234;
    int32_t uid = 20012001;
    int32_t ret = MemMgrClient::GetInstance().DeregisterActiveApps(pid, uid);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(InnerkitsTest, SubscribeAppState_Test, TestSize.Level1)
{
    std::shared_ptr<AppStateSubscriberTest> appStateSubscriberTest_1 = std::make_shared<AppStateSubscriberTest>();
    EXPECT_NE(appStateSubscriberTest_1, nullptr);
    int32_t ret = MemMgrClient::GetInstance().SubscribeAppState(*appStateSubscriberTest_1);
    appStateSubscriberTests.emplace_back(appStateSubscriberTest_1);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(InnerkitsTest, UnsubscribeAppState_Test, TestSize.Level1)
{
    std::shared_ptr<AppStateSubscriberTest> appStateSubscriberTest_1 = std::make_shared<AppStateSubscriberTest>();
    EXPECT_NE(appStateSubscriberTest_1, nullptr);
    MemMgrClient::GetInstance().SubscribeAppState(*appStateSubscriberTest_1);
    appStateSubscriberTests.emplace_back(appStateSubscriberTest_1);
    int32_t ret = MemMgrClient::GetInstance().UnsubscribeAppState(*appStateSubscriberTest_1);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(InnerkitsTest, GetAvailableMemory_Test, TestSize.Level1)
{
    int32_t ret = MemMgrClient::GetInstance().GetAvailableMemory();
    EXPECT_NE(ret, 0);
}
#else
HWTEST_F(InnerkitsTest, RegisterActiveApps_Test, TestSize.Level1)
{
    int32_t pid = 1234;
    int32_t uid = 20012001;
    int32_t ret = MemMgrClient::GetInstance().RegisterActiveApps(pid, uid);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(InnerkitsTest, DeregisterActiveApps_Test, TestSize.Level1)
{
    int32_t pid = 1234;
    int32_t uid = 20012001;
    int32_t ret = MemMgrClient::GetInstance().DeregisterActiveApps(pid, uid);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(InnerkitsTest, SubscribeAppState_Test, TestSize.Level1)
{
    std::shared_ptr<AppStateSubscriberTest> appStateSubscriberTest_1 = std::make_shared<AppStateSubscriberTest>();
    EXPECT_NE(appStateSubscriberTest_1, nullptr);
    int32_t ret = MemMgrClient::GetInstance().SubscribeAppState(*appStateSubscriberTest_1);
    appStateSubscriberTests.emplace_back(appStateSubscriberTest_1);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(InnerkitsTest, UnsubscribeAppState_Test, TestSize.Level1)
{
    std::shared_ptr<AppStateSubscriberTest> appStateSubscriberTest_1 = std::make_shared<AppStateSubscriberTest>();
    EXPECT_NE(appStateSubscriberTest_1, nullptr);
    MemMgrClient::GetInstance().SubscribeAppState(*appStateSubscriberTest_1);
    appStateSubscriberTests.emplace_back(appStateSubscriberTest_1);
    int32_t ret = MemMgrClient::GetInstance().UnsubscribeAppState(*appStateSubscriberTest_1);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(InnerkitsTest, GetAvailableMemory_Test, TestSize.Level1)
{
    int32_t ret = MemMgrClient::GetInstance().GetAvailableMemory();
    EXPECT_EQ(ret, -1);
}
#endif // USE_PURGEABLE_MEMORY

}
}
