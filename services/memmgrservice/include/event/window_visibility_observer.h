/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_MEMORY_MEMMGR_WINDOW_VISIBILITY_OBSERVER_H
#define OHOS_MEMORY_MEMMGR_WINDOW_VISIBILITY_OBSERVER_H

#include <sys/types.h>
#include <map>
#include <mutex>
#include <set>

#include "event_handler.h"
#include "iremote_object.h"
#include "i_mem_mgr.h"
#include "mem_mgr_window_info.h"
#include "reclaim_priority_manager.h"
#include "single_instance.h"

namespace OHOS {
namespace Memory {
struct ProcessWindowVisibilityInfo {
    bool visible;
    int uid;
    std::set<uint32_t> visibleWindowIds;
};

typedef struct ProcessWindowVisibilityInfo ProcessWindowVisibilityInfo;

class WindowVisibilityObserver {
    DECLARE_SINGLE_INSTANCE_BASE(WindowVisibilityObserver);
public:
    void UpdateWindowVisibilityPriority(const std::vector<sptr<MemMgrWindowInfo>> &memMgrWindowInfo);
    void OnProcessDied(int pid);

private:
    WindowVisibilityObserver();
    ~WindowVisibilityObserver();
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    std::function<void()> timerFunc_;
    std::map<int32_t, ProcessWindowVisibilityInfo> windowVisibleMap_;
    std::mutex mutex_ {};

    void SetTimer();
    void CheckMapSize(int type);
    void UpdatePriorityForVisible(std::map<int32_t, ProcessWindowVisibilityInfo> &windowVisibleMap_);
    void UpdateWindowVisibilityPriorityInner(const std::vector<sptr<MemMgrWindowInfo>> &MemMgrWindowInfo);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_WINDOW_VISIBILITY_H
