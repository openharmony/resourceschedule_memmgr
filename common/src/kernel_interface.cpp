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

#include <sstream>
#include <csignal>

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "KernelInterface";
}

IMPLEMENT_SINGLE_INSTANCE(KernelInterface);

const std::string KernelInterface::MEMCG_BASE_PATH = "/dev/memcg";
const std::string KernelInterface::CURRENT_BUFFER_PATH = "/dev/memcg/memory.zswapd_pressure_show";

bool KernelInterface::EchoToPath(const char* path, const char* content)
{
    int fd = open(path, O_RDWR, 0666);
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

bool KernelInterface::RemoveDirRecursively(const std::string& path)
{
    return OHOS::ForceRemoveDirectory(path) || (remove(path.c_str()) == 0);
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
            continue;
        }
        if (S_ISDIR(buf.st_mode)) { // sub dir
            if (RemoveDirRecursively(fullPath) == false) {
                remove(fullPath.c_str());
            }
            continue;
        }
        if (!RemoveFile(fullPath)) { // rm file
            HILOGE("remove file failed: %{public}s", fullPath.c_str());
            continue;
        }
    }
    closedir(dir);
    return IsEmptyFolder(dirPath);
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

bool KernelInterface::GetPidProcInfo(struct ProcInfo &procInfo)
{
    HILOGD("called!");

    std::string statPath = JoinPath("/proc/", std::to_string(procInfo.pid), "/stat");
    HILOGD("statPath=%{public}s", statPath.c_str());

    // format like:
    // 1 (init) S 0 0 0 0 -1 4210944 1 ...
    std::string stat, statm, statPid, vss, rss;
    if (!ReadFromFile(statPath, stat)) {
        return false;
    }
    HILOGD("stat = [%{public}s]", stat.c_str());
    std::istringstream isStat(stat);
    isStat >> statPid >> procInfo.name >> procInfo.status;
    HILOGD("pid=[%{public}d], name=[%{public}s], status=[%{public}s]",
           procInfo.pid, procInfo.name.c_str(), procInfo.status.c_str());

    if (statPid != std::to_string(procInfo.pid)) {
        return false;
    }

    std::string statmPath = JoinPath("/proc/", std::to_string(procInfo.pid), "/statm");
    HILOGD("statmPath=%{public}s", statmPath.c_str());
    // format like:
    // 640 472 369 38 0 115 0
    if (!ReadFromFile(statmPath, statm)) {
        return false;
    }
    HILOGD("statm = [%{public}s]", statm.c_str());
    std::istringstream isStatm(statm);
    isStatm >> vss >> rss; // pages
    HILOGD("vss=[%{public}s], rss=[%{public}s]", vss.c_str(), rss.c_str());

    procInfo.size = atoi(rss.c_str()) * PAGE_TO_KB;
    HILOGI("GetProcInfo success: name is %{public}s, status is %{public}s, size = %{public}d",
           procInfo.name.c_str(), procInfo.status.c_str(), procInfo.size);
    return true;
}

int KernelInterface::GetCurrentBuffer()
{
    HILOGD("called!");
    std::string content, buffer, size, buffer_;

    if (!ReadFromFile(CURRENT_BUFFER_PATH, content)) {
        HILOGE("read %{public}s failed, return max value", CURRENT_BUFFER_PATH.c_str());
        return MAX_BUFFER_KB;
    }
    HILOGI("read %{public}s success, content=[%{public}s]", CURRENT_BUFFER_PATH.c_str(), content.c_str());

    std::istringstream is(content);
    is >> buffer >> size >> buffer_;

    HILOGI("GetCurrentBuffer success: %{public}s MB", buffer_.c_str());
    return atoi(buffer_.c_str()) * KB_PER_MB;
}

int KernelInterface::KillOneProcessByPid(int pid)
{
    HILOGD("called! pid=%{public}d", pid);
    struct ProcInfo procInfo;
    int freedBuffer = 0;
    procInfo.pid = pid;

    if (!GetPidProcInfo(procInfo)) {
        HILOGE("GetPidProcInfo fail !!!");
        goto out;
    }

    if (procInfo.status == "D") {
        HILOGE("Task %{public}s is at D status!", procInfo.name.c_str());
        goto out;
    }

    if (kill(pid, SIGKILL)) {
        HILOGE("Kill %{public}s errno=%{public}d!", procInfo.name.c_str(), errno);
    }
    HILOGE("%{public}s has been Killed ! (pid=%{public}d, freedSize=%{public}d KB)",
        procInfo.name.c_str(), procInfo.pid, procInfo.size);

    freedBuffer = procInfo.size;
out:
    return freedBuffer;
}
} // namespace Memory
} // namespace OHOS
