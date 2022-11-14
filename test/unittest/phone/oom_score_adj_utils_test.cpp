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
#include "reclaim_priority_manager.h"
#include "multi_account_manager.h"
#include "oom_score_adj_utils.h"
#include "account_bundle_info.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class OomScoreAdjUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void OomScoreAdjUtilsTest::SetUpTestCase()
{
}

void OomScoreAdjUtilsTest::TearDownTestCase()
{
}

void OomScoreAdjUtilsTest::SetUp()
{
}

void OomScoreAdjUtilsTest::TearDown()
{
}

/**
 * @tc.name: WriteOomScoreAdjToKernel
 * @tc.desc: Test the branch into "for" branch
 * @tc.desc: Test the branch of bundle equals to nullptr
 * @tc.type: FUNC
 */
HWTEST_F(OomScoreAdjUtilsTest, WriteOomScoreAdjToKernelTest1, TestSize.Level1)
{
    int accountId = 100;
    std::shared_ptr<BundlePriorityInfo> bundle = std::make_shared<BundlePriorityInfo>("app",
            accountId * USER_ID_SHIFT + 1, 100);

    // Test the branch into "for" branch
    ProcessPriorityInfo proc1(1001, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc2(1002, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc3(1003, bundle->uid_, bundle->priority_);
    ProcessPriorityInfo proc4(1004, bundle->uid_, bundle->priority_);
    bundle->AddProc(proc1);
    bundle->AddProc(proc2);
    bundle->AddProc(proc3);
    bundle->AddProc(proc4);
    OomScoreAdjUtils::WriteOomScoreAdjToKernel(bundle);
    EXPECT_NE(bundle->procs_.begin(), bundle->procs_.end());

    // Test the branch of bundle equals to nullptr
    bundle = nullptr;
    bool ret = OomScoreAdjUtils::WriteOomScoreAdjToKernel(bundle);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: WriteOomScoreAdjToKernel
 * @tc.type: FUNC
 */
HWTEST_F(OomScoreAdjUtilsTest, WriteOomScoreAdjToKernelTest2, TestSize.Level1)
{
    pid_t pid = 0;
    int priority = 0;
    static bool ret = OomScoreAdjUtils::WriteOomScoreAdjToKernel(pid, priority);
    EXPECT_EQ(ret, true);
}

}
}
