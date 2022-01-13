/*
 * Copyright (c) 2021 XXXX.
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


/**
 * @tc.name: PutDeviceProfile_001
 * @tc.desc: put device profile with empty service id
 * @tc.type: FUNC
 * @tc.require: AR000GID2P
 */
HWTEST_F(MemMgrTest, GetRecalimPriorityManager_001, TestSize.Level1)
{
    ReclaimPriorityManager::GetInstance().CurrentOsAccountChanged(0);
    EXPECT_TRUE(0 != ReclaimPriorityManager::GetInstance().curOsAccountId_);
}

}
}