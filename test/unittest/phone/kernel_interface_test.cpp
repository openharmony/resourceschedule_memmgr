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

HWTEST_F(MemMgrTest, EchoToPath_InvalidPath, TestSize.Level1)
{
    bool ret = KernelInterface::GetInstance().EchoToPath("", "");
    EXPECT_EQ(ret, false);
}

HWTEST_F(MemMgrTest, GetPidProcInfoTest, TestSize.Level1)
{
    ProcInfo procInfo;
    procInfo.pid = 1;
    bool ret = KernelInterface::GetInstance().GetPidProcInfo(procInfo);
    printf("pid=[%d], name=[%s], status=[%s], size=[%d KB]\n",
           procInfo.pid, procInfo.name.c_str(), procInfo.status.c_str(), procInfo.size);
    EXPECT_EQ(ret, true);
}

HWTEST_F(MemMgrTest, GetCurrentBufferTest, TestSize.Level1)
{
    int buffer = KernelInterface::GetInstance().GetCurrentBuffer();
    printf("buffer=%d", buffer);
    EXPECT_GT(buffer, 0);
}

HWTEST_F(MemMgrTest, KillOneProcessByPidTest, TestSize.Level1)
{
    int pid = -1;
    printf("please input pid to kill\n");
    scanf("%d", &pid);
    ProcInfo procInfo;
    procInfo.pid = pid;
    bool ret = KernelInterface::GetInstance().GetPidProcInfo(procInfo);
    EXPECT_EQ(ret, true);

    int killedSize = KernelInterface::GetInstance().KillOneProcessByPid(pid);
    printf("killedSize=%d", killedSize);
    EXPECT_EQ(killedSize, procInfo.size);
}
}
}
