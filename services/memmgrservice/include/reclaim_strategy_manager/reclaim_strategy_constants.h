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

#ifndef OHOS_MEMORY_MEMMGR_RECALIM_STRATEGY_CONSTANTS_H
#define OHOS_MEMORY_MEMMGR_RECALIM_STRATEGY_CONSTANTS_H

namespace OHOS {
namespace Memory {
// default values of buffer param (MB)
constexpr int AVAIL_BUFFER = 800;
constexpr int MIN_AVAIL_BUFFER = 750;
constexpr int HIGH_AVAIL_BUFFER = 850;
constexpr int SWAP_RESERVE = 200;
// default values of memcg reclaim ratios
constexpr int MEMCG_MEM_2_ZRAM_RATIO = 60; // 60%
constexpr int MEMCG_ZRAM_2_UFS_RATIO = 10; // 10%
constexpr int MEMCG_REFAULT_THRESHOLD = 50; // 50 times
// default reclaim ratios of root memcg
constexpr int ROOT_MEMCG_MEM_2_ZRAM_RATIO = 40; // 60%
constexpr int ROOT_MEMCG_ZRAM_2_UFS_RATIO = 0; // 0%
constexpr int ROOT_MEMCG_REFAULT_THRESHOLD = 0; // 0 times
// default score of memcg
constexpr int APP_SCORE = 300;
constexpr int PERCENT_100 = 100;
// default score range
constexpr int RECLAIM_SCORE_MIN = 0;
constexpr int RECLAIM_SCORE_MAX = 1000;
// min value of user id
constexpr int VALID_USER_ID_MIN = 100;
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECALIM_STRATEGY_CONSTANTS_H
