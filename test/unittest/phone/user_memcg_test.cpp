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
#include "memcg.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class UserMemcgTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static constexpr unsigned int userId_ = 123456u; // ensure it is a new ID
};

void UserMemcgTest::SetUpTestCase()
{
}

void UserMemcgTest::TearDownTestCase()
{
}

void UserMemcgTest::SetUp()
{
}

void UserMemcgTest::TearDown()
{
}

HWTEST_F(UserMemcgTest, CreateMemcgTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    EXPECT_EQ(memcg != nullptr, true);
    EXPECT_EQ(memcg->swapInfo_ != nullptr, true);
    EXPECT_EQ(memcg->memInfo_ != nullptr, true);
    EXPECT_EQ(memcg->reclaimRatios_ != nullptr, true);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(UserMemcgTest, UpdateMemInfoFromKernelTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    memcg->CreateMemcgDir();
    bool ret = memcg->UpdateMemInfoFromKernel();
    EXPECT_EQ(ret, true);
    memcg->RemoveMemcgDir();
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(UserMemcgTest, SetScoreTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
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

HWTEST_F(UserMemcgTest, SetReclaimRatiosTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    memcg->SetReclaimRatios(0, 0, 0);
    EXPECT_EQ(memcg->reclaimRatios_->mem2zramRatio_, 0u);
    EXPECT_EQ(memcg->reclaimRatios_->zram2ufsRatio_, 0u);
    EXPECT_EQ(memcg->reclaimRatios_->refaultThreshold_, 0u);
    memcg->SetReclaimRatios(100, 100, 100);
    EXPECT_EQ(memcg->reclaimRatios_->mem2zramRatio_, 100u);
    EXPECT_EQ(memcg->reclaimRatios_->zram2ufsRatio_, 100u);
    EXPECT_EQ(memcg->reclaimRatios_->refaultThreshold_, 100u);
    memcg->SetReclaimRatios(101, 101, 101);
    EXPECT_EQ(memcg->reclaimRatios_->mem2zramRatio_, 100u);
    EXPECT_EQ(memcg->reclaimRatios_->zram2ufsRatio_, 100u);
    EXPECT_EQ(memcg->reclaimRatios_->refaultThreshold_, 101u);

    ReclaimRatios* ratios = new ReclaimRatios();
    EXPECT_EQ(memcg->SetReclaimRatios(ratios), true);
    ratios->SetRatios(101, 101, 101);
    EXPECT_EQ(memcg->SetReclaimRatios(ratios), true);
    EXPECT_EQ(memcg->reclaimRatios_->mem2zramRatio_, 100u);
    EXPECT_EQ(memcg->reclaimRatios_->zram2ufsRatio_, 100u);
    EXPECT_EQ(memcg->reclaimRatios_->refaultThreshold_, 101u);
    delete ratios;
    ratios = nullptr;
    EXPECT_EQ(memcg->SetReclaimRatios(ratios), false);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(UserMemcgTest, SetScoreAndReclaimRatiosToKernelTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    memcg->CreateMemcgDir();
    EXPECT_EQ(memcg->SetScoreAndReclaimRatiosToKernel(), true);
    memcg->RemoveMemcgDir();
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(UserMemcgTest, SwapInTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    memcg->CreateMemcgDir();
    EXPECT_EQ(memcg->SwapIn(), true);
    memcg->RemoveMemcgDir();
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(UserMemcgTest, CreateMemcgDirTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    EXPECT_EQ(memcg->CreateMemcgDir(), true);
    EXPECT_EQ(KernelInterface::GetInstance().IsDirExists(memcg->GetMemcgPath_()), true);
    memcg->RemoveMemcgDir();
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(UserMemcgTest, RemoveMemcgDirTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    memcg->CreateMemcgDir();
    EXPECT_EQ(memcg->RemoveMemcgDir(), true);
    EXPECT_EQ(KernelInterface::GetInstance().IsDirExists(memcg->GetMemcgPath_()), false);
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(UserMemcgTest, AddProcTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    memcg->CreateMemcgDir();
    EXPECT_EQ(memcg->AddProc(12345678u), true);
    memcg->RemoveMemcgDir();
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(UserMemcgTest, GetMemcgPathTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    std::string path = "/dev/memcg/" + std::to_string(userId_);
    EXPECT_STREQ(memcg->GetMemcgPath_().c_str(), path.c_str());
    delete memcg;
    memcg = nullptr;
}

HWTEST_F(UserMemcgTest, ReadScoreAndReclaimRatiosFromKernelTest, TestSize.Level1)
{
    UserMemcg* memcg = new UserMemcg(userId_);
    memcg->CreateMemcgDir();
    int score = 0;
    unsigned int mem2zramRatio = 0;
    unsigned int zram2ufsRatio = 0;
    unsigned int refaultThreshold = 0;
    bool ret = memcg->ReadScoreAndReclaimRatiosFromKernel_(score, mem2zramRatio, zram2ufsRatio, refaultThreshold);
    EXPECT_EQ(ret, true);
    memcg->RemoveMemcgDir();
    delete memcg;
    memcg = nullptr;
}
}
}
