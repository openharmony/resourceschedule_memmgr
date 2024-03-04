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

#ifndef OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_KERNEL_INTERFACE_H
#define OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_KERNEL_INTERFACE_H

#include <fcntl.h>
#include <map>
#include <string>
#include <sys/types.h>
#include <vector>

#include "single_instance.h"

namespace OHOS {
namespace Memory {
#define PAGE_TO_KB 4
#define KB_PER_MB 1024
#define MAX_BUFFER_KB 0x7fffffff

struct ProcInfo {
    int tgid;
    int pid;
    int pidfd;
    int size;
    std::string name;
    std::string status;
};

class KernelInterface {
    DECLARE_SINGLE_INSTANCE(KernelInterface);

public:
    bool EchoToPath(const char* path, const char* content);

    // file operations
    bool IsFileExists(const std::string& path);
    bool CreateFile(const std::string& path); // default mode 644(-rw-r--r--)
    bool CreateFile(const std::string& path, const mode_t& mode);
    bool RemoveFile(const std::string& path);
    bool WriteToFile(const std::string& path, const std::string& content, bool truncated = true);
    bool ReadFromFile(const std::string& path, std::string& content);
    bool ReadLinesFromFile(const std::string& path, std::vector<std::string>& lines);
    // dir operations
    bool IsDirExists(const std::string& path);
    bool IsExists(const std::string& path); // file or dir
    bool IsEmptyDir(const std::string& path);
    bool CreateDir(const std::string& path); // create dir recursively 755
    bool RemoveDirRecursively(const std::string& path);
    std::string AddDelimiter(const std::string& path); // IncludeTrailingPathDelimiter
    std::string RmDelimiter(const std::string& path); // ExcludeTrailingPathDelimiter
    std::string JoinPath(const std::string& prefixPath, const std::string& subPath);
    std::string JoinPath(const std::string& prefixPath, const std::string& midPath, const std::string& subPath);

    void SplitOneLineByDelim(const std::string &input, const char delimiter, std::vector<std::string> &res);
    void SplitOneLineByBlank(const std::string &input, std::vector<std::string> &res);

    bool GetPidProcInfo(struct ProcInfo &procInfo);
    bool GetProcNameByPid(int pid, std::string &name);
    void ReadZswapdPressureShow(std::map<std::string, std::string>& result);
    int GetCurrentBuffer();
    int KillOneProcessByPid(int pid);
    bool GetAllProcPids(std::vector<unsigned int>& pids);
    bool GetUidByPid(unsigned int pid, unsigned int& uid);
    bool ReadSwapOutKBSinceKernelBoot(const std::string &path, const std::string &tagStr, unsigned long long &ret);
    int64_t GetSystemCurTime();
    int64_t GetSystemTimeMs();

    int GetTotalBuffer();
    bool GetMemcgPids(const std::string &memcgPath, std::vector<int> &memcgPids);
    bool GetAllUserIds(std::vector<int> &userIds);

    static const std::string ROOT_PROC_PATH;
    static const std::string MEMCG_BASE_PATH;
    static const std::string FILE_MEMCG_PROCS;
    static const std::string ZWAPD_PRESSURE_SHOW_PATH;
    static const std::string ZWAPD_PRESSURE_SHOW_BUFFER_SIZE;
    static const std::string MEMINFO_PATH;
    static const std::string FILE_PROC_STATUS;
    static const std::string TOTAL_MEMORY;

    static constexpr mode_t FILE_MODE_666 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // -rw-rw-rw--
    static constexpr mode_t FILE_MODE_664 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH; // -rw-rw-r--
    static constexpr mode_t FILE_MODE_644 = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // -rw-r--r--
    static constexpr mode_t FILE_MODE_660 = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    static constexpr mode_t FILE_MODE_755 = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    static constexpr mode_t FILE_MODE_775 = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    static constexpr mode_t FILE_MODE_770 = S_IRWXU | S_IRWXG;
    static constexpr mode_t FILE_MODE_700 = S_IRWXU;
private:
    int ParseMeminfo(const std::string &contentStr, const std::string &itemName);

    int totalBuffer_ = -1;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_KERNEL_INTERFACE_H
