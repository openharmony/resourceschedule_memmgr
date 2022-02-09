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

#include <sys/types.h>
#include <string>

namespace OHOS {
namespace Memory {
class SwapInfo {
public:
    unsigned int swapOutCount;
    unsigned int swapOutSize;
    unsigned int swapInCount;
    unsigned int swapInSize;
    unsigned int pageInCount;
    unsigned int swapSizeCurr;
    unsigned int swapSizeMax;

    SwapInfo();
    SwapInfo(unsigned int swapOutCount, unsigned int swapOutSize, unsigned int swapInCount,
        unsigned int swapInSize, unsigned int pageInCount, unsigned int swapSizeCurr, unsigned int swapSizeMax);
    std::string ToString();
}; // end class SwapInfo

class MemInfo {
public:
    unsigned int anonKiB;
    unsigned int zramKiB;
    unsigned int eswapKiB;

    MemInfo();
    MemInfo(unsigned int anonKiB, unsigned int zramKiB, unsigned int eswapKiB);
    std::string ToString();
}; // end class MemInfo

class ReclaimRatios {
public:
    unsigned int reclaimRatio;
    unsigned int eswapRatio;
    unsigned int reclaimRefault;

    ReclaimRatios();
    ReclaimRatios(unsigned int reclaimRatio, unsigned int eswapRatio, unsigned int reclaimRefault);
    void UpdateReclaimRatios(unsigned int reclaimRatio, unsigned int eswapRatio, unsigned int reclaimRefault);
    std::string NumsToString(); // only nums, not easy for reading
    std::string ToString(); // easy for reading
}; // end class ReclaimRatios

class Memcg {
public:
    SwapInfo* swapInfo;
    MemInfo* memInfo;
    ReclaimRatios* reclaimRatios;

    Memcg();
    ~Memcg();

    void UpdateSwapInfoFromKernel();
    void UpdateMemInfoFromKernel();

    bool SetScoreToKernel(int score);
    bool SetRatiosToKernel();
    bool AddProc(pid_t pid);

private:
    std::string GetMemcgPath_();
    bool WriteToFile_(const std::string& path, const std::string& content, bool truncated = true);
}; // end class Memcg
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_MEMCG_H
