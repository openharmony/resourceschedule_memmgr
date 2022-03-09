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
#include "memcg_mgr.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class MemcgMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MemcgMgrTest::SetUpTestCase()
{
}

void MemcgMgrTest::TearDownTestCase()
{
}

void MemcgMgrTest::SetUp()
{
}

void MemcgMgrTest::TearDown()
{
}

HWTEST_F(MemcgMgrTest, GetRootMemcgTest, TestSize.Level1)
{
    Memcg* memcg = MemcgMgr::GetInstance().GetRootMemcg();
    EXPECT_EQ(memcg != nullptr, true);
    EXPECT_STREQ(memcg->GetMemcgPath_().c_str(), "/dev/memcg");
}

HWTEST_F(MemcgMgrTest, SetRootMemcgParaTest, TestSize.Level1)
{
    EXPECT_EQ(MemcgMgr::GetInstance().SetRootMemcgPara(), true);
}

HWTEST_F(MemcgMgrTest, GetUserMemcgTest, TestSize.Level1)
{
    unsigned int memcgId = 123456u; // ensure it is a new ID
    EXPECT_EQ(MemcgMgr::GetInstance().GetUserMemcg(memcgId) == nullptr, true);
}

HWTEST_F(MemcgMgrTest, AddUserMemcgTest, TestSize.Level1)
{
    unsigned int memcgId = 123456u; // ensure it is a new ID
    EXPECT_EQ(MemcgMgr::GetInstance().GetUserMemcg(memcgId) == nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().AddUserMemcg(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().GetUserMemcg(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(memcgId), true);
    EXPECT_EQ(MemcgMgr::GetInstance().GetUserMemcg(memcgId) == nullptr, true);
}

HWTEST_F(MemcgMgrTest, RemoveUserMemcgTest, TestSize.Level1)
{
    unsigned int memcgId = 123456u; // ensure it is a new ID
    EXPECT_EQ(MemcgMgr::GetInstance().AddUserMemcg(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(memcgId), true);
    EXPECT_EQ(MemcgMgr::GetInstance().GetUserMemcg(memcgId) == nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(memcgId), false);
}

HWTEST_F(MemcgMgrTest, UpdateMemcgScoreAndReclaimRatiosTest, TestSize.Level1)
{
    unsigned int memcgId = 123456u; // ensure it is a new ID
    EXPECT_EQ(MemcgMgr::GetInstance().AddUserMemcg(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().UpdateMemcgScoreAndReclaimRatios(memcgId, 100, nullptr), false);
    ReclaimRatios* ratios = new ReclaimRatios();
    EXPECT_EQ(MemcgMgr::GetInstance().UpdateMemcgScoreAndReclaimRatios(memcgId, -1, ratios), false);
    EXPECT_EQ(MemcgMgr::GetInstance().UpdateMemcgScoreAndReclaimRatios(memcgId, 200, ratios), true);
    ratios->SetRatios(101, 101, 101);
    EXPECT_EQ(MemcgMgr::GetInstance().UpdateMemcgScoreAndReclaimRatios(memcgId, 300, ratios), false);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(memcgId), true);
    EXPECT_EQ(MemcgMgr::GetInstance().UpdateMemcgScoreAndReclaimRatios(memcgId, 300, ratios), false);
    delete ratios;
    ratios = nullptr;
}

HWTEST_F(MemcgMgrTest, AddProcToMemcgTest, TestSize.Level1)
{
    unsigned int memcgId = 123456u; // ensure it is a new ID
    EXPECT_EQ(MemcgMgr::GetInstance().AddProcToMemcg(1234567, memcgId), true);
    EXPECT_EQ(MemcgMgr::GetInstance().GetUserMemcg(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(memcgId), true);
}

HWTEST_F(MemcgMgrTest, SwapInMemcgTest, TestSize.Level1)
{
    unsigned int memcgId = 123456u; // ensure it is a new ID
    EXPECT_EQ(MemcgMgr::GetInstance().SwapInMemcg(memcgId), false);
    EXPECT_EQ(MemcgMgr::GetInstance().AddUserMemcg(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().SwapInMemcg(memcgId), true);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(memcgId), true);
}

HWTEST_F(MemcgMgrTest, GetMemcgSwapInfoTest, TestSize.Level1)
{
    unsigned int memcgId = 123456u; // ensure it is a new ID
    EXPECT_EQ(MemcgMgr::GetInstance().GetMemcgSwapInfo(memcgId) == nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().AddUserMemcg(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().GetMemcgSwapInfo(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(memcgId), true);
    EXPECT_EQ(MemcgMgr::GetInstance().GetMemcgSwapInfo(memcgId) == nullptr, true);
}

HWTEST_F(MemcgMgrTest, GetMemcgMemInfoTest, TestSize.Level1)
{
    unsigned int memcgId = 123456u; // ensure it is a new ID
    EXPECT_EQ(MemcgMgr::GetInstance().GetMemcgMemInfo(memcgId) == nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().AddUserMemcg(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().GetMemcgMemInfo(memcgId) != nullptr, true);
    EXPECT_EQ(MemcgMgr::GetInstance().RemoveUserMemcg(memcgId), true);
    EXPECT_EQ(MemcgMgr::GetInstance().GetMemcgMemInfo(memcgId) == nullptr, true);
}
}
}
