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

#ifndef OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_RECLAIM_PARAM_H
#define OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_RECLAIM_PARAM_H

#include <sys/types.h>

#include <string>

namespace OHOS {
namespace Memory {
enum class AppAction {
    CREATE_PROCESS_AND_APP = 0,
    CREATE_PROCESS_ONLY,
    APP_DIED,
    APP_FOREGROUND,
    APP_BACKGROUND,
    OTHERS,
};

class ReclaimParam {
public:
    pid_t pid;
    int bundleUid;
    std::string bundleName;
    int accountId;
    int appScore;
    AppAction action;

    explicit ReclaimParam(pid_t pid, int bundleUid, std::string bundleName, int accountId,
                          int appScore, AppAction action)
        : pid(pid),
          bundleUid(bundleUid),
          bundleName(bundleName),
          accountId(accountId),
          appScore(appScore),
          action(action) {}
}; // end class ReclaimParam
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_RECLAIM_PARAM_H
