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

#include "kernel_interface.h"
#include "memmgr_log.h"

#include "directory_ex.h"
#include "file_ex.h"

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <fstream>

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "KernelInterface";
}

IMPLEMENT_SINGLE_INSTANCE(KernelInterface);

const std::string KernelInterface::MEMCG_BASE_PATH = "/dev/memcg";

bool KernelInterface::EchoToPath(const char* path, const char* content)
{
    uint32_t fd = open(path, O_RDWR, 0666);
    if (fd == -1) {
        HILOGE("echo %{public}s > %{public}s failed: file is not open", content, path);
        return false;
    }
    if (write(fd, content, strlen(content)) < 0) {
        HILOGE("echo %{public}s > %{public}s failed: write failed", content, path);
        close(fd);
        return false;
    }
    close(fd);
    HILOGI("echo %{public}s > %{public}s", content, path);
    return true;
}

bool KernelInterface::IsFileExists(const std::string& path)
{
    if (path.empty()) {
        return false;
    }
    struct stat st = {};
    if (stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
        return true;
    }
    return false;
}

bool KernelInterface::CreateFile(const std::string& path, const mode_t& mode)
{
    if (path.empty()) {
        return false;
    }
    if (IsFileExists(path)) {
        return true;
    }

    std::ofstream fout(path);
    if (!fout.is_open()) {
        return false;
    }
    fout.flush();
    fout.close();
    if (chmod(path.c_str(), mode) != 0) {
        return false;
    }

    return true;
}

bool KernelInterface::CreateFile(const std::string& path)
{
    return CreateFile(path, FILE_MODE_644);
}

bool KernelInterface::RemoveFile(const std::string& path)
{
    return OHOS::RemoveFile(path);
}

bool KernelInterface::WriteToFile(const std::string& path, const std::string& content, bool truncated)
{
    return OHOS::SaveStringToFile(path, content, truncated);
}

bool KernelInterface::WriteLinesToFile(const std::string& path, const std::vector<std::string>& lines, bool truncated)
{
    if (truncated) {
        RemoveFile(path); // clear file content
    }
    for (std::string line : lines) {
        if (!SaveStringToFile(path, line + "\n", false)) {
            return false;
        }
    }
    return true;
}

bool KernelInterface::ReadFromFile(const std::string& path, std::string& content)
{
    return OHOS::LoadStringFromFile(path, content);
}

bool KernelInterface::ReadLinesFromFile(const std::string& path, std::vector<std::string>& lines)
{
    if (!IsFileExists(path)) {
        HILOGE("no such file: %{public}s", path.c_str());
        return false;
    }
    std::string line;
    std::ifstream inf(path, std::ifstream::in);
    if (!inf) {
        HILOGE("ifstream(%{public}s) failed", path.c_str());
        return false;
    }
    lines.clear();
    while (!inf.eof()) {
        getline(inf, line);
        lines.push_back(line);
    }
    inf.close();
    return true;
}

bool KernelInterface::IsDirExists(const std::string& path)
{
    if (path.empty()) {
        return false;
    }
    struct stat st = {};
    if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    return false;
}

bool KernelInterface::IsExists(const std::string& path)
{
    return OHOS::FileExists(path);
}

bool KernelInterface::IsEmptyDir(const std::string& path)
{
    return OHOS::IsEmptyFolder(path);
}

bool KernelInterface::CreateDir(const std::string& path)
{
    return OHOS::ForceCreateDirectory(path); // default mode 755
}

bool KernelInterface::CreateDir(const std::string& path, const mode_t& mode)
{
    if (path.empty()) {
        return false;
    }
    std::string::size_type index = 0;
    do {
        index = path.find('/', index + 1);
        std::string subPath = (index == std::string::npos) ? path : path.substr(0, index);
        if (access(subPath.c_str(), F_OK) != 0) {
            if (mkdir(subPath.c_str(), mode) != 0) {
                return false;
            }
        }
    } while (index != std::string::npos);
    return access(path.c_str(), F_OK) == 0;
}

bool KernelInterface::RemoveDirRecursively(const std::string& path)
{
    return OHOS::ForceRemoveDirectory(path);
}

bool KernelInterface::RemoveItemsInDir(const std::string& dirPath)
{
    if (!IsDirExists(dirPath)) {
        return false;
    }
    if (IsEmptyFolder(dirPath)) {
        return true;
    }
    DIR* dir = NULL;
    struct dirent* dirp = NULL;
    struct stat buf = {};
    if ((dir = opendir(dirPath.c_str())) == NULL) {
        HILOGE("opendir failed: %{public}s", dirPath.c_str());
        return false;
    }
    while (true) {
        dirp = readdir(dir);
        if (dirp == NULL) {
            break;
        }
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) {
            continue;
        }
        std::string fullPath = JoinPath(std::string(dirPath), std::string(dirp->d_name));
        if (stat(fullPath.c_str(), &buf) == -1) {
            HILOGE("stat file failed: %{public}s", fullPath.c_str());
            closedir(dir);
            return false;
        }
        if (S_ISDIR(buf.st_mode)) { // sub dir
            RemoveDirRecursively(fullPath);
            continue;
        }
        if (!RemoveFile(fullPath)) { // rm file
            HILOGE("remove file failed: %{public}s", fullPath.c_str());
            closedir(dir);
            return false;
        }
    }
    closedir(dir);
    return true;
}

std::string KernelInterface::RmDelimiter(const std::string& path)
{
    return OHOS::ExcludeTrailingPathDelimiter(path);
}

std::string KernelInterface::AddDelimiter(const std::string& path)
{
    return OHOS::IncludeTrailingPathDelimiter(path);
}

std::string KernelInterface::JoinPath(const std::string& prefixPath, const std::string& subPath)
{
    return AddDelimiter(prefixPath) + subPath;
}

std::string KernelInterface::JoinPath(const std::string& prefixPath, const std::string& midPath,
                                      const std::string& subPath)
{
    return JoinPath(JoinPath(prefixPath, midPath), subPath);
}
} // namespace Memory
} // namespace OHOS
