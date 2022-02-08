/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this->file except in compliance with the License.
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

#include <regex>

#include "memmgr_log.h"
#include "kernel_interface.h"
#include "memcg.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "Memcg";
} // namespace

SwapInfo::SwapInfo()
    : swapOutCount(0),
      swapOutSize(0),
      swapInCount(0),
      swapInSize(0),
      pageInCount(0),
      swapSizeCurr(0),
      swapSizeMax(0) {}

SwapInfo::SwapInfo(unsigned int swapOutCount, unsigned int swapOutSize, unsigned int swapInCount,
                   unsigned int swapInSize, unsigned int pageInCount, unsigned int swapSizeCurr,
                   unsigned int swapSizeMax)
    : swapOutCount(swapOutCount),
      swapOutSize(swapOutSize),
      swapInCount(swapInCount),
      swapInSize(swapInSize),
      pageInCount(pageInCount),
      swapSizeCurr(swapSizeCurr),
      swapSizeMax(swapSizeMax) {}

inline std::string SwapInfo::ToString()
{
    std::string ret = "swapOutCount:" + std::to_string(this->swapOutCount)
                    + " swapOutSize:" + std::to_string(this->swapOutSize)
                    + " swapInCount:" + std::to_string(this->swapInCount)
                    + " swapInSize:" + std::to_string(this->swapInSize)
                    + " pageInCount:" + std::to_string(this->pageInCount)
                    + " swapSizeCurr:" + std::to_string(this->swapSizeCurr)
                    + " swapSizeMax:" + std::to_string(this->swapSizeMax);
    return ret;
}

MemInfo::MemInfo()
{
    this->anonKiB = 0;
    this->zramKiB = 0;
    this->eswapKiB = 0;
}

MemInfo::MemInfo(unsigned int anonKiB, unsigned int zramKiB, unsigned int eswapKiB)
    : anonKiB(anonKiB),
      zramKiB(zramKiB),
      eswapKiB(eswapKiB) {}

inline std::string MemInfo::ToString()
{
    std::string ret = "anonKiB:" + std::to_string(this->anonKiB)
                    + " zramKiB:" + std::to_string(this->zramKiB)
                    + " eswapKiB:" + std::to_string(this->eswapKiB);
    return ret;
}

ReclaimRatios::ReclaimRatios()
{
    UpdateReclaimRatios(0, 0, 0);
}

ReclaimRatios::ReclaimRatios(unsigned int reclaimRatio, unsigned int eswapRatio, unsigned int reclaimRefault)
{
    UpdateReclaimRatios(reclaimRatio, eswapRatio, reclaimRefault);
}

void ReclaimRatios::UpdateReclaimRatios(unsigned int reclaimRatio, unsigned int eswapRatio,
                                        unsigned int reclaimRefault)
{
    this->reclaimRatio = reclaimRatio;
    this->eswapRatio = eswapRatio;
    this->reclaimRefault = reclaimRefault;
}

inline std::string ReclaimRatios::NumsToString()
{
    std::string ret = std::to_string(this->reclaimRatio) + " "
                    + std::to_string(this->eswapRatio) + " "
                    + std::to_string(this->reclaimRefault);
    return ret;
}

inline std::string ReclaimRatios::ToString()
{
    std::string ret = "reclaimRatio:" + std::to_string(this->reclaimRatio)
                    + " eswapRatio:" + std::to_string(this->eswapRatio)
                    + " reclaimRefault:" + std::to_string(this->reclaimRefault);
    return ret;
}

Memcg::Memcg()
{
    this->swapInfo = new SwapInfo();
    this->memInfo = new MemInfo();
    this->reclaimRatios = new ReclaimRatios();
    HILOGI("init memcg success");
}

Memcg::~Memcg()
{
    delete this->swapInfo;
    delete this->memInfo;
    delete this->reclaimRatios;
    HILOGI("release memcg success");
}

void Memcg::UpdateSwapInfoFromKernel()
{
    std::string path = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.eswap_stat");
    std::string content;
    if (!KernelInterface::GetInstance().ReadFromFile(path, content)) {
        return;
    }
    content = std::regex_replace(content, std::regex("\n+"), " "); // replace \n with space
    std::regex re(".*swapOutTotal:([[:d:]]+)[[:s:]]*"
                  "swapOutSize:([[:d:]]*) MB[[:s:]]*"
                  "swapInSize:([[:d:]]*) MB[[:s:]]*"
                  "swapInTotal:([[:d:]]*)[[:s:]]*"
                  "pageInTotal:([[:d:]]*)[[:s:]]*"
                  "swapSizeCur:([[:d:]]*) MB[[:s:]]*"
                  "swapSizeMax:([[:d:]]*) MB[[:s:]]*");
    std::smatch res;
    if (std::regex_match(content, res, re)) {
        this->swapInfo->swapOutCount = std::stoi(res.str(1)); // 1: swapOutCount index
        this->swapInfo->swapOutSize = std::stoi(res.str(2)); // 2: swapOutSize index
        this->swapInfo->swapInSize = std::stoi(res.str(3)); // 3: swapInSize index
        this->swapInfo->swapInCount = std::stoi(res.str(4)); // 4: swapInCount index
        this->swapInfo->pageInCount = std::stoi(res.str(5)); // 5: pageInCount index
        this->swapInfo->swapSizeCurr = std::stoi(res.str(6)); // 6: swapSizeCurr index
        this->swapInfo->swapSizeMax = std::stoi(res.str(7)); // 7: swapSizeMax index
    }
    HILOGD("success. %{public}s", this->swapInfo->ToString().c_str());
}

void Memcg::UpdateMemInfoFromKernel()
{
    std::string path = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.stat");
    std::string content;
    if (!KernelInterface::GetInstance().ReadFromFile(path, content)) {
        return;
    }
    content = std::regex_replace(content, std::regex("\n+"), " "); // replace \n with space
    std::regex re(".*Anon:[[:s:]]*([[:d:]]+) kB[[:s:]]*"
                  ".*Zram:[[:s:]]*([[:d:]]+) kB[[:s:]]*"
                  "Eswap:[[:s:]]*([[:d:]]+) kB[[:s:]]*");
    std::smatch res;
    if (std::regex_match(content, res, re)) {
        this->memInfo->anonKiB = std::stoi(res.str(1)); // 1: anonKiB index
        this->memInfo->zramKiB = std::stoi(res.str(2)); // 2: zramKiB index
        this->memInfo->eswapKiB = std::stoi(res.str(3)); // 3: eswapKiB index
    }
    HILOGD("success. %{public}s", this->memInfo->ToString().c_str());
}

bool Memcg::SetScoreToKernel(int score)
{
    std::string path = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.app_score");
    std::string content = std::to_string(score);
    return WriteToFile_(path, content);
}

bool Memcg::SetRatiosToKernel()
{
    std::string path = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.zswapd_single_memcg_param");
    std::string content = this->reclaimRatios->NumsToString();
    return WriteToFile_(path, content);
}

bool Memcg::AddProc(pid_t pid)
{
    std::string fullPath = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "cgroup.procs");
    std::string content = std::to_string(pid);
    return WriteToFile_(fullPath, content, false);
}

inline std::string Memcg::GetMemcgPath_()
{
    // memcg dir = /dev/memcg/
    return KernelInterface::MEMCG_BASE_PATH;
}

inline bool Memcg::WriteToFile_(const std::string& path, const std::string& content, bool truncated)
{
    std::string op = truncated ? ">" : ">>";
    if (!KernelInterface::GetInstance().WriteToFile(path, content, truncated)) {
        HILOGE("failed. %{public}s %{public}s %{public}s", content.c_str(), op.c_str(), path.c_str());
        return false;
    }
    HILOGD("success. %{public}s %{public}s %{public}s", content.c_str(), op.c_str(), path.c_str());
    return true;
}
} // namespace Memory
} // namespace OHOS
