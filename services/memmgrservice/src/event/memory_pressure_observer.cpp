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

#include "memory_pressure_observer.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "low_memory_killer.h"
#include "memory_level_manager.h"

#include <string>
#include <sys/epoll.h>
#include <cerrno>
#include <unistd.h>
#include <sstream>

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemoryPressureObserver";
const int MAX_CMD_LINE_LENGTH = 256;
}

MemoryPressureObserver::MemoryPressureObserver()
{
    HILOGI("called");
    MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return,
        AppExecFwk::EventRunner::Create());
    HILOGE("handler init success!");
}

void MemoryPressureObserver::Init()
{
    HILOGI("called");
    epollfd_ = epoll_create(6); // 6 : max epoll events
    if (epollfd_ == -1) {
        HILOGE("epoll_create failed (errno=%{public}d)", errno);
        return;
    }
    HILOGI("epoll_create success epollfd_=<%{public}d>", epollfd_);

    if (!MonitorLevel(MemPressureLevel::LEVEL_0)) {
        HILOGE("register memory pressure low level failed!");
        return;
    }
    // call MainLoop at handler thread
    handler_->PostImmediateTask([this] { this->MainLoop(); });
    HILOGI("call MainLoop at handler thread");
}

bool MemoryPressureObserver::MonitorLevel(MemPressureLevel level)
{
    int fd = CreateLevelFileFd(levelConfigArr[level].stallType,
                               levelConfigArr[level].thresholdInMs * US_PER_MS,
                               WINDOW_IN_MS * US_PER_MS);
    HILOGI("fd for level %{public}d = %{public}d", level, fd);
    if (fd < 0) {
        return false;
    }

    levelConfigArr[level].levelHandler.handler = HandleLevelReport;
    levelConfigArr[level].levelHandler.data = level;
    if (AddLevelFileFdToEpoll(epollfd_, fd, &levelConfigArr[level].levelHandler) < 0) {
        CloseLevelFileFd(fd);
        levelConfigArr[level].levelFileFd = -1;
        return false;
    }
    curLevelCount_++;
    levelConfigArr[level].levelFileFd = fd;

    return true;
}

int MemoryPressureObserver::CreateLevelFileFd(StallType stallType, int thresholdInUs, int windowInUs)
{
    int fd = -1;
    int res = -1;
    std::string cmdStr;
    std::stringstream ss;

    // determine whether it is a valid StallType
    if (stallType != SOME && stallType != FULL) {
        HILOGE("invalid stall type: %{public}d", stallType);
        errno = EINVAL;
        goto err;
    }

    // open file
    do {
        fd = open(MEMORY_PRESSURE_FILE, O_WRONLY | O_CLOEXEC);
    } while (fd == -1 && errno == EINTR);
    if (fd < 0) {
        HILOGE("invalid fd (errno=%{public}d)", errno);
        return -1;
    }

    // make monitor parameters
    ss << (stallType == SOME ? "some" : "full") << " " << thresholdInUs << " " << windowInUs;
    cmdStr = ss.str();
    if (cmdStr.length() > MAX_CMD_LINE_LENGTH) {
        HILOGE("cmd too long");
        errno = EINVAL;
        goto err;
    }
    HILOGI("prepare to write to fd : <%{public}s>", cmdStr.c_str());

    // write monitor parameters
    do {
        res = write(fd, cmdStr.c_str(), cmdStr.length() + 1);
    } while (res == -1 && errno == EINTR);
    if (res < 0) {
        HILOGE("write failed (errno=%{public}d)", errno);
        goto err;
    }

    return fd;

err:
    if (fd > 0) {
        close(fd);
    }
    return -1;
}

int MemoryPressureObserver::AddLevelFileFdToEpoll(int epollfd, int fd, void* data)
{
    int ret;
    struct epoll_event epollEvent;

    epollEvent.events = EPOLLPRI;
    epollEvent.data.ptr = data;
    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epollEvent);
    if (ret < 0) {
        HILOGE("failed to add file fd to epoll ; errno=%{public}d", errno);
    }
    return ret;
}

void MemoryPressureObserver::MainLoop(void)
{
    HILOGE("enter");
    struct epoll_event *curEpollEvent;

    while (1) {
        HILOGD("waiting for epoll event ...");
        struct epoll_event events[curLevelCount_];
        int i;
        int nevents = epoll_wait(epollfd_, events, curLevelCount_, -1);
        if (nevents == -1) {
            if (errno == EINTR)
                continue;
            HILOGE("failed to wait epoll event(errno=%{public}d)", errno);
            continue;
        }

        for (i = 0, curEpollEvent = &events[0]; i < nevents; ++i, curEpollEvent++) {
            if ((curEpollEvent->events & EPOLLHUP) && curEpollEvent->data.ptr) {
                HILOGE("disconnected!");
                handlerInfo_ = (struct LevelHandler*)curEpollEvent->data.ptr;
                int level = handlerInfo_->data;
                UnMonitorLevel(levelConfigArr[level].level);
                continue;
            }
            if (curEpollEvent->events & EPOLLERR)
                HILOGE("epoll err in events[%{public}d]", i);
            if (curEpollEvent->data.ptr) {
                HandleEpollEvent(curEpollEvent);
            }
        } // end of for
    } // end of while
}

void MemoryPressureObserver::HandleEpollEvent(struct epoll_event *curEpollEvent)
{
    handlerInfo_ = (struct LevelHandler*)curEpollEvent->data.ptr;
    HILOGD("#2 call handler");
    handlerInfo_->handler(handlerInfo_->data, curEpollEvent->events);
}

void HandleLevelReport(int level, uint32_t events)
{
    HILOGI("level=%{public}d !", level);
    MemoryLevelManager::GetInstance().PsiHandler();
    LowMemoryKiller::GetInstance().PsiHandler();
}

MemoryPressureObserver::~MemoryPressureObserver()
{
    HILOGI("called");
    UnMonitorLevel(MemPressureLevel::LEVEL_0);
    if (epollfd_ >= 0) {
        close(epollfd_);
    }
}

void MemoryPressureObserver::UnMonitorLevel(MemPressureLevel level)
{
    int fd = levelConfigArr[level].levelFileFd;

    if (delLevelFileFdFromEpoll(epollfd_, fd) < 0) {
        HILOGE("Failed to unmonitor for level %{public}d, errno=%{public}d",
            level, errno);
    }
    CloseLevelFileFd(fd);
    levelConfigArr[level].levelFileFd = -1;
    curLevelCount_--;
}

int MemoryPressureObserver::delLevelFileFdFromEpoll(int epollfd, int fd)
{
    return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
}

void MemoryPressureObserver::CloseLevelFileFd(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}
} // namespace Memory
} // namespace OHOS
