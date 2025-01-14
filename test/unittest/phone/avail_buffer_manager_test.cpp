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
#include "avail_buffer_manager.h"
#include "memmgr_config_manager.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class AvailBufferManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AvailBufferManagerTest::SetUpTestCase()
{
}

void AvailBufferManagerTest::TearDownTestCase()
{
}

void AvailBufferManagerTest::SetUp()
{
}

void AvailBufferManagerTest::TearDown()
{
}

HWTEST_F(AvailBufferManagerTest, InitTest, TestSize.Level1)
{
    MemmgrConfigManager::GetInstance().Init();
    EXPECT_EQ(AvailBufferManager::GetInstance().Init(), true);
}

HWTEST_F(AvailBufferManagerTest, WriteAvailBufferToKernelTest, TestSize.Level1)
{
    EXPECT_EQ(AvailBufferManager::GetInstance().WriteAvailBufferToKernel(), true);
}

HWTEST_F(AvailBufferManagerTest, SetAvailBufferTest, TestSize.Level1)
{
    unsigned int availBuffer = 123;
    unsigned int minAvailBuffer = 100;
    unsigned int highAvailBuffer = 666;
    unsigned int swapReserve = 888;
    std::shared_ptr<AvailBufferConfig> para = std::make_shared<AvailBufferConfig>(availBuffer, minAvailBuffer,
            highAvailBuffer, swapReserve);
    EXPECT_EQ(AvailBufferManager::GetInstance().SetAvailBuffer(para), true);
}

HWTEST_F(AvailBufferManagerTest, LoadAvailBufferFromConfigTest, TestSize.Level1)
{
    EXPECT_EQ(AvailBufferManager::GetInstance().LoadAvailBufferFromConfig(), true);
}

HWTEST_F(AvailBufferManagerTest, CloseZswapdTest, TestSize.Level1)
{
    AvailBufferManager::GetInstance().CloseZswapd();
    EXPECT_EQ(AvailBufferManager::GetInstance().availBuffer_, 0);
    EXPECT_EQ(AvailBufferManager::GetInstance().minAvailBuffer_, 0);
    EXPECT_EQ(AvailBufferManager::GetInstance().highAvailBuffer_, 0);
    EXPECT_EQ(AvailBufferManager::GetInstance().swapReserve_, 0);
}

HWTEST_F(AvailBufferManagerTest, InitAvailBufferTest, TestSize.Level1)
{
    AvailBufferManager::GetInstance().zramEnable_ = false;
    EXPECT_EQ(AvailBufferManager::GetInstance().zramEnable_, false);
    AvailBufferManager::GetInstance().InitAvailBuffer();
    EXPECT_EQ(AvailBufferManager::GetInstance().zramEnable_, true);
}

HWTEST_F(AvailBufferManagerTest, GetEventHandlerTest, TestSize.Level1)
{
    EXPECT_EQ(AvailBufferManager::GetInstance().GetEventHandler(), true);
}

HWTEST_F(AvailBufferManagerTest, UpdateZramEnableFromKernelTest, TestSize.Level1)
{
    AvailBufferManager::GetInstance().zramEnable_ = false;
    EXPECT_EQ(AvailBufferManager::GetInstance().zramEnable_, false);
    AvailBufferManager::GetInstance().UpdateZramEnableFromKernel();
    EXPECT_EQ(AvailBufferManager::GetInstance().zramEnable_, true);
}

HWTEST_F(AvailBufferManagerTest, CheckAvailBufferTest, TestSize.Level1)
{
    unsigned int availBuffer = 123;
    unsigned int minAvailBuffer = 100;
    unsigned int highAvailBuffer = 666;
    unsigned int swapReserve = 888;
    std::shared_ptr<AvailBufferConfig> para = std::make_shared<AvailBufferConfig>(availBuffer, minAvailBuffer,
            highAvailBuffer, swapReserve);
    EXPECT_EQ(AvailBufferManager::GetInstance().CheckAvailBuffer(para), true);
}

HWTEST_F(AvailBufferManagerTest, UpdateMemTotalFromKernelTest, TestSize.Level1)
{
    AvailBufferManager::GetInstance().memTotal_ = 0;
    EXPECT_EQ(AvailBufferManager::GetInstance().memTotal_, 0);
    AvailBufferManager::GetInstance().UpdateMemTotalFromKernel();
    EXPECT_NE(AvailBufferManager::GetInstance().memTotal_, 0);
}
}
}
