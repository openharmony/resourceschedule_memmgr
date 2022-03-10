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

#ifndef OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_MEMCG_H
#define OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_MEMCG_H

#include <string>

namespace OHOS {
namespace Memory {
class SwapInfo final {
public:
    unsigned int swapOutCount_;
    unsigned int swapOutSize_;
    unsigned int swapInCount_;
    unsigned int swapInSize_;
    unsigned int pageInCount_;
    unsigned int swapSizeCurr_;
    unsigned int swapSizeMax_;

    SwapInfo();
    SwapInfo(unsigned int swapOutCount, unsigned int swapOutSize, unsigned int swapInCount,
        unsigned int swapInSize, unsigned int pageInCount, unsigned int swapSizeCurr, unsigned int swapSizeMax);
    std::string ToString() const;

    SwapInfo(const SwapInfo&) = delete;
    SwapInfo& operator=(const SwapInfo&) = delete;
    SwapInfo(SwapInfo&&) = delete;
    SwapInfo& operator=(SwapInfo&&) = delete;
}; // end class SwapInfo

class MemInfo final {
public:
    unsigned int anonKiB_;
    unsigned int zramKiB_;
    unsigned int eswapKiB_;

    MemInfo();
    MemInfo(unsigned int anonKiB, unsigned int zramKiB, unsigned int eswapKiB);
    std::string ToString() const;

    MemInfo(const MemInfo&) = delete;
    MemInfo& operator=(const MemInfo&) = delete;
    MemInfo(MemInfo&&) = delete;
    MemInfo& operator=(MemInfo&&) = delete;
}; // end class MemInfo

class ReclaimRatios final {
public:
    unsigned int mem2zramRatio_;
    unsigned int zram2ufsRatio_;
    unsigned int refaultThreshold_;

    ReclaimRatios();
    ReclaimRatios(unsigned int mem2zramRatio, unsigned int zram2ufsRatio, unsigned int refaultThreshold);
    void SetRatios(unsigned int mem2zramRatio, unsigned int zram2ufsRatio, unsigned int refaultThreshold);
    bool SetRatios(ReclaimRatios * const ratios);
    std::string NumsToString() const; // only nums, not easy for reading
    std::string ToString() const; // easy for reading

    ReclaimRatios(const ReclaimRatios&) = delete;
    ReclaimRatios& operator=(const ReclaimRatios&) = delete;
    ReclaimRatios(ReclaimRatios&&) = delete;
    ReclaimRatios& operator=(ReclaimRatios&&) = delete;
}; // end class ReclaimRatios

class Memcg {
public:
    int score_;
    SwapInfo* swapInfo_;
    MemInfo* memInfo_;
    ReclaimRatios* reclaimRatios_;

    Memcg();
    virtual ~Memcg();
    Memcg(const Memcg&) = delete;
    Memcg& operator=(const Memcg&) = delete;
    Memcg(Memcg&&) = delete;
    Memcg& operator=(Memcg&&) = delete;

    bool UpdateSwapInfoFromKernel();
    bool UpdateMemInfoFromKernel();

    void SetScore(int score);
    void SetReclaimRatios(unsigned int mem2zramRatio, unsigned int zram2ufsRatio, unsigned int refaultThreshold);
    bool SetReclaimRatios(ReclaimRatios * const ratios);
    bool SetScoreAndReclaimRatiosToKernel();
    bool SwapIn(); // 100% load to mem
protected:
    virtual std::string GetMemcgPath_();
    bool WriteToFile_(const std::string& path, const std::string& content, bool truncated = true);
    bool ReadScoreAndReclaimRatiosFromKernel_(int& score, unsigned int& mem2zramRatio, unsigned int& zram2ufsRatio,
                                              unsigned int& refaultThreshold);
}; // end class Memcg

class UserMemcg final : public Memcg {
public:
    unsigned int userId_;

    explicit UserMemcg(unsigned int userId);
    ~UserMemcg();
    UserMemcg() = delete;
    UserMemcg(const UserMemcg&) = delete;
    UserMemcg& operator=(const UserMemcg&) = delete;
    UserMemcg(UserMemcg&&) = delete;
    UserMemcg& operator=(UserMemcg&&) = delete;

    bool CreateMemcgDir();
    bool RemoveMemcgDir();
    bool AddProc(unsigned int pid);
protected:
    std::string GetMemcgPath_() final;
}; // end class UserMemcg
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_MEMCG_H
