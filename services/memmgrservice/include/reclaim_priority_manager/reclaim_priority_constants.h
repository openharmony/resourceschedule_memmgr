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

namespace OHOS {
namespace Memory {
// system app
constexpr int RECLAIM_PRIORITY_SYSTEM = -1000;
// killable system app
constexpr int RECLAIM_PRIORITY_KILLABLE_SYSTEM = -800;
// foreground process priority
constexpr int RECLAIM_PRIORITY_FOREGROUND = 0;
// visible process priority
constexpr int RECLAIM_PRIORITY_VISIBLE = 1;
// perceived suspend delay case
constexpr int RECLAIM_PRIORITY_BG_SUSPEND_DELAY = 100;
// extension and a bg process bind to it
constexpr int RECLAIM_PRIORITY_FG_BIND_EXTENSION = 100;
// perceived background process priority
constexpr int RECLAIM_PRIORITY_BG_PERCEIVED = 200;
// extension and a bg process bind to it (reserved, now unsed)
constexpr int RECLAIM_PRIORITY_BG_BIND_EXTENSION = 240;
// background and connected by distribute device
constexpr int RECLAIM_PRIORITY_BG_DIST_DEVICE = 260;
// extension and no process bind to it
constexpr int RECLAIM_PRIORITY_NO_BIND_EXTENSION = 400;
// background priority
constexpr int RECLAIM_PRIORITY_BACKGROUND = 400;
// frozen process priority
constexpr int RECLAIM_PRIORITY_FROZEN = 600;
// suspend process priority
constexpr int RECLAIM_PRIORITY_SUSPEND = 800;
// empty process priority
constexpr int RECLAIM_PRIORITY_EMPTY = 900;
// unknown process priority
constexpr int RECLAIM_PRIORITY_UNKNOWN = 1000;

constexpr int RECLAIM_PRIORITY_MIN = -1000;
constexpr int RECLAIM_PRIORITY_MAX = 1000;

const int USER_ID_SHIFT = 200000;
#define GET_OS_ACCOUNT_ID_BY_UID(bundleUid) ((bundleUid) / USER_ID_SHIFT)

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
    APPLICATION_SUSPEND,
    PROCESS_TERMINATED,
    OS_ACCOUNT_CHANGED,
    DIST_DEVICE_CONNECTED,
    DIST_DEVICE_DISCONNECTED,
    BIND_EXTENSION,
    UNBIND_EXTENSION,
    UPDATE_EXTENSION_PROCESS,
    VISIBLE,
    UN_VISIBLE,
};

enum class BundleState {
    STATE_DEFAULT = 0,
    STATE_WAITING_FOR_KILL,
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECALIM_PRIORITY_CONSTANTS_H
