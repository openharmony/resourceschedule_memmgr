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
#ifndef OHOS_MEMORY_MEMMGR_KSWAPD_OBSERVER_H
#define OHOS_MEMORY_MEMMGR_KSWAPD_OBSERVER_H

#include "event_handler.h"

struct epoll_event;

namespace OHOS {
namespace Memory {

void HandleKswapdReport();

#define KSWAPD_PRESSURE_FILE "/proc/kswapd_monitor"

class KswapdObserver {
public:
    KswapdObserver();
    ~KswapdObserver();
    void Init();

private:
    // run MainLoop on handler thread
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    int epollfd_ = -1;
    int kswapdMonitorFd_ = -1;
    bool RegisterKswapdListener();
    void MainLoop(void);
    void HandleEventEpollHup(struct epoll_event *curEpollEvent);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_KSWAPD_OBSERVER_H