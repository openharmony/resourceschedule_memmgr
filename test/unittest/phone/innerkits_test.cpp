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
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class InnerkitsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

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
}
}
