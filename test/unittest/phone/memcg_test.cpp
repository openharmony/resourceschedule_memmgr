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
#include "memcg.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class MemcgTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MemcgTest::SetUpTestCase()
{
}

void MemcgTest::TearDownTestCase()
{
}

void MemcgTest::SetUp()
{
}

void MemcgTest::TearDown()
{
}

HWTEST_F(MemcgTest, CreateMemcgTest, TestSize.Level1)
{
    Memcg* memcg = new Memcg();
    EXPECT_EQ(memcg != nullptr, true);
    EXPECT_EQ(memcg->swapInfo_ != nullptr, true);
    EXPECT_EQ(memcg->memInfo_ != nullptr, true);
    EXPECT_EQ(memcg->reclaimRatios_ != nullptr, true);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(MemcgTest, UpdateMemInfoFromKernelTest, TestSize.Level1)
{
    Memcg* memcg = new Memcg();
    bool ret = memcg->UpdateMemInfoFromKernel();
    EXPECT_EQ(ret, true);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(MemcgTest, SetScoreTest, TestSize.Level1)
{
    Memcg* memcg = new Memcg();
    int score = 0;
    memcg->SetScore(score);
    EXPECT_EQ(memcg->score_, score);
    score = -1;
    memcg->SetScore(score);
    EXPECT_EQ(memcg->score_, score);
    score = 2000;
    memcg->SetScore(score);
    EXPECT_EQ(memcg->score_, score);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(MemcgTest, SetReclaimRatiosTest, TestSize.Level1)
{
    Memcg* memcg = new Memcg();
    memcg->SetReclaimRatios(0, 0, 0);
    EXPECT_EQ(memcg->reclaimRatios_->mem2zramRatio_, 0u);
    EXPECT_EQ(memcg->reclaimRatios_->zram2ufsRatio_, 0u);
    EXPECT_EQ(memcg->reclaimRatios_->refaultThreshold_, 0u);
    memcg->SetReclaimRatios(100, 100, 100);
    EXPECT_EQ(memcg->reclaimRatios_->mem2zramRatio_, 100u);
    EXPECT_EQ(memcg->reclaimRatios_->zram2ufsRatio_, 100u);
    EXPECT_EQ(memcg->reclaimRatios_->refaultThreshold_, 100u);

    ReclaimRatios ratios(50, 50, 50);
    EXPECT_EQ(memcg->SetReclaimRatios(ratios), true);
    EXPECT_EQ(memcg->reclaimRatios_->mem2zramRatio_, 50u);
    EXPECT_EQ(memcg->reclaimRatios_->zram2ufsRatio_, 50u);
    EXPECT_EQ(memcg->reclaimRatios_->refaultThreshold_, 50u);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(MemcgTest, SetScoreAndReclaimRatiosToKernelTest, TestSize.Level1)
{
    Memcg* memcg = new Memcg();
    EXPECT_EQ(memcg->SetScoreAndReclaimRatiosToKernel(), true);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(MemcgTest, SwapInTest, TestSize.Level1)
{
    Memcg* memcg = new Memcg();
    EXPECT_EQ(memcg->SwapIn(), true);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(MemcgTest, GetMemcgPathTest, TestSize.Level1)
{
    Memcg* memcg = new Memcg();
    EXPECT_STREQ(memcg->GetMemcgPath_().c_str(), "/dev/memcg");
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(MemcgTest, ReadScoreAndReclaimRatiosFromKernelTest, TestSize.Level1)
{
    Memcg* memcg = new Memcg();
    int score = 0;
    unsigned int mem2zramRatio = 0;
    unsigned int zram2ufsRatio = 0;
    unsigned int refaultThreshold = 0;
    bool ret = memcg->ReadScoreAndReclaimRatiosFromKernel_(score, mem2zramRatio, zram2ufsRatio, refaultThreshold);
    EXPECT_EQ(ret, true);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(MemcgTest, SetRatiosByValueTest, TestSize.Level1)
{
    unsigned int mem2zramRatio = 12345;
    unsigned int zram2ufsRatio = 54321;
    unsigned int refaultThreshold = 66666;
    ReclaimRatios ratios(mem2zramRatio, zram2ufsRatio, refaultThreshold);
    ratios.SetRatiosByValue(mem2zramRatio, zram2ufsRatio, refaultThreshold);
    EXPECT_EQ(ratios.mem2zramRatio_, 100);
    EXPECT_EQ(ratios.zram2ufsRatio_, 100);
    EXPECT_EQ(ratios.refaultThreshold_, 66666);
    mem2zramRatio = 58;
    zram2ufsRatio = 85;
    refaultThreshold = 77777;
    ratios.SetRatiosByValue(mem2zramRatio, zram2ufsRatio, refaultThreshold);
    EXPECT_EQ(ratios.mem2zramRatio_, 58);
    EXPECT_EQ(ratios.zram2ufsRatio_, 85);
    EXPECT_EQ(ratios.refaultThreshold_, 77777);
}

HWTEST_F(MemcgTest, SetRatiosTest, TestSize.Level1)
{
    unsigned int mem2zramRatio = 12345;
    unsigned int zram2ufsRatio = 54321;
    unsigned int refaultThreshold = 66666;
    ReclaimRatios ratios(mem2zramRatio, zram2ufsRatio, refaultThreshold);
    ratios.SetRatios(ratios);
    EXPECT_EQ(ratios.mem2zramRatio_, 100);
    EXPECT_EQ(ratios.zram2ufsRatio_, 100);
    EXPECT_EQ(ratios.refaultThreshold_, 66666);
    mem2zramRatio = 58;
    zram2ufsRatio = 85;
    refaultThreshold = 77777;
    ReclaimRatios ratios2(mem2zramRatio, zram2ufsRatio, refaultThreshold);
    ratios.SetRatios(ratios2);
    EXPECT_EQ(ratios.mem2zramRatio_, 58);
    EXPECT_EQ(ratios.zram2ufsRatio_, 85);
    EXPECT_EQ(ratios.refaultThreshold_, 77777);
}

HWTEST_F(MemcgTest, ToStringTest, TestSize.Level1)
{
    unsigned int mem2zramRatio = 12345;
    unsigned int zram2ufsRatio = 54321;
    unsigned int refaultThreshold = 66666;
    ReclaimRatios ratios(mem2zramRatio, zram2ufsRatio, refaultThreshold);
    std::string ret = ratios.ToString();
    EXPECT_EQ(ret, "mem2zramRatio:100 zram2ufsRatio:100 refaultThreshold:66666");
}

HWTEST_F(MemcgTest, CreateMemcgDirTest, TestSize.Level1)
{
    unsigned int userId = 23456;
    UserMemcg usermemcg= UserMemcg(userId);
    EXPECT_EQ(usermemcg.CreateMemcgDir(), true);
    EXPECT_EQ(usermemcg.RemoveMemcgDir(), true);
}

HWTEST_F(MemcgTest, RemoveMemcgDirTest, TestSize.Level1)
{
    unsigned int userId = 23456;
    UserMemcg usermemcg= UserMemcg(userId);
    EXPECT_EQ(usermemcg.CreateMemcgDir(), true);
    EXPECT_EQ(usermemcg.RemoveMemcgDir(), true);
    EXPECT_EQ(usermemcg.RemoveMemcgDir(), false);
}

HWTEST_F(MemcgTest, AddProcTest, TestSize.Level1)
{
    unsigned int userId = 23456;
    UserMemcg usermemcg= UserMemcg(userId);
    EXPECT_EQ(usermemcg.CreateMemcgDir(), true);
    EXPECT_EQ(usermemcg.AddProc(userId), false);
    EXPECT_EQ(usermemcg.RemoveMemcgDir(), true);
}
}
}
