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
#include "system_memory_level_config.h"
#include "memmgr_config_manager.h"
#include "xml_helper.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

namespace {
    const std::string TAG = "MemmgrConfigManager";
    const std::string XML_PATH = "/etc/memmgr/";
    const std::string MEMCG_PATH = KernelInterface::MEMCG_BASE_PATH;
} // namespace

class SystemMemoryLevelConfigTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void SystemMemoryLevelConfigTest::SetUpTestCase()
{
}

void SystemMemoryLevelConfigTest::TearDownTestCase()
{
}

void SystemMemoryLevelConfigTest::SetUp()
{
}

void SystemMemoryLevelConfigTest::TearDown()
{
}

/**
 * @tc.name: SetModerate and GetModerate
 * @tc.desc: Test set value of moderate_ equals to parameter
 * @tc.desc: Test get value of moderate_ equals to parameter
 * @tc.type: FUNC
 */
HWTEST_F(SystemMemoryLevelConfigTest, SetModerate, TestSize.Level1)
{
    unsigned int moderate = 0;
    SystemMemoryLevelConfig setMod;
    setMod.SetModerate(moderate);
    EXPECT_EQ(setMod.GetModerate(), moderate);
}

/**
 * @tc.name: SetLow and GetLow
 * @tc.desc: Test set value of low_ equals to parameter
 * @tc.desc: Test get value of low_ equals to parameter
 * @tc.type: FUNC
 */
HWTEST_F(SystemMemoryLevelConfigTest, SetLow, TestSize.Level1)
{
    unsigned int low = 0;
    SystemMemoryLevelConfig setLow;
    setLow.SetLow(low);
    EXPECT_EQ(setLow.GetLow(), low);
}

/**
 * @tc.name: SetCritical and GetCritical
 * @tc.desc: Test set value of critical_ equals to parameter
 * @tc.desc: Test get value of moderate_ and low_ and critical_
 * @tc.type: FUNC
 */
HWTEST_F(SystemMemoryLevelConfigTest, SetCritical, TestSize.Level1)
{
    unsigned int critical = 0;
    SystemMemoryLevelConfig setCri;
    setCri.SetCritical(critical);
    EXPECT_EQ(setCri.GetCritical(), critical);
}

/**
 * @tc.name: ParseConfig
 * @tc.desc: Test the branch of if
 * @tc.desc: Test set value of critical_ equals to parameter
 * @tc.type: FUNC
 */
HWTEST_F(SystemMemoryLevelConfigTest, ParseConfig1, TestSize.Level1)
{
    const xmlNodePtr rootNodePtr = nullptr;
    SystemMemoryLevelConfig parCon;
    EXPECT_EQ(XmlHelper::CheckNode(rootNodePtr), false);
    EXPECT_EQ(XmlHelper::HasChild(rootNodePtr), false);
    parCon.ParseConfig(rootNodePtr);
}

/**
 * @tc.name: ParseConfig
 * @tc.desc: Test set value of critical_ equals to parameter
 * @tc.type: FUNC
 */
HWTEST_F(SystemMemoryLevelConfigTest, ParseConfig2, TestSize.Level1)
{
    std::string path = KernelInterface::GetInstance().JoinPath(XML_PATH, "memmgr_config.xml");
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(path.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    const xmlNodePtr rootNodePtr = xmlDocGetRootElement(docPtr.get());
    std::map<std::string, std::string> param;
    SystemMemoryLevelConfig parCon;
    EXPECT_EQ(XmlHelper::CheckNode(rootNodePtr), true);
    EXPECT_EQ(XmlHelper::HasChild(rootNodePtr), true);
    EXPECT_EQ(XmlHelper::CheckNode(rootNodePtr->xmlChildrenNode), true);
    parCon.ParseConfig(rootNodePtr);
    EXPECT_NE(parCon.GetModerate(), 0);
    EXPECT_NE(parCon.GetLow(), 0);
    EXPECT_NE(parCon.GetCritical(), 0);
}

}
}

