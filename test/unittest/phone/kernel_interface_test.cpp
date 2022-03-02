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

class KernelInterfaceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void KernelInterfaceTest::SetUpTestCase()
{
}

void KernelInterfaceTest::TearDownTestCase()
{
}

void KernelInterfaceTest::SetUp()
{
}

void KernelInterfaceTest::TearDown()
{
}

HWTEST_F(KernelInterfaceTest, EchoToPath_InvalidPath, TestSize.Level1)
{
    bool ret = KernelInterface::GetInstance().EchoToPath("", "");
    EXPECT_EQ(ret, false);
}

HWTEST_F(KernelInterfaceTest, PathDelimiterAddAndRmTest, TestSize.Level1)
{
    std::string base = "/test/tmp";
    // add delimiter test
    std::string pathWithDeli = KernelInterface::GetInstance().AddDelimiter(base);
    EXPECT_EQ(pathWithDeli.compare(base + "/"), 0);

    std::string pathTmp = KernelInterface::GetInstance().AddDelimiter("");
    EXPECT_EQ(pathTmp.length(), 0);

    // rm delimiter test
    pathTmp = KernelInterface::GetInstance().RmDelimiter(pathWithDeli);
    EXPECT_EQ(pathTmp.compare(base), 0);

    pathTmp = KernelInterface::GetInstance().RmDelimiter("");
    EXPECT_EQ(pathTmp.length(), 0);

    pathTmp = KernelInterface::GetInstance().RmDelimiter(base);
    EXPECT_EQ(pathTmp.compare(base), 0);
}

HWTEST_F(KernelInterfaceTest, JoinPathTest, TestSize.Level1)
{
    std::string first = "/test/tmp";
    std::string second = "2";
    std::string third = "3";
    std::string empty = "";
    // JoinPath with 2 param test
    std::string path = KernelInterface::GetInstance().JoinPath(first, second);
    EXPECT_EQ(path.compare(first + "/" + second), 0);
    path = KernelInterface::GetInstance().JoinPath(empty, second);
    EXPECT_EQ(path.compare(second), 0);
    path = KernelInterface::GetInstance().JoinPath(first, empty);
    EXPECT_EQ(path.compare(first + "/"), 0);
    path = KernelInterface::GetInstance().JoinPath(empty, empty);
    EXPECT_EQ(path.compare(empty), 0);

    // JoinPath with 3 param test
    path = KernelInterface::GetInstance().JoinPath(first, second, third);
    EXPECT_EQ(path.compare(first + "/" + second + "/" + third), 0);
    path = KernelInterface::GetInstance().JoinPath(empty, second, third);
    EXPECT_EQ(path.compare(second + "/" + third), 0);
    path = KernelInterface::GetInstance().JoinPath(first, empty, third);
    EXPECT_EQ(path.compare(first + "/" + third), 0);
    path = KernelInterface::GetInstance().JoinPath(first, second, empty);
    EXPECT_EQ(path.compare(first + "/" + second + "/"), 0);

    path = KernelInterface::GetInstance().JoinPath(empty, empty, third);
    EXPECT_EQ(path.compare(third), 0);
    path = KernelInterface::GetInstance().JoinPath(empty, second, empty);
    EXPECT_EQ(path.compare(second + "/"), 0);
    path = KernelInterface::GetInstance().JoinPath(first, empty, empty);
    EXPECT_EQ(path.compare(first + "/"), 0);

    path = KernelInterface::GetInstance().JoinPath(empty, empty, empty);
    EXPECT_EQ(path.compare(empty), 0);
}

HWTEST_F(KernelInterfaceTest, FileCreateTest, TestSize.Level1)
{
    const std::string BASE_PATH = "/data/local/tmp";
    bool ret = KernelInterface::GetInstance().IsDirExists(BASE_PATH);
    EXPECT_EQ(ret, true);

    std::string path = KernelInterface::GetInstance().JoinPath(BASE_PATH, "testFile");
    ret = KernelInterface::GetInstance().CreateFile(path);
    EXPECT_EQ(ret, true);
    ret = KernelInterface::GetInstance().IsFileExists(path);
    EXPECT_EQ(ret, true);
    KernelInterface::GetInstance().RemoveFile(path);
}

HWTEST_F(KernelInterfaceTest, FileRemoveTest, TestSize.Level1)
{
    const std::string BASE_PATH = "/data/local/tmp";
    std::string path = KernelInterface::GetInstance().JoinPath(BASE_PATH, "testFile");
    KernelInterface::GetInstance().RemoveFile(path);
    bool ret = KernelInterface::GetInstance().CreateFile(path);
    ret = ret && KernelInterface::GetInstance().IsFileExists(path);
    ret = ret && KernelInterface::GetInstance().RemoveFile(path);
    EXPECT_EQ(ret, true);
    ret = KernelInterface::GetInstance().IsFileExists(path);
    EXPECT_EQ(ret, false);
}

HWTEST_F(KernelInterfaceTest, SingleLineFileReadWriteTest, TestSize.Level1)
{
    const std::string BASE_PATH = "/data/local/tmp";
    std::string path = KernelInterface::GetInstance().JoinPath(BASE_PATH, "testFile");
    KernelInterface::GetInstance().RemoveFile(path);
    bool ret = KernelInterface::GetInstance().CreateFile(path);
    ret = ret && KernelInterface::GetInstance().IsFileExists(path);
    EXPECT_EQ(ret, true);
    std::string line1 = "line1";
    std::string output;

    ret = KernelInterface::GetInstance().WriteToFile(path, line1); // curr content: "line1"
    ret = ret && KernelInterface::GetInstance().ReadFromFile(path, output);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(output.compare(line1), 0);

    ret = KernelInterface::GetInstance().WriteToFile(path, line1, false); // curr content: "line1line1"
    ret = ret && KernelInterface::GetInstance().ReadFromFile(path, output);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(output.compare(line1 + line1), 0);
    KernelInterface::GetInstance().RemoveFile(path);
}

HWTEST_F(KernelInterfaceTest, SingleLineFileReadWriteLinesTest, TestSize.Level1)
{
    const std::string BASE_PATH = "/data/local/tmp";
    std::string path = KernelInterface::GetInstance().JoinPath(BASE_PATH, "testFile");
    KernelInterface::GetInstance().RemoveFile(path);
    bool ret = KernelInterface::GetInstance().CreateFile(path);
    ret = ret && KernelInterface::GetInstance().IsFileExists(path);
    EXPECT_EQ(ret, true);
    std::string line1 = "line1";
    std::vector<std::string> inputLines = {line1};
    std::string output;
    std::vector<std::string> outputLines;

    ret = KernelInterface::GetInstance().WriteLinesToFile(path, inputLines); // curr content: "line1\n"
    ret = ret && KernelInterface::GetInstance().ReadLinesFromFile(path, outputLines);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(outputLines.size(), 2);
    EXPECT_EQ(outputLines[0].compare(line1), 0);
    EXPECT_EQ(outputLines[1].length(), 0);
    KernelInterface::GetInstance().RemoveFile(path);
}

HWTEST_F(KernelInterfaceTest, MultLinesFileReadWriteTest, TestSize.Level1)
{
    const std::string BASE_PATH = "/data/local/tmp";
    std::string path = KernelInterface::GetInstance().JoinPath(BASE_PATH, "testFile");
    KernelInterface::GetInstance().RemoveFile(path);
    bool ret = KernelInterface::GetInstance().CreateFile(path);
    ret = ret && KernelInterface::GetInstance().IsFileExists(path);
    EXPECT_EQ(ret, true);
    std::string line1 = "line1";
    std::string line2 = "line2";
    std::string line3 = "line3";
    std::string lines = line1 + "\n" + line2 + "\n" + line3; // "line1\nline2\nline3"
    std::string output;

    ret = KernelInterface::GetInstance().WriteToFile(path, line1); // curr content: "line1"
    ret = ret && KernelInterface::GetInstance().WriteToFile(path, "\n", false); // curr content: "line1\n"
    ret = ret && KernelInterface::GetInstance().WriteToFile(path, line2, false); // curr content: "line1\nline2"
    ret = ret && KernelInterface::GetInstance().ReadFromFile(path, output);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(output.compare(line1 + "\n" + line2), 0);

    ret = KernelInterface::GetInstance().WriteToFile(path, lines); // curr content: "line1\nline2\nline3"
    ret = ret && KernelInterface::GetInstance().ReadFromFile(path, output);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(output.compare(lines), 0);
    KernelInterface::GetInstance().RemoveFile(path);
}

HWTEST_F(KernelInterfaceTest, MultLinesFileReadWriteLinesTest, TestSize.Level1)
{
    const std::string BASE_PATH = "/data/local/tmp";
    std::string path = KernelInterface::GetInstance().JoinPath(BASE_PATH, "testFile");
    KernelInterface::GetInstance().RemoveFile(path);
    bool ret = KernelInterface::GetInstance().CreateFile(path);
    ret = ret && KernelInterface::GetInstance().IsFileExists(path);
    EXPECT_EQ(ret, true);
    std::string line1 = "line1";
    std::string line2 = "line2";
    std::string line3 = "line3";
    std::vector<std::string> outputLines;
    std::vector<std::string> inputLines = {line1, line2, line3};

    ret = KernelInterface::GetInstance().WriteLinesToFile(path, inputLines); // curr content: "line1\nline2\nline3\n"
    ret = ret && KernelInterface::GetInstance().ReadLinesFromFile(path, outputLines);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(outputLines.size(), 4);
    EXPECT_EQ(outputLines[0].compare(line1), 0);
    EXPECT_EQ(outputLines[1].compare(line2), 0);
    EXPECT_EQ(outputLines[2].compare(line3), 0);
    EXPECT_EQ(outputLines[3].length(), 0);
    KernelInterface::GetInstance().RemoveFile(path);
}

HWTEST_F(KernelInterfaceTest, NoSuchFileCreateTest, TestSize.Level1)
{
    std::string basedir = "/NoSuchDir";
    std::string path = "/NoSuchDir/file";
    KernelInterface::GetInstance().RemoveDirRecursively(basedir);
    bool ret = KernelInterface::GetInstance().CreateFile("");
    ret = ret || KernelInterface::GetInstance().CreateFile(path);
    ret = ret || KernelInterface::GetInstance().IsFileExists(path);
    EXPECT_EQ(ret, false);
}

HWTEST_F(KernelInterfaceTest, NoSuchFileRemoveTest, TestSize.Level1)
{
    std::string basedir = "/NoSuchDir";
    std::string path1 = "/NoSuchDir/file";
    std::string path2 = "/proc/NoSuchFile";
    KernelInterface::GetInstance().RemoveDirRecursively(basedir);
    KernelInterface::GetInstance().RemoveFile(path2);
    bool ret = KernelInterface::GetInstance().RemoveFile(path1);
    ret = ret && KernelInterface::GetInstance().RemoveFile(path2);
    EXPECT_EQ(ret, true);
}

HWTEST_F(KernelInterfaceTest, NoSuchFileReadWriteTest, TestSize.Level1)
{
    std::string path = "/NoSuchDir/NoSuchFile";
    KernelInterface::GetInstance().RemoveFile(path);
    std::string line1 = "line1";
    std::string line2 = "line2";
    std::string line3 = "line3";
    std::string output;
    std::vector<std::string> outputLines;
    std::vector<std::string> inputLines = {line1, line2, line3};

    bool ret = KernelInterface::GetInstance().WriteToFile(path, line1, false);
    ret = ret || KernelInterface::GetInstance().WriteToFile(path, line1);
    ret = ret || KernelInterface::GetInstance().WriteToFile("", line1);
    ret = ret || KernelInterface::GetInstance().ReadFromFile(path, output);
    ret = ret || KernelInterface::GetInstance().ReadFromFile("", output);
    EXPECT_EQ(ret, false);

    ret = KernelInterface::GetInstance().WriteLinesToFile(path, inputLines);
    ret = ret || KernelInterface::GetInstance().ReadLinesFromFile(path, outputLines);
    ret = ret || KernelInterface::GetInstance().WriteLinesToFile("", inputLines);
    ret = ret || KernelInterface::GetInstance().ReadLinesFromFile("", outputLines);
    EXPECT_EQ(ret, false);
}

HWTEST_F(KernelInterfaceTest, DirCreateTest, TestSize.Level1)
{
    const std::string BASE_PATH = "/data/local/tmp";
    bool ret = KernelInterface::GetInstance().IsDirExists(BASE_PATH);
    std::string path = KernelInterface::GetInstance().JoinPath(BASE_PATH, "testDir");
    KernelInterface::GetInstance().RemoveDirRecursively(path);
    ret = ret && KernelInterface::GetInstance().CreateDir(path);
    ret = ret && KernelInterface::GetInstance().IsDirExists(path);
    EXPECT_EQ(ret, true);
    KernelInterface::GetInstance().RemoveDirRecursively(path);

    path = KernelInterface::GetInstance().JoinPath(path, "midDir", "subDir");
    ret = ret && KernelInterface::GetInstance().CreateDir(path);
    ret = ret && KernelInterface::GetInstance().IsDirExists(path);
    EXPECT_EQ(ret, true);
    KernelInterface::GetInstance().RemoveDirRecursively(path);
}

HWTEST_F(KernelInterfaceTest, DirRemoveTest, TestSize.Level1)
{
    const std::string BASE_PATH = "/data/local/tmp";
    bool ret = KernelInterface::GetInstance().IsDirExists(BASE_PATH);
    std::string basedir = KernelInterface::GetInstance().JoinPath(BASE_PATH, "testDir");
    std::string subDir1 = KernelInterface::GetInstance().JoinPath(basedir, "subDir1");
    std::string subDir2 = KernelInterface::GetInstance().JoinPath(basedir, "subDir2");
    std::string file1 = KernelInterface::GetInstance().JoinPath(subDir1, "testFile1");
    std::string file2 = KernelInterface::GetInstance().JoinPath(subDir2, "testFile2");
    KernelInterface::GetInstance().CreateDir(subDir1); // ../testDir/subDir1/
    KernelInterface::GetInstance().CreateFile(file1); // ../testDir/subDir1/testFile1

    ret = KernelInterface::GetInstance().RemoveDirRecursively(subDir1);
    ret = ret && KernelInterface::GetInstance().IsEmptyDir(basedir);
    EXPECT_EQ(ret, true);
    ret = KernelInterface::GetInstance().IsDirExists(subDir1);
    EXPECT_EQ(ret, false);

    KernelInterface::GetInstance().CreateDir(subDir1);
    KernelInterface::GetInstance().CreateDir(subDir2); // ../testDir/subDir2/
    KernelInterface::GetInstance().CreateFile(file1); // ../testDir/subDir1/testFile1
    KernelInterface::GetInstance().CreateFile(file2); // ../testDir/subDir2/testFile2
    ret = KernelInterface::GetInstance().RemoveDirRecursively(basedir);
    EXPECT_EQ(ret, true);
    ret = KernelInterface::GetInstance().IsDirExists(basedir);
    EXPECT_EQ(ret, false);
}

HWTEST_F(KernelInterfaceTest, RemoveItemsInDirTest, TestSize.Level1)
{
    const std::string BASE_PATH = "/data/local/tmp";
    bool ret = KernelInterface::GetInstance().IsDirExists(BASE_PATH);
    std::string basedir = KernelInterface::GetInstance().JoinPath(BASE_PATH, "testDir");
    std::string subDir1 = KernelInterface::GetInstance().JoinPath(basedir, "subDir1");
    std::string subDir2 = KernelInterface::GetInstance().JoinPath(basedir, "subDir2");
    std::string file1 = KernelInterface::GetInstance().JoinPath(subDir1, "testFile1");
    std::string file2 = KernelInterface::GetInstance().JoinPath(subDir2, "testFile2");
    KernelInterface::GetInstance().CreateDir(subDir1); // ../testDir/subDir1/
    KernelInterface::GetInstance().CreateDir(subDir2); // ../testDir/subDir2/
    KernelInterface::GetInstance().CreateFile(file1); // ../testDir/subDir1/testFile1
    KernelInterface::GetInstance().CreateFile(file2); // ../testDir/subDir2/testFile2

    ret = KernelInterface::GetInstance().RemoveItemsInDir(basedir);
    ret = ret && KernelInterface::GetInstance().IsEmptyDir(basedir);
    EXPECT_EQ(ret, true);
    KernelInterface::GetInstance().RemoveDirRecursively(basedir);
}

HWTEST_F(KernelInterfaceTest, GetPidProcInfoTest, TestSize.Level1)
{
    ProcInfo procInfo;
    procInfo.pid = 1;
    bool ret = KernelInterface::GetInstance().GetPidProcInfo(procInfo);
    printf("pid=[%d], name=[%s], status=[%s], size=[%d KB]\n",
           procInfo.pid, procInfo.name.c_str(), procInfo.status.c_str(), procInfo.size);
    EXPECT_EQ(ret, true);
}

HWTEST_F(KernelInterfaceTest, GetCurrentBufferTest, TestSize.Level1)
{
    int buffer = KernelInterface::GetInstance().GetCurrentBuffer();
    printf("buffer=%d", buffer);
    EXPECT_GT(buffer, 0);
}
}
}
