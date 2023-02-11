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

#ifdef USE_PURGEABLE_MEMORY
#include "gtest/gtest.h"
#include <thread>
#include "utils.h"

#define private public
#define protected public
#include "mem_mgr_client.h"
#include "system_memory_level_config.h"
#include "purgeable_mem_manager.h"
#include "memory_level_manager.h"
#include "memmgr_config_manager.h"
#include "app_state_subscriber.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
namespace {
static constexpr int32_t SLEEP_TIME = 500;
static constexpr int32_t SLEEP_TIME_LONG = 5000;
}
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
    int OnConnectedCallTimes = 0;
    int OnDisconnectedCallTimes = 0;
    int OnAppStateChangedCallTimes = 0;
    int ForceReclaimCallTimes = 0;
    int OnTrimCallTimes = 0;
    int OnRemoteDiedCallTimes = 0;
};

class PurgeableMemMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    inline void SleepForFC()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    }
    inline void SleepForFiveSecond()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_LONG));
    }
    static std::shared_ptr<MemMgrClient> memMgrClient_;
    static std::vector<std::shared_ptr<AppStateSubscriberTest>> appStateSubscriberTests;
};
std::vector<std::shared_ptr<AppStateSubscriberTest>> PurgeableMemMgrTest::appStateSubscriberTests =
    std::vector<std::shared_ptr<AppStateSubscriberTest>>();
void AppStateSubscriberTest::OnConnected()
{
    OnConnectedCallTimes++;
}

void AppStateSubscriberTest::OnDisconnected()
{
    OnDisconnectedCallTimes++;
}

void AppStateSubscriberTest::OnAppStateChanged(int32_t pid, int32_t uid, int32_t state)
{
    OnAppStateChangedCallTimes++;
}

void AppStateSubscriberTest::ForceReclaim(int32_t pid, int32_t uid)
{
    ForceReclaimCallTimes++;
}

void AppStateSubscriberTest::OnTrim(SystemMemoryLevel level)
{
    OnTrimCallTimes++;
}

void AppStateSubscriberTest::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    OnRemoteDiedCallTimes++;
}

void PurgeableMemMgrTest::SetUpTestCase()
{
}

void PurgeableMemMgrTest::TearDownTestCase()
{
}

void PurgeableMemMgrTest::SetUp()
{
}

void PurgeableMemMgrTest::TearDown()
{
    for (auto appStateSubscriberTest_ : appStateSubscriberTests) {
        MemMgrClient::GetInstance().UnsubscribeAppState(*appStateSubscriberTest_);
    }
    appStateSubscriberTests.clear();
}

HWTEST_F(PurgeableMemMgrTest, SubscribeAppState_Test, TestSize.Level1)
{
    std::shared_ptr<AppStateSubscriberTest> appStateSubscriberTest_1 = std::make_shared<AppStateSubscriberTest>();
    EXPECT_NE(appStateSubscriberTest_1, nullptr);
    MemMgrClient::GetInstance().SubscribeAppState(*appStateSubscriberTest_1);
    appStateSubscriberTests.emplace_back(appStateSubscriberTest_1);
    SleepForFC();
    EXPECT_EQ(appStateSubscriberTest_1->OnConnectedCallTimes, 1);
    MemMgrClient::GetInstance().SubscribeAppState(*appStateSubscriberTest_1);
    SleepForFC();
    EXPECT_EQ(appStateSubscriberTest_1->OnConnectedCallTimes, 1);
    std::shared_ptr<AppStateSubscriberTest> appStateSubscriberTest_2 = std::make_shared<AppStateSubscriberTest>();
    EXPECT_NE(appStateSubscriberTest_2, nullptr);
    MemMgrClient::GetInstance().SubscribeAppState(*appStateSubscriberTest_2);
    appStateSubscriberTests.emplace_back(appStateSubscriberTest_2);
    SleepForFC();
    EXPECT_EQ(appStateSubscriberTest_1->OnConnectedCallTimes, 1);
    EXPECT_EQ(appStateSubscriberTest_2->OnConnectedCallTimes, 1);
}

HWTEST_F(PurgeableMemMgrTest, UnsubscribeAppState_Test, TestSize.Level1)
{
    std::shared_ptr<AppStateSubscriberTest> appStateSubscriberTest_1 = std::make_shared<AppStateSubscriberTest>();
    EXPECT_NE(appStateSubscriberTest_1, nullptr);
    MemMgrClient::GetInstance().SubscribeAppState(*appStateSubscriberTest_1);
    appStateSubscriberTests.emplace_back(appStateSubscriberTest_1);
    SleepForFC();
    MemMgrClient::GetInstance().UnsubscribeAppState(*appStateSubscriberTest_1);
    SleepForFC();
    EXPECT_EQ(appStateSubscriberTest_1->OnDisconnectedCallTimes, 1);
}

HWTEST_F(PurgeableMemMgrTest, AppStateListener_Test, TestSize.Level1)
{
    auto subscriber = AppStateSubscriberTest();
    auto subscriberImpl = std::make_shared<AppStateSubscriber::AppStateSubscriberImpl>(subscriber);
    subscriberImpl->recipient_ = sptr<AppStateSubscriber::AppStateSubscriberImpl::DeathRecipient>(
        new AppStateSubscriber::AppStateSubscriberImpl::DeathRecipient(*subscriberImpl));
    subscriberImpl->OnConnected();
    EXPECT_EQ(subscriber.OnConnectedCallTimes, 1);
    subscriberImpl->OnDisconnected();
    EXPECT_EQ(subscriber.OnDisconnectedCallTimes, 1);
    SystemMemoryLevel level = SystemMemoryLevel::MEMORY_LEVEL_MODERATE;
    subscriberImpl->OnTrim(level);
    EXPECT_EQ(subscriber.OnTrimCallTimes, 1);
    int32_t pid = 2001;
    int32_t uid = 20012001;
    int32_t state = 1;
    subscriberImpl->OnAppStateChanged(pid, uid, state);
    EXPECT_EQ(subscriber.OnAppStateChangedCallTimes, 1);
    subscriberImpl->ForceReclaim(pid, uid);
    EXPECT_EQ(subscriber.ForceReclaimCallTimes, 1);
    subscriberImpl->recipient_->OnRemoteDied(nullptr);
    EXPECT_EQ(subscriber.OnRemoteDiedCallTimes, 1);
}
}
}
#endif // USE_PURGEABLE_MEMORY