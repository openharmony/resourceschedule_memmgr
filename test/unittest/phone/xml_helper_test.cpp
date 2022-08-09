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
#include "xml_helper.h"
#undef private
#undef protected

namespace OHOS {
namespace Memory {
using namespace testing;
using namespace testing::ext;

class XmlHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void XmlHelperTest::SetUpTestCase()
{
}

void XmlHelperTest::TearDownTestCase()
{
}

void XmlHelperTest::SetUp()
{
}

void XmlHelperTest::TearDown()
{
}

HWTEST_F(XmlHelperTest, setIntParamTest1, TestSize.Level1)
{
    std::map<std::string, std::string> param;
    param.insert(std::make_pair("key", "2147483647"));
    std::string key = "key";
    int dst;
    XmlHelper::SetIntParam(param, key, dst, 0);
    EXPECT_EQ(dst, 2147483647);
}

HWTEST_F(XmlHelperTest, setIntParamTest2, TestSize.Level1)
{
    std::map<std::string, std::string> param;
    param.insert(std::make_pair("key", "."));
    std::string key = "key";
    int dst;
    XmlHelper::SetIntParam(param, key, dst, 0);
    EXPECT_EQ(dst, 0);
}

HWTEST_F(XmlHelperTest, setIntParamTest3, TestSize.Level1)
{
    std::map<std::string, std::string> param;
    param.insert(std::make_pair("key", " "));
    std::string key = "key";
    int dst;
    XmlHelper::SetIntParam(param, key, dst, 0);
    EXPECT_EQ(dst, 0);
}

HWTEST_F(XmlHelperTest, setIntParamTest4, TestSize.Level1)
{
    std::map<std::string, std::string> param;
    param.insert(std::make_pair("key", "2147483648"));
    std::string key = "key";
    int dst;
    XmlHelper::SetIntParam(param, key, dst, 0);
    EXPECT_EQ(dst, 0);
}
}
}
