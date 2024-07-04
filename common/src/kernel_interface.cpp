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

#include <climits>
#include <csignal>
#include <dirent.h>
#include <fstream>
#include <regex>
#include <securec.h>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>

#include "directory_ex.h"
#include "file_ex.h"
#include "memmgr_log.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "KernelInterface";
}

IMPLEMENT_SINGLE_INSTANCE(KernelInterface);

const std::string KernelInterface::ROOT_PROC_PATH = "/proc";
const std::string KernelInterface::MEMCG_BASE_PATH = "/dev/memcg";
const std::string KernelInterface::FILE_MEMCG_PROCS = "cgroup.procs";

#ifdef USE_HYPERHOLD_MEMORY
const std::string KernelInterface::ZWAPD_PRESSURE_SHOW_PATH = "/dev/memcg/memory.zswapd_pressure_show";
const std::string KernelInterface::ZWAPD_PRESSURE_SHOW_BUFFER_SIZE = "buffer_size";
#else
const std::string KernelInterface::ZWAPD_PRESSURE_SHOW_PATH = "/proc/meminfo";
const std::string KernelInterface::ZWAPD_PRESSURE_SHOW_BUFFER_SIZE = "MemAvailable";
#endif
const std::string KernelInterface::MEMINFO_PATH = "/proc/meminfo";
const std::string KernelInterface::FILE_PROC_STATUS = "status";
const std::string KernelInterface::TOTAL_MEMORY = "MemTotal";

bool KernelInterface::EchoToPath(const char* path, const char* content)
{
    int fd = open(path, O_WRONLY);
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
    int fd;
    char actualPath[PATH_MAX + 1] = {0};
    char* ptrRet = NULL;

    if (strlen(path.c_str()) == 0 || strlen(path.c_str()) > PATH_MAX) {
        HILOGE("file path is invalid");
        return false;
    }
    ptrRet = realpath(path.c_str(), actualPath);
    if (!ptrRet) {
        HILOGE("file path cannot be canonicalized");
        return false;
    }
    HILOGD("path:%{public}s, actualPath:%{public}s", path.c_str(), actualPath);
    fd = open(actualPath, O_RDWR | (truncated ? O_TRUNC : O_APPEND));
    if (fd == -1) {
        HILOGE("echo %{public}s %{public}s %{public}s failed: file is not open",
            content.c_str(), truncated ? ">" : ">>", path.c_str());
        ptrRet = NULL;
        return false;
    }
    if (write(fd, content.c_str(), strlen(content.c_str())) < 0) {
        HILOGE("echo %{public}s %{public}s %{public}s failed: write failed",
            content.c_str(), truncated ? ">" : ">>", path.c_str());
        ptrRet = NULL;
        close(fd);
        return false;
    }
    ptrRet = NULL;
    close(fd);
    HILOGD("echo %{public}s %{public}s %{public}s", content.c_str(), truncated ? ">" : ">>", path.c_str());
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

int64_t KernelInterface::GetSystemCurTime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t KernelInterface::GetSystemTimeMs()
{
    return std::chrono::duration_cast<std::chrono::seconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
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

std::string KernelInterface::RmDelimiter(const std::string& path)
{
    if (path.empty()) {
        return path;
    }
    return OHOS::ExcludeTrailingPathDelimiter(path);
}

std::string KernelInterface::AddDelimiter(const std::string& path)
{
    if (path.empty()) {
        return path;
    }
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

    // format like:
    // 1 (init) S 0 0 0 0 -1 4210944 1 ...
    std::string stat;
    std::string statm;
    std::string statPid;
    std::string vss;
    std::string rss;
    if (!ReadFromFile(statPath, stat)) {
        HILOGD("stat file error!");
        return false;
    }
    std::istringstream isStat(stat);
    isStat >> statPid >> procInfo.name >> procInfo.status;

    if (statPid != std::to_string(procInfo.pid)) {
        HILOGD("pid error!");
        return false;
    }

    std::string statmPath = JoinPath("/proc/", std::to_string(procInfo.pid), "/statm");
    // format like:
    // 640 472 369 38 0 115 0
    if (!ReadFromFile(statmPath, statm)) {
        HILOGD("statm file error!");
        return false;
    }
    std::istringstream isStatm(statm);
    isStatm >> vss >> rss; // pages
    int rssValue = 0;
    try {
        rssValue = std::stoi(rss);
    } catch (...) {
        HILOGE("stoi(%{public}s) failed!", rss.c_str());
        return false;
    }

    if (rssValue < 0 || rssValue > INT_MAX / PAGE_TO_KB) {
        HILOGE("rssValue=%{public}d, rss is less than 0 or overflow!", rssValue);
        return false;
    }
    procInfo.size = rssValue * PAGE_TO_KB;
    HILOGI("GetProcInfo success: name is %{public}s, status is %{public}s, size = %{public}d KB",
           procInfo.name.c_str(), procInfo.status.c_str(), procInfo.size);
    return true;
}

bool KernelInterface::GetProcNameByPid(int pid, std::string &name)
{
    std::string statusPath = JoinPath("/proc/", std::to_string(pid), "/status");
    std::string statusContent;
    std::string nameTag;
    if (!ReadFromFile(statusPath, statusContent)) {
        HILOGE("status file [%{public}s] error!", statusPath.c_str());
        return false;
    }
    std::istringstream statusStream(statusContent);
    statusStream >> nameTag >> name;
    return true;
}

void KernelInterface::ReadZswapdPressureShow(std::map<std::string, std::string>& result)
{
    std::string contentStr;
    if (!ReadFromFile(ZWAPD_PRESSURE_SHOW_PATH, contentStr)) {
        HILOGE("read %{public}s faild, content=[%{public}s]", ZWAPD_PRESSURE_SHOW_PATH.c_str(), contentStr.c_str());
        return;
    }
    char *contentPtr = new char[contentStr.size() + 1];
    if (contentPtr == nullptr) {
        HILOGE("alloc buffer fail");
        return;
    }
    if (strcpy_s(contentPtr, contentStr.size() + 1, contentStr.c_str()) != EOK) {
        HILOGE("copy fail");
        delete [] contentPtr;
        return;
    }
    char *restPtr;
    char *line = strtok_r(contentPtr, "\n", &restPtr);
    do {
        for (size_t i = 0; i < strlen(line); i++) {
            if (line[i] == ':') {
                line[i] = ' ';
            }
        }
        std::string lineStr(line);
        std::istringstream is(lineStr);
        std::string name;
        std::string value;
        is >> name >> value;
        result.insert(std::make_pair(name, value));

        line = strtok_r(NULL, "\n", &restPtr);
    } while (line);
    if (restPtr) {
        delete [] restPtr;
    }
    if (contentPtr) {
        delete [] contentPtr;
    }
    return;
}

int KernelInterface::GetCurrentBuffer()
{
    std::map<std::string, std::string> result;
    ReadZswapdPressureShow(result);
    auto value = result.find(ZWAPD_PRESSURE_SHOW_BUFFER_SIZE);
    if (value != result.end()) {
#ifdef USE_HYPERHOLD_MEMORY
        HILOGD("buffer_size=%{public}s MB", result[ZWAPD_PRESSURE_SHOW_BUFFER_SIZE].c_str());
        return atoi(result[ZWAPD_PRESSURE_SHOW_BUFFER_SIZE].c_str()) * KB_PER_MB;
#else
        HILOGD("buffer_size=%{public}s KB", result[ZWAPD_PRESSURE_SHOW_BUFFER_SIZE].c_str());
        return atoi(result[ZWAPD_PRESSURE_SHOW_BUFFER_SIZE].c_str());
#endif
    }
    return MAX_BUFFER_KB;
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

bool KernelInterface::GetAllProcPids(std::vector<unsigned int> &pids)
{
    pids.clear();
    DIR *dir = opendir(ROOT_PROC_PATH.c_str());
    if (dir == nullptr) {
        HILOGE("dir %{public}s is not exist", ROOT_PROC_PATH.c_str());
        return false;
    }
    struct dirent *ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
            // current dir OR parent dir
            continue;
        } else if (ptr->d_type == DT_DIR) {
            int pid = atoi(ptr->d_name);
            if (pid > 0) {
                pids.push_back((unsigned int)pid);
            }
        }
    }
    if (dir) {
        closedir(dir);
    }
    HILOGD("there are %{public}zu pids under %{public}s", pids.size(), ROOT_PROC_PATH.c_str());
    return true;
}

bool KernelInterface::GetUidByPid(unsigned int pid, unsigned int& uid)
{
    std::string path = JoinPath(ROOT_PROC_PATH, std::to_string(pid), "status");
    std::string content;
    if (!ReadFromFile(path, content)) {
        HILOGE("read file failed. %{public}s", path.c_str());
        return false;
    }
    content = std::regex_replace(content, std::regex("\n+"), " "); // replace \n with space
    std::regex re(".*Uid:[[:s:]]*([[:d:]]+)[[:s:]]*([[:d:]]+)[[:s:]]*([[:d:]]+)[[:s:]]*([[:d:]]+).*");
    std::smatch res;
    if (!std::regex_match(content, res, re)) {
        HILOGD("re not match. %{public}s", content.c_str());
        return false;
    }
    try {
        uid = (unsigned int)std::stoi(res.str(1)); // 1: Uid index
    } catch (...) {
        HILOGE("stoi(%{public}s) failed", res.str(1).c_str());
        return false;
    }
    return true;
}

void DeleteCharArrayIfNotNull(char * charArray)
{
    if (charArray) {
        delete [] charArray;
        charArray = nullptr;
    }
}

bool KernelInterface::ReadSwapOutKBSinceKernelBoot(const std::string &path, const std::string &tagStr,
    unsigned long long &ret)
{
    std::string contentStr;
    if (!ReadFromFile(path, contentStr)) {
        return false;
    }
    char *contentPtr = new char[contentStr.size() + 1];
    if (contentPtr == nullptr) {
        HILOGE("alloc buffer fail");
        return false;
    }
    if (strcpy_s(contentPtr, contentStr.size() + 1, contentStr.c_str()) != EOK) {
        HILOGE("copy fail");
        DeleteCharArrayIfNotNull(contentPtr);
        return false;
    }
    bool success = false;
    char *restPtr;
    char *line = strtok_r(contentPtr, "\n", &restPtr);
    do {
        std::string lineStr(line);

        size_t i = 0;
        for (; i < strlen(line); i++) {
            if (line[i] == ':') {
                break;
            }
        }
        if (i >= strlen(line) - 2) { // 2: no : in the line or : is at end of line
            line = strtok_r(NULL, "\n", &restPtr);
            continue;
        }
        std::string tag = lineStr.substr(0, i);
        if (tag == tagStr) {
            std::string value = lineStr.substr(i + 1);
            std::istringstream iss(value);
            std::string sizeStr;
            std::string unitStr;
            iss >> sizeStr >> unitStr;
            try {
                ret = std::strtoull(sizeStr.c_str(), nullptr, 10); // 10:Decimal
                success = true;
            } catch (...) {
                HILOGE("parse [%{public}s] to unsigned long long error!", sizeStr.c_str());
            }
            break;
        }

        line = strtok_r(NULL, "\n", &restPtr);
    } while (line);
    DeleteCharArrayIfNotNull(contentPtr);
    return success;
}

int KernelInterface::ParseMeminfo(const std::string &contentStr, const std::string &itemName)
{
    char *contentPtr = new (std::nothrow) char[contentStr.size() + 1];
    if (contentPtr == nullptr) {
        HILOGE("alloc buffer fail");
        return -1;
    }
    if (strcpy_s(contentPtr, contentStr.size() + 1, contentStr.c_str()) != EOK) {
        HILOGE("copy fail");
        delete [] contentPtr;
        return -1;
    }
    char *restPtr = nullptr;
    char *line = strtok_r(contentPtr, "\n", &restPtr);
    std::string name;
    std::string value;
    bool findTotalMem = false;
    do {
        for (size_t i = 0; i < strlen(line); i++) {
            if (line[i] == ':') {
                line[i] = ' ';
            }
        }
        std::string lineStr(line);
        std::istringstream is(lineStr);

        is >> name >> value;
        if (name == itemName) {
            findTotalMem = true;
            break;
        }
        line = strtok_r(NULL, "\n", &restPtr);
    } while (line);
    if (contentPtr) {
        delete [] contentPtr;
    }

    if (findTotalMem == false) {
        return -1;
    }
    std::string valueTemp = "";
    for (auto c : value) {
        if (c >= '0' && c <= '9') {
            valueTemp = valueTemp + c;
        }
    }
    return atoi(valueTemp.c_str());
}

int KernelInterface::GetTotalBuffer()
{
    if (totalBuffer_ >= 0) {
        return totalBuffer_;
    }

    std::string contentStr;
    if (!ReadFromFile(MEMINFO_PATH, contentStr)) {
        HILOGE("read %{public}s faild, content=[%{public}s]", MEMINFO_PATH.c_str(), contentStr.c_str());
        return -1;
    }
    totalBuffer_ = ParseMeminfo(contentStr, TOTAL_MEMORY);
    return totalBuffer_;
}

bool KernelInterface::GetMemcgPids(const std::string &memcgPath, std::vector<int> &memcgPids)
{
    std::string path = JoinPath(memcgPath, FILE_MEMCG_PROCS);
    std::vector<std::string> strLines;
    if (!ReadLinesFromFile(path, strLines)) {
        HILOGE("read file and split to lines failed : %{public}s", path.c_str());
        return false;
    }

    memcgPids.clear();
    int pid;
    for (auto &it : strLines) {
        try {
            pid = stoi(it);
        } catch (...) {
            continue;
        }
        memcgPids.emplace_back(pid);
    }
    HILOGD("there are %{public}zu pids in %{public}s", memcgPids.size(), path.c_str());
    return true;
}

bool KernelInterface::GetAllUserIds(std::vector<int> &userIds)
{
    userIds.clear();
    DIR *dir = opendir(MEMCG_BASE_PATH.c_str());
    if (dir == nullptr) {
        HILOGE("dir %{public}s is not exist", MEMCG_BASE_PATH.c_str());
        return false;
    }
    struct dirent *ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
            // current dir OR parent dir
            continue;
        } else if (ptr->d_type == DT_DIR) {
            int userId = atoi(ptr->d_name);
            if (userId > 0) {
                userIds.push_back(userId);
            }
        }
    }
    if (dir) {
        closedir(dir);
    }
    HILOGD("there are %{public}zu userIds under %{public}s", userIds.size(), MEMCG_BASE_PATH.c_str());
    return true;
}

void KernelInterface::SplitOneLineByDelim(const std::string &input, const char delimiter,
    std::vector<std::string> &res)
{
    std::stringstream ss(input);
    std::string temp;
    while (getline(ss, temp, delimiter)) {
        if (!temp.empty()) {
            res.emplace_back(temp);
        }
    }
}

void KernelInterface::SplitOneLineByBlank(const std::string &input, std::vector<std::string> &res)
{
    std::stringstream ss(input);
    std::string temp;
    while (ss >> temp) {
        if (!temp.empty()) {
            res.emplace_back(temp);
        }
    }
}
} // namespace Memory
} // namespace OHOS
