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
#include "low_memory_killer.h"
#include "reclaim_strategy_manager.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class LowMemoryKillerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void LowMemoryKillerTest::SetUpTestCase()
{
}

void LowMemoryKillerTest::TearDownTestCase()
{
}

void LowMemoryKillerTest::SetUp()
{
}

void LowMemoryKillerTest::TearDown()
{
}

HWTEST_F(LowMemoryKillerTest, PsiHandlerTest, TestSize.Level1)
{
    LowMemoryKiller::GetInstance().PsiHandler();
}

HWTEST_F(LowMemoryKillerTest, QueryKillMemoryPriorityPairTest, TestSize.Level1)
{
    unsigned int reta = 1;
    int retb = 1;
    std::pair<unsigned int, int> ret = std::make_pair(reta, retb);
    unsigned int currBufferKB = 54632;
    unsigned int targetBufKB = 65231;
    int killLevel = 0;
    
    ret = LowMemoryKiller::GetInstance().QueryKillMemoryPriorityPair(currBufferKB, targetBufKB, killLevel);
    EXPECT_NE(ret.first, 1);
    EXPECT_NE(ret.second, 1);
}

HWTEST_F(LowMemoryKillerTest, PsiHandlerInnerTest, TestSize.Level1)
{
    LowMemoryKiller::GetInstance().PsiHandlerInner();
}

HWTEST_F(LowMemoryKillerTest, KillOneBundleByPrioTest, TestSize.Level1)
{
    int pid = 123;
    int appId = 111;
    std::string appName = "com.test";
    int userId = 234;
    int score = 100;
    std::shared_ptr<ReclaimParam> para = std::make_shared<ReclaimParam>(pid, appId, appName, userId, score,
        AppAction::OTHERS);
    ReclaimStrategyManager::GetInstance().NotifyAppStateChanged(para);
    int minPrio = 10;
    int ret = 1;
    ret = LowMemoryKiller::GetInstance().KillOneBundleByPrio(minPrio);
    EXPECT_NE(ret, 1);
    MemcgMgr::GetInstance().RemoveUserMemcg(userId);
}

HWTEST_F(LowMemoryKillerTest, GetEventHandlerTest, TestSize.Level1)
{
    bool ret = LowMemoryKiller::GetInstance().GetEventHandler();
    EXPECT_EQ(ret, true);
}
}
}