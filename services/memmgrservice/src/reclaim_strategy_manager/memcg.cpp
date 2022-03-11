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
#include "reclaim_strategy_constants.h"
#include "memcg.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "Memcg";
} // namespace

SwapInfo::SwapInfo()
    : swapOutCount_(0),
      swapOutSize_(0),
      swapInCount_(0),
      swapInSize_(0),
      pageInCount_(0),
      swapSizeCurr_(0),
      swapSizeMax_(0) {}

SwapInfo::SwapInfo(unsigned int swapOutCount, unsigned int swapOutSize, unsigned int swapInCount,
                   unsigned int swapInSize, unsigned int pageInCount, unsigned int swapSizeCurr,
                   unsigned int swapSizeMax)
    : swapOutCount_(swapOutCount),
      swapOutSize_(swapOutSize),
      swapInCount_(swapInCount),
      swapInSize_(swapInSize),
      pageInCount_(pageInCount),
      swapSizeCurr_(swapSizeCurr),
      swapSizeMax_(swapSizeMax) {}

inline std::string SwapInfo::ToString() const
{
    std::string ret = "swapOutCount:" + std::to_string(swapOutCount_)
                    + " swapOutSize:" + std::to_string(swapOutSize_)
                    + " swapInCount:" + std::to_string(swapInCount_)
                    + " swapInSize:" + std::to_string(swapInSize_)
                    + " pageInCount:" + std::to_string(pageInCount_)
                    + " swapSizeCurr:" + std::to_string(swapSizeCurr_)
                    + " swapSizeMax:" + std::to_string(swapSizeMax_);
    return ret;
}

MemInfo::MemInfo() : anonKiB_(0), zramKiB_(0), eswapKiB_(0) {}

MemInfo::MemInfo(unsigned int anonKiB, unsigned int zramKiB, unsigned int eswapKiB)
    : anonKiB_(anonKiB),
      zramKiB_(zramKiB),
      eswapKiB_(eswapKiB) {}

inline std::string MemInfo::ToString() const
{
    std::string ret = "anonKiB:" + std::to_string(anonKiB_)
                    + " zramKiB:" + std::to_string(zramKiB_)
                    + " eswapKiB:" + std::to_string(eswapKiB_);
    return ret;
}

ReclaimRatios::ReclaimRatios()
    : mem2zramRatio_(MEMCG_MEM_2_ZRAM_RATIO),
      zram2ufsRatio_(MEMCG_ZRAM_2_UFS_RATIO),
      refaultThreshold_(MEMCG_REFAULT_THRESHOLD) {}

ReclaimRatios::ReclaimRatios(unsigned int mem2zramRatio, unsigned int zram2ufsRatio, unsigned int refaultThreshold)
    : refaultThreshold_(refaultThreshold)
{
    mem2zramRatio_ = (mem2zramRatio > PERCENT_100) ? PERCENT_100 : mem2zramRatio;
    zram2ufsRatio_ = (zram2ufsRatio > PERCENT_100) ? PERCENT_100 : zram2ufsRatio;
}

void ReclaimRatios::SetRatiosByValue(unsigned int mem2zramRatio, unsigned int zram2ufsRatio,
                                     unsigned int refaultThreshold)
{
    mem2zramRatio_ = (mem2zramRatio > PERCENT_100) ? PERCENT_100 : mem2zramRatio;
    zram2ufsRatio_ = (zram2ufsRatio > PERCENT_100) ? PERCENT_100 : zram2ufsRatio;
    refaultThreshold_ = refaultThreshold;
}

void ReclaimRatios::SetRatios(const ReclaimRatios& ratios)
{
    SetRatiosByValue(ratios.mem2zramRatio_, ratios.zram2ufsRatio_, ratios.refaultThreshold_);
}

inline std::string ReclaimRatios::NumsToString() const
{
    std::string ret = std::to_string(mem2zramRatio_) + " "
                    + std::to_string(zram2ufsRatio_) + " "
                    + std::to_string(refaultThreshold_);
    return ret;
}

std::string ReclaimRatios::ToString() const
{
    std::string ret = "mem2zramRatio:" + std::to_string(mem2zramRatio_)
                    + " zram2ufsRatio:" + std::to_string(zram2ufsRatio_)
                    + " refaultThreshold:" + std::to_string(refaultThreshold_);
    return ret;
}

Memcg::Memcg() : score_(0)
{
    swapInfo_ = new (std::nothrow) SwapInfo();
    memInfo_ = new (std::nothrow) MemInfo();
    reclaimRatios_ = new (std::nothrow) ReclaimRatios();
    if (swapInfo_ == nullptr || memInfo_ == nullptr || reclaimRatios_ == nullptr) {
        HILOGE("new obj failed! init memcg failed");
    } else {
        HILOGI("init memcg success");
    }
}

Memcg::~Memcg()
{
    delete swapInfo_;
    swapInfo_ = nullptr;
    delete memInfo_;
    memInfo_ = nullptr;
    delete reclaimRatios_;
    reclaimRatios_ = nullptr;
    HILOGI("release memcg success");
}

bool Memcg::UpdateSwapInfoFromKernel()
{
    if (swapInfo_ == nullptr) {
        HILOGE("swapInfo_ nullptr");
        return false;
    }
    std::string path = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.eswap_stat");
    std::string content;
    if (!KernelInterface::GetInstance().ReadFromFile(path, content)) {
        HILOGE("file not found. %{public}s", path.c_str());
        return false;
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
    if (!std::regex_match(content, res, re)) {
        HILOGI("re not match. %{public}s", content.c_str());
        return false;
    }
    try {
        swapInfo_->swapOutCount_ = (unsigned int)std::stoi(res.str(1)); // 1: swapOutCount index
        swapInfo_->swapOutSize_ = (unsigned int)std::stoi(res.str(2)); // 2: swapOutSize index
        swapInfo_->swapInSize_ = (unsigned int)std::stoi(res.str(3)); // 3: swapInSize index
        swapInfo_->swapInCount_ = (unsigned int)std::stoi(res.str(4)); // 4: swapInCount index
        swapInfo_->pageInCount_ = (unsigned int)std::stoi(res.str(5)); // 5: pageInCount index
        swapInfo_->swapSizeCurr_ = (unsigned int)std::stoi(res.str(6)); // 6: swapSizeCurr index
        swapInfo_->swapSizeMax_ = (unsigned int)std::stoi(res.str(7)); // 7: swapSizeMax index
    } catch (std::out_of_range&) {
        HILOGE("stoi() failed: out_of_range");
        return false;
    }
    HILOGI("success. %{public}s", swapInfo_->ToString().c_str());
    return true;
}

bool Memcg::UpdateMemInfoFromKernel()
{
    if (memInfo_ == nullptr) {
        HILOGE("memInfo_ nullptr");
        return false;
    }
    std::string path = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.stat");
    std::string content;
    if (!KernelInterface::GetInstance().ReadFromFile(path, content)) {
        HILOGE("file not found. %{public}s", path.c_str());
        return false;
    }
    content = std::regex_replace(content, std::regex("\n+"), " "); // replace \n with space
    std::regex re(".*Anon:[[:s:]]*([[:d:]]+) kB[[:s:]]*"
                  ".*[zZ]ram:[[:s:]]*([[:d:]]+) kB[[:s:]]*"
                  "Eswap:[[:s:]]*([[:d:]]+) kB[[:s:]]*");
    std::smatch res;
    if (!std::regex_match(content, res, re)) {
        HILOGI("re not match. %{public}s", content.c_str());
        return false;
    }
    try {
        memInfo_->anonKiB_ = (unsigned int)std::stoi(res.str(1)); // 1: anonKiB index
        memInfo_->zramKiB_ = (unsigned int)std::stoi(res.str(2)); // 2: zramKiB index
        memInfo_->eswapKiB_ = (unsigned int)std::stoi(res.str(3)); // 3: eswapKiB index
    } catch (std::out_of_range&) {
        HILOGE("stoi() failed: out_of_range");
        return false;
    }
    HILOGI("success. %{public}s", memInfo_->ToString().c_str());
    return true;
}

void Memcg::SetScore(int score)
{
    score_ = score;
}

void Memcg::SetReclaimRatios(unsigned int mem2zramRatio, unsigned int zram2ufsRatio, unsigned int refaultThreshold)
{
    if (reclaimRatios_ == nullptr) {
        HILOGE("reclaimRatios_ nullptr");
        return;
    }
    reclaimRatios_->mem2zramRatio_ = (mem2zramRatio > PERCENT_100) ? PERCENT_100 : mem2zramRatio;
    reclaimRatios_->zram2ufsRatio_ = (zram2ufsRatio > PERCENT_100) ? PERCENT_100 : zram2ufsRatio;
    reclaimRatios_->refaultThreshold_ = refaultThreshold;
}

bool Memcg::SetReclaimRatios(const ReclaimRatios& ratios)
{
    if (reclaimRatios_ == nullptr) {
        HILOGE("reclaimRatios_ nullptr");
        return false;
    }
    reclaimRatios_->SetRatios(ratios);
    return true;
}

bool Memcg::SetScoreAndReclaimRatiosToKernel()
{
    if (reclaimRatios_ == nullptr) {
        HILOGE("reclaimRatios_ nullptr");
        return false;
    }
    bool ret = false;
    // write score
    std::string scorePath = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.app_score");
    ret = WriteToFile_(scorePath, std::to_string(score_));
    // write reclaim ratios
    std::string ratiosPath = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(),
        "memory.zswapd_single_memcg_param");
    ret = ret && WriteToFile_(ratiosPath, reclaimRatios_->NumsToString());
    // double check: check file content
    int score = 0;
    unsigned int mem2zramRatio = 0;
    unsigned int zram2ufsRatio = 0;
    unsigned int refaultThreshold = 0;
    if (!ReadScoreAndReclaimRatiosFromKernel_(score, mem2zramRatio, zram2ufsRatio, refaultThreshold)) {
        return ret;
    }
    ret = ret && (score_ == score);
    ret = ret && (reclaimRatios_->mem2zramRatio_ == mem2zramRatio);
    ret = ret && (reclaimRatios_->zram2ufsRatio_ == zram2ufsRatio);
    ret = ret && (reclaimRatios_->refaultThreshold_ == refaultThreshold);
    if (ret == false) { // if values of mem and kernel not matched, using kernel values
        score_ = score;
        reclaimRatios_->mem2zramRatio_ = mem2zramRatio;
        reclaimRatios_->zram2ufsRatio_ = zram2ufsRatio;
        reclaimRatios_->refaultThreshold_ = refaultThreshold;
    }
    return ret;
}

bool Memcg::SwapIn()
{
    std::string zramPath = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.ub_ufs2zram_ratio");
    bool ret = WriteToFile_(zramPath, std::to_string(PERCENT_100)); // load 100% to zram
    std::string swapinPath = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.force_swapin");
    ret = ret && WriteToFile_(swapinPath, "0"); // echo 0 to tigger force swapin
    return ret;
}

inline std::string Memcg::GetMemcgPath_()
{
    // memcg dir: "/dev/memcg"
    return KernelInterface::MEMCG_BASE_PATH;
}

inline bool Memcg::WriteToFile_(const std::string& path, const std::string& content, bool truncated)
{
    std::string op = truncated ? ">" : ">>";
    if (!KernelInterface::GetInstance().WriteToFile(path, content, truncated)) {
        HILOGE("failed. %{public}s %{public}s %{public}s", content.c_str(), op.c_str(), path.c_str());
        return false;
    }
    HILOGI("success. %{public}s %{public}s %{public}s", content.c_str(), op.c_str(), path.c_str());
    return true;
}

bool Memcg::ReadScoreAndReclaimRatiosFromKernel_(int& score, unsigned int& mem2zramRatio,
    unsigned int& zram2ufsRatio, unsigned int& refaultThreshold)
{
    std::string path = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "memory.zswapd_single_memcg_param");
    std::string content;
    if (!KernelInterface::GetInstance().ReadFromFile(path, content)) {
        HILOGE("file not found. %{public}s", path.c_str());
        return false;
    }
    content = std::regex_replace(content, std::regex("\n+"), " "); // replace \n with space
    std::regex re("memcg score:[[:s:]]*([[:d:]]+)[[:s:]]*"
                  "memcg ub_mem2zram_ratio:[[:s:]]*([[:d:]]+)[[:s:]]*"
                  "memcg ub_zram2ufs_ratio:[[:s:]]*([[:d:]]+)[[:s:]]*"
                  "memcg refault_threshold:[[:s:]]*([[:d:]]+)[[:s:]]*");
    std::smatch res;
    if (!std::regex_match(content, res, re)) {
        HILOGI("re not match. %{public}s", content.c_str());
        return false;
    }
    try {
        score = std::stoi(res.str(1)); // 1: memcg score index
        mem2zramRatio = (unsigned int)std::stoi(res.str(2)); // 2: memcg mem2zramRatio index
        zram2ufsRatio = (unsigned int)std::stoi(res.str(3)); // 3: memcg zram2ufsRatio index
        refaultThreshold = (unsigned int)std::stoi(res.str(4)); // 4: memcg refaultThreshold index
    } catch (std::out_of_range&) {
        HILOGE("stoi() failed: out_of_range");
        return false;
    }
    return true;
}

UserMemcg::UserMemcg(unsigned int userId) : userId_(userId)
{
    HILOGI("init UserMemcg success");
}

UserMemcg::~UserMemcg()
{
    HILOGI("release UserMemcg success");
}

bool UserMemcg::CreateMemcgDir()
{
    std::string fullPath = GetMemcgPath_();
    if (!KernelInterface::GetInstance().CreateDir(fullPath)) {
        HILOGE("failed. %{public}s", fullPath.c_str());
        return false;
    }
    HILOGI("success. %{public}s", fullPath.c_str());
    return true;
}

bool UserMemcg::RemoveMemcgDir()
{
    std::string fullPath = GetMemcgPath_();
    if (!KernelInterface::GetInstance().RemoveDirRecursively(fullPath)) {
        HILOGE("failed. %{public}s", fullPath.c_str());
        return false;
    }
    HILOGI("success. %{public}s", fullPath.c_str());
    return true;
}

std::string UserMemcg::GetMemcgPath_()
{
    // user memcg dir: "/dev/memcg/${userId}"
    return KernelInterface::GetInstance().JoinPath(KernelInterface::MEMCG_BASE_PATH, std::to_string(userId_));
}

bool UserMemcg::AddProc(unsigned int pid)
{
    std::string fullPath = KernelInterface::GetInstance().JoinPath(GetMemcgPath_(), "cgroup.procs");
    bool ret = WriteToFile_(fullPath, std::to_string(pid), false);
    // double check file content
    bool dirExists = KernelInterface::GetInstance().IsDirExists(GetMemcgPath_());
    bool fileExists = KernelInterface::GetInstance().IsFileExists(fullPath);
    std::string content;
    KernelInterface::GetInstance().ReadFromFile(fullPath, content);
    content = std::regex_replace(content, std::regex("\n+"), " "); // replace \n with space
    HILOGI("dir:%{public}s exist=%{public}d. file:%{public}s exist=%{public}d content=*%{public}s* ret=%{public}d",
           GetMemcgPath_().c_str(), dirExists, fullPath.c_str(), fileExists, content.c_str(), ret);
    return ret;
}
} // namespace Memory
} // namespace OHOS
