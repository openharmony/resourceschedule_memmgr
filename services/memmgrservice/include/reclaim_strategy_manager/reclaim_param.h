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
    CREATE_PROCESS_ONLY = 1,
    APP_DIED = 2,
    APP_FOREGROUND = 3,
    APP_BACKGROUND = 4,
    OTHERS = 5,
};

const std::map<int, std::string> appActionStrMap_ = {
    { static_cast<int32_t>(AppAction::CREATE_PROCESS_AND_APP), "CREATE_PROCESS_AND_APP" },
    { static_cast<int32_t>(AppAction::CREATE_PROCESS_ONLY), "CREATE_PROCESS_ONLY" },
    { static_cast<int32_t>(AppAction::APP_DIED), "APP_DIED" },
    { static_cast<int32_t>(AppAction::APP_FOREGROUND), "APP_FOREGROUND" },
    { static_cast<int32_t>(AppAction::APP_BACKGROUND), "APP_BACKGROUND" },
    { static_cast<int32_t>(AppAction::OTHERS), "OTHERS" },
};

class ReclaimParam {
public:
    pid_t pid_;
    int bundleUid_;
    std::string bundleName_;
    int accountId_;
    int score_;
    AppAction action_;

    ReclaimParam(pid_t pid, int bundleUid, std::string bundleName, int accountId, int score, AppAction action)
        : pid_(pid),
          bundleUid_(bundleUid),
          bundleName_(bundleName),
          accountId_(accountId),
          score_(score),
          action_(action) {}

    static std::string GetAppActionStr(const AppAction action)
    {
        auto pair = appActionStrMap_.find(static_cast<int32_t>(action));
        if (pair == appActionStrMap_.end()) {
            return "none"; // none means no such action
        }
        return pair->second;
    }

    inline std::string ToString() const
    {
        std::string ret = "pid:" + std::to_string(pid_)
                        + " bundle:" + std::to_string(bundleUid_)
                        + " " + bundleName_
                        + " userId:" + std::to_string(accountId_)
                        + " score:" + std::to_string(score_)
                        + " action:" + GetAppActionStr(action_);
        return ret;
    }

    ReclaimParam() = delete;
    ReclaimParam(const ReclaimParam&) = delete;
    ReclaimParam& operator=(const ReclaimParam&) = delete;
    ReclaimParam(ReclaimParam&&) = delete;
    ReclaimParam& operator=(ReclaimParam&&) = delete;
}; // end class ReclaimParam
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_RECLAIM_STRATEGY_RECLAIM_PARAM_H
