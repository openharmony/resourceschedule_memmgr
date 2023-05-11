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
#include "memory_level_manager.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class MemoryLevelManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MemoryLevelManagerTest::SetUpTestCase()
{
}

void MemoryLevelManagerTest::TearDownTestCase()
{
}

void MemoryLevelManagerTest::SetUp()
{
}

void MemoryLevelManagerTest::TearDown()
{
}

/**
 * @tc.name: SetModerate and GetModerate
 * @tc.desc: Test PsiHandler
 * @tc.desc: Test PsiHandler
 * @tc.type: FUNC
 */
HWTEST_F(MemoryLevelManagerTest, PsiHandlerTest, TestSize.Level1)
{
    MemoryLevelManager::GetInstance().PsiHandler();
}

/**
 * @tc.name: SetModerate and GetModerate
 * @tc.desc: Test PsiHandlerInner
 * @tc.desc: Test PsiHandlerInner
 * @tc.type: FUNC
 */
HWTEST_F(MemoryLevelManagerTest, PsiHandlerInnerTest, TestSize.Level1)
{
    MemoryLevelManager::GetInstance().PsiHandlerInner();
}

/**
 * @tc.name: SetModerate and GetModerate
 * @tc.desc: Test GetEventHandler
 * @tc.desc: Test GetEventHandler
 * @tc.type: FUNC
 */
HWTEST_F(MemoryLevelManagerTest, GetEventHandlerTest, TestSize.Level1)
{
    bool ret = MemoryLevelManager::GetInstance().GetEventHandler();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SetModerate and GetModerate
 * @tc.desc: Test CalcSystemMemoryLevel
 * @tc.desc: Test CalcSystemMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(MemoryLevelManagerTest, CalcSystemMemoryLevelTest, TestSize.Level1)
{
    SystemMemoryInfo info;
    info.level = SystemMemoryLevel::MEMORY_LEVEL_LOW;
    bool ret = MemoryLevelManager::GetInstance().CalcSystemMemoryLevel(info);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: SetModerate and GetModerate
 * @tc.desc: Test CalcReclaimAppList
 * @tc.desc: Test CalcReclaimAppList
 * @tc.type: FUNC
 */
HWTEST_F(MemoryLevelManagerTest, CalcReclaimAppListTest, TestSize.Level1)
{
    std::vector<std::shared_ptr<AppEntity>> appList;
    bool ret = MemoryLevelManager::GetInstance().CalcReclaimAppList(appList);
    EXPECT_EQ(ret, true);
}
} // namespace Memory
} // namespace OHOS
