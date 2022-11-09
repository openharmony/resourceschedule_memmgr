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
#include "kernel_interface.h"
#include "reclaim_strategy_manager.h"
#include "reclaim_strategy_constants.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class ReclaimStrategyManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ReclaimStrategyManagerTest::SetUpTestCase()
{
}

void ReclaimStrategyManagerTest::TearDownTestCase()
{
}

void ReclaimStrategyManagerTest::SetUp()
{
}

void ReclaimStrategyManagerTest::TearDown()
{
}

HWTEST_F(ReclaimStrategyManagerTest, InitTest, TestSize.Level1)
{
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().Init(), true);
}

HWTEST_F(ReclaimStrategyManagerTest, NotifyAppStateChangedTest, TestSize.Level1)
{
    int pid = 1234567;
    int appId = 111111;
    std::string appName = "com.test";
    int userId = 234561;
    int score = 100;
    std::shared_ptr<ReclaimParam> para = std::make_shared<ReclaimParam>(pid, appId, appName, userId, score,
        AppAction::OTHERS);
    ReclaimStrategyManager::GetInstance().NotifyAppStateChanged(para);
    ReclaimStrategyManager::GetInstance().NotifyAppStateChanged(nullptr);
}

HWTEST_F(ReclaimStrategyManagerTest, NotifyAccountDiedTest, TestSize.Level1)
{
    int pid = 1234567;
    int appId = 111111;
    std::string appName = "com.test";
    int userId = 234561;
    int score = 100;
    std::shared_ptr<ReclaimParam> para = std::make_shared<ReclaimParam>(pid, appId, appName, userId, score,
        AppAction::CREATE_PROCESS_AND_APP);
    ReclaimStrategyManager::GetInstance().NotifyAppStateChanged(para);
    sleep(3);
    UserMemcg* memcg = MemcgMgr::GetInstance().GetUserMemcg(userId);
    EXPECT_EQ(memcg != nullptr, true);
    std::string memcgPath = memcg->GetMemcgPath_();
    ReclaimStrategyManager::GetInstance().NotifyAccountDied(userId);
    sleep(3);
    EXPECT_EQ(MemcgMgr::GetInstance().GetUserMemcg(userId) == nullptr, true);
    EXPECT_EQ(KernelInterface::GetInstance().IsDirExists(memcgPath), false);
}

HWTEST_F(ReclaimStrategyManagerTest, NotifyAccountPriorityChangedTest, TestSize.Level1)
{
    int pid = 1234567;
    int appId = 111111;
    std::string appName = "com.test";
    int userId = 234561;
    int score = 100;
    ReclaimStrategyManager::GetInstance().NotifyAccountPriorityChanged(userId, score);
    sleep(3);
    EXPECT_EQ(MemcgMgr::GetInstance().GetUserMemcg(userId) == nullptr, true);
    
    std::shared_ptr<ReclaimParam> para = std::make_shared<ReclaimParam>(pid, appId, appName, userId, score,
        AppAction::CREATE_PROCESS_AND_APP);
    ReclaimStrategyManager::GetInstance().NotifyAppStateChanged(para);
    sleep(3);
    UserMemcg* memcg = MemcgMgr::GetInstance().GetUserMemcg(userId);
    EXPECT_EQ(memcg != nullptr, true);
    std::string memcgPath = memcg->GetMemcgPath_();
    ReclaimStrategyManager::GetInstance().NotifyAccountPriorityChanged(userId, score + score);
    sleep(3);
    EXPECT_EQ(memcg->score_, score + score);
    ReclaimStrategyManager::GetInstance().NotifyAccountDied(userId);
    sleep(3);
    EXPECT_EQ(KernelInterface::GetInstance().IsDirExists(memcgPath), false);
}

HWTEST_F(ReclaimStrategyManagerTest, GetEventHandlerTest, TestSize.Level1)
{
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().GetEventHandler_(), true);
}

HWTEST_F(ReclaimStrategyManagerTest, HandleProcessCreateTest, TestSize.Level1)
{
    int pid = 1234567;
    int appId = 111111;
    std::string appName = "com.test";
    int userId = 234561;
    int score = 100;
    std::shared_ptr<ReclaimParam> para = std::make_shared<ReclaimParam>(pid, appId, appName, userId, score,
        AppAction::CREATE_PROCESS_AND_APP);
    ReclaimStrategyManager::GetInstance().NotifyAppStateChanged(para);
    sleep(3);
    UserMemcg* memcg = MemcgMgr::GetInstance().GetUserMemcg(userId);
    EXPECT_EQ(memcg != nullptr, true);
    std::string memcgPath = memcg->GetMemcgPath_();
    EXPECT_EQ(KernelInterface::GetInstance().IsDirExists(memcgPath), true);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(userId), true);
}

HWTEST_F(ReclaimStrategyManagerTest, GetReclaimRatiosByScoreTest, TestSize.Level1)
{
    ReclaimRatios ratios(50, 50, 50);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().GetReclaimRatiosByScore_(0, ratios), true);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().GetReclaimRatiosByScore_(100, ratios), true);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().GetReclaimRatiosByScore_(200, ratios), true);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().GetReclaimRatiosByScore_(500, ratios), true);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().GetReclaimRatiosByScore_(1000, ratios), true);
}

HWTEST_F(ReclaimStrategyManagerTest, GetValidScoreTest, TestSize.Level1)
{
    int score = RECLAIM_SCORE_MIN - 1;
    ReclaimStrategyManager::GetInstance().GetValidScore_(score);
    EXPECT_EQ(score, RECLAIM_SCORE_MIN);

    score = RECLAIM_SCORE_MIN;
    ReclaimStrategyManager::GetInstance().GetValidScore_(score);
    EXPECT_EQ(score, RECLAIM_SCORE_MIN);

    score = RECLAIM_SCORE_MAX;
    ReclaimStrategyManager::GetInstance().GetValidScore_(score);
    EXPECT_EQ(score, RECLAIM_SCORE_MAX);

    score = RECLAIM_SCORE_MAX + 1;
    ReclaimStrategyManager::GetInstance().GetValidScore_(score);
    EXPECT_EQ(score, RECLAIM_SCORE_MAX);
}

HWTEST_F(ReclaimStrategyManagerTest, HandleAppStateChangedTest, TestSize.Level1)
{
    int pid = 1234567;
    int appId = 111111;
    std::string appName = "com.test";
    int userId = 234561;
    int userId2 = VALID_USER_ID_MIN - 1;
    int score = 100;
    std::shared_ptr<ReclaimParam> para = std::make_shared<ReclaimParam>(pid, appId, appName, userId, score,
        AppAction::OTHERS);
    std::shared_ptr<ReclaimParam> para2 = std::make_shared<ReclaimParam>(pid, appId, appName, userId2, score,
        AppAction::OTHERS);
    ReclaimStrategyManager::GetInstance().HandleProcessCreate_(para);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().HandleAppStateChanged_(para2), false);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().HandleAppStateChanged_(nullptr), false);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(userId), true);
}

HWTEST_F(ReclaimStrategyManagerTest, HandleAccountDiedTest, TestSize.Level1)
{
    int pid = 1234567;
    int appId = 111111;
    std::string appName = "com.test";
    int userId = 234561;
    int score = 100;
    std::shared_ptr<ReclaimParam> para = std::make_shared<ReclaimParam>(pid, appId, appName, userId, score,
        AppAction::OTHERS);
    ReclaimStrategyManager::GetInstance().HandleProcessCreate_(para);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().HandleAccountDied_(userId), true);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().HandleAccountDied_(userId), false);
    userId = 23456;
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().HandleAccountDied_(userId), false);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(userId), false);
}

HWTEST_F(ReclaimStrategyManagerTest, HandleAccountPriorityChangedTest, TestSize.Level1)
{
    int pid = 1234567;
    int appId = 111111;
    std::string appName = "com.test";
    int userId = 234561;
    int score = 100;
    std::shared_ptr<ReclaimParam> para = std::make_shared<ReclaimParam>(pid, appId, appName, userId, score,
        AppAction::OTHERS);
    ReclaimStrategyManager::GetInstance().HandleProcessCreate_(para);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().HandleAccountPriorityChanged_(userId, RECLAIM_SCORE_MIN), true);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().HandleAccountDied_(userId), true);
    EXPECT_EQ(ReclaimStrategyManager::GetInstance().HandleAccountDied_(RECLAIM_SCORE_MIN), false);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(userId), false);
}
}
}
