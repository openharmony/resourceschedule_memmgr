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

#ifndef OHOS_MEMORY_MEMMGR_RECALIM_PRIORITY_CONSTANTS_H
#define OHOS_MEMORY_MEMMGR_RECALIM_PRIORITY_CONSTANTS_H

#include <map>
#include <string>
#include <set>

namespace OHOS {
namespace Memory {
// system app
constexpr int RECLAIM_PRIORITY_SYSTEM = -1000;
// foreground process priority
constexpr int RECLAIM_PRIORITY_FOREGROUND = 0;
// perceived suspend delay case
constexpr int RECLAIM_PRIORITY_BG_SUSPEND_DELAY = 100;
// perceived background process priority
constexpr int RECLAIM_PRIORITY_BG_PERCEIVED = 200;
// backgroud priority
constexpr int RECLAIM_PRIORITY_BACKGROUND = 400;
// suspend process priority
constexpr int RECLAIM_PRIORITY_SUSPEND = 800;
// visible process priority
constexpr int RECLAIM_PRIORITY_VISIBLE = 1;
// frozen process priority
constexpr int RECLAIM_PRIORITY_FROZEN = 600;
// empty process priority
constexpr int RECLAIM_PRIORITY_EMPTY = 900;
// unknown process priority
constexpr int RECLAIM_PRIORITY_UNKNOWN = 1000;

constexpr std::string_view SYSTEM_UI_BUNDLE_NAME = "com.ohos.systemui";
constexpr std::string_view LAUNCHER_BUNDLE_NAME = "com.ohos.launcher";

const int USER_ID_SHIFT = 100000;

const int IGNORE_PID = -1;

enum class AppStateUpdateReason {
    CREATE_PROCESS = 0,
    PROCESS_READY,
    FOREGROUND,
    BACKGROUND,
    SUSPEND_DELAY_START,
    SUSPEND_DELAY_END,
    BACKGROUND_RUNNING_START,
    BACKGROUND_RUNNING_END,
    EVENT_START,
    EVENT_END,
    DATA_ABILITY_START,
    DATA_ABILITY_END,
    APPLICATION_SUSPEND,
    PROCESS_TERMINATED,
    OS_ACCOUNT_CHANGED,
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECALIM_PRIORITY_CONSTANTS_H
