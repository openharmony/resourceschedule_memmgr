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

#ifndef OHOS_MEMORY_MEMMGR_NANDLIFE_CONTROLLER_H
#define OHOS_MEMORY_MEMMGR_NANDLIFE_CONTROLLER_H

#include "single_instance.h"
#include "event_handler.h"
#include "memmgr_config_manager.h"

namespace OHOS {
namespace Memory {
class NandLifeController {
    DECLARE_SINGLE_INSTANCE_BASE(NandLifeController);

public:
    bool Init();
private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    NandLifeConfig config_;
    std::function<void()> timerFunc_;

    unsigned long long DAILY_SWAP_OUT_QUOTA_KB;
    unsigned long long TOTAL_SWAP_OUT_QUOTA_KB;

    unsigned long long lastSwapOutKB_ = 0;
    unsigned long long nowSwapOutKB_ = 0;

    unsigned long long minsToday_ = 0;
    unsigned long long swapOutKBToday_ = 0;

    unsigned long long minsSinceBirth_ = 0;
    unsigned long long swapOutKBSinceBirth_ = 0;

    unsigned long long iter = 0;

    NandLifeController();
    bool GetEventHandler();
    bool IsNandLifeParamExist();
    void PrintNandLifeParam();
    bool LoadNandLifeParam();
    bool IsSwapOutClosedPermently();
    bool GetAndValidateNandLifeConfig();
    void SetTimer();
    void CheckSwapOut();
    bool GetSwapOutKBSinceKernelBoot(unsigned long long &ret);
    bool CheckReachedDailyLimit();
    bool CheckReachedTotalLimit();
    bool SetParameterRetry(const std::string &paramName, const std::string &paramValue, int retryTimes);
    bool UpdateNandLifeParam();

    void OpenSwapOutTemporarily(const std::string &reason);
    void CloseSwapOutTemporarily(const std::string &reason);
    void OpenSwapOutPermanently();
    void CloseSwapOutPermanently();
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_NANDLIFE_CONTROLLER_H
