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

#ifndef OHOS_MEMORY_MEMMGR_MEMORY_LEVEL_MANAGER_H
#define OHOS_MEMORY_MEMMGR_MEMORY_LEVEL_MANAGER_H

#include "event_handler.h"
#include "memory_level_constants.h"
#include "single_instance.h"

namespace OHOS {
namespace Memory {
class AppEntity {
public:
    int uid_;
    std::string name_;
    AppEntity(int id, std::string name) : uid_(id), name_(name) {};
};

class MemoryLevelManager {
    DECLARE_SINGLE_INSTANCE_BASE(MemoryLevelManager);

public:
    void PsiHandler();

private:
    MemoryLevelManager();
    ~MemoryLevelManager() = default;
    void PsiHandlerInner();
    void NotifyMemoryLevel(SystemMemoryInfo &info);
    bool GetEventHandler();
    bool CalcSystemMemoryLevel(SystemMemoryInfo &info);
    bool CalcReclaimAppList(std::vector<std::shared_ptr<AppEntity>> &appList);
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    bool initialized_ = false;
    long calledCount_ = 0;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_MEMORY_LEVEL_MANAGER_H
