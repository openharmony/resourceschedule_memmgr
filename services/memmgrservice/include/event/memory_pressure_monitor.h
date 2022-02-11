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

#ifndef OHOS_MEMORY_MEMMGR_MEMORY_PRESSURE_MONITOR_H
#define OHOS_MEMORY_MEMMGR_MEMORY_PRESSURE_MONITOR_H

#include "event_handler.h"
#include <sys/epoll.h>

#define MS_PER_SECOND 1000

#define NS_PER_MS 1000000

#define US_PER_MS 1000

#define WINDOW_IN_MS 1000

#define MEMORY_PRESSURE_FILE "/proc/pressure/memory"

#define POLL_PERIOD_MS 10

#define POLL_COUNT (WINDOW_IN_MS / POLL_PERIOD_MS)

namespace OHOS {
namespace Memory {
struct MemPressCallback {
    std::function<void(const int &level)> OnMemPressLevelUploaded;
};

enum MemPressureLevel {
    LEVEL_0 = 0,
    MEM_PRESS_LEVEL_COUNT
};

enum StallType {
    SOME,
    FULL,
    STALL_TYPE_COUNT
};

struct LevelHandler {
    int data;
    void (*handler)(int data, uint32_t events);
};

struct MemPressLevelCfg {
    enum MemPressureLevel level;
    enum StallType stallType;
    int thresholdInMs;
    int levelFileFd;
    struct LevelHandler levelHandler;
};

static struct MemPressLevelCfg levelConfigArr[MEM_PRESS_LEVEL_COUNT] = {
    { LEVEL_0, SOME, 70, -1},    /* 70ms out of 1sec for partial stall */
};


void HandleLevelReport(int level, uint32_t events);

class MemoryPressureMonitor {
public:
    MemoryPressureMonitor(const MemPressCallback &callback);
    ~MemoryPressureMonitor();
private:
    // callback is used to call event center (and is unused now)
    MemPressCallback callback_;
    // run MainLoop on handler thread
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
    int epollfd_;
    // current monitor level count
    int curLevelCount_;
    int polling_ = 0;

    struct timespec lastTime_;
    struct timespec currentTime_;
    struct LevelHandler *handlerInfo_;
    struct LevelHandler *pollHandler_ = NULL;

    void Init();

    bool MonitorLevel(MemPressureLevel level);
    int CreateLevelFileFd(StallType stallType, int thresholdInUs, int windowInUs);
    int AddLevelFileFdToEpoll(int epollfd, int fd, void* data);
    void UnMonitorLevel(MemPressureLevel level);
    int delLevelFileFdFromEpoll(int epollfd, int fd);
    void CloseLevelFileFd(int fd);
    void MainLoop(void);
    void HandleEpollEvent(struct epoll_event *curEpollEvent);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_MEMORY_PRESSURE_MONITOR_H
