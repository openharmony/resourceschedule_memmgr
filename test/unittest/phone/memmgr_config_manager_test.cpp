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

class MemmgrConfigManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MemmgrConfigManagerTest::SetUpTestCase()
{
}

void MemmgrConfigManagerTest::TearDownTestCase()
{
}

void MemmgrConfigManagerTest::SetUp()
{
}

void MemmgrConfigManagerTest::TearDown()
{
}

HWTEST_F(MemmgrConfigManagerTest, initTest, TestSize.Level1)
{
    MemmgrConfigManager::GetInstance().Init();
}
}
}
