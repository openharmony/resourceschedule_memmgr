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

#ifndef OHOS_MEMORY_MEMMGR_PURGEABLE_MEM_CONSTANTS_H
#define OHOS_MEMORY_MEMMGR_PURGEABLE_MEM_CONSTANTS_H

namespace OHOS {
namespace Memory {
constexpr unsigned int PURGEABLE_SUBSCRIBER_MAX_NUM = 100;
constexpr unsigned int PURGEABLE_APPSTATE_MAX_NUM = 100;
constexpr unsigned int PARAM_SIZE_ONTRIM = 2;
constexpr unsigned int PARAM_SIZE_RECLAIMALL = 1;
constexpr unsigned int PARAM_SIZE_RECLAIM_BY_PID = 3;
constexpr unsigned int PARAM_SIZE_SHOW_APPS = 1;
constexpr unsigned int FIRST_ARG_INDEX = 0;
constexpr unsigned int PID_INDEX = 2;
constexpr unsigned int SYSTEM_MEMORY_LEVEL_INDEX = 1;
constexpr unsigned int SECOND_ARG_INDEX = 1;

constexpr unsigned int MEMORY_LEVEL_PURGEABLE = 1;
constexpr unsigned int MEMORY_LEVEL_MODERATE = 2;
constexpr unsigned int MEMORY_LEVEL_LOW = 3;
constexpr unsigned int MEMORY_LEVEL_CRITICAL = 4;

constexpr unsigned int APP_STATE_FOREGROUND = 2;
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_PURGEABLE_MEM_CONSTANTS_H
