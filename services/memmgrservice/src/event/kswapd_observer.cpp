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
#include "kswapd_observer.h"

#include <cerrno>
#include <fcntl.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "memory_level_constants.h"
#ifdef USE_PURGEABLE_MEMORY
#include "purgeable_mem_manager.h"
#endif

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "KswapdObserver";
}

KswapdObserver::KswapdObserver()
{
    MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return,
                 AppExecFwk::EventRunner::Create());
    HILOGI("handler init success!");
}

void KswapdObserver::Init()
{
    epollfd_ = epoll_create(6); // 6 : max epoll events
    if (epollfd_ == -1) {
        HILOGE("epoll_create failed (errno=%{public}d)", errno);
        return;
    }
    HILOGI("epoll_create success epollfd_=<%{public}d>", epollfd_);

    if (!RegisterKswapdListener()) {
        HILOGE("register kswapd pressure failed!");
        return;
    }
    // call MainLoop at handler thread
    if (handler_ != nullptr) {
        std::function<void()> mainLoopFun = std::bind(&KswapdObserver::MainLoop, this);
        handler_->PostImmediateTask(mainLoopFun);
        HILOGD("call MainLoop at handler thread");
    }
}

bool KswapdObserver::RegisterKswapdListener()
{
    struct epoll_event epollEvent;

    // open file
    do {
        kswapdMonitorFd_ = open(KSWAPD_PRESSURE_FILE, O_WRONLY | O_CLOEXEC);
    } while (kswapdMonitorFd_ == -1 && errno == EINTR);
    if (kswapdMonitorFd_ < 0) {
        HILOGE("invalid fd (errno=%{public}d)", errno);
        return false;
    }

    epollEvent.events = EPOLLPRI;
    epollEvent.data.ptr = nullptr;
    epollEvent.data.fd = kswapdMonitorFd_;
    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, kswapdMonitorFd_, &epollEvent) < 0) {
        close(kswapdMonitorFd_);
        kswapdMonitorFd_ = -1;
        HILOGE("failed to add file fd to epoll : errno=%{public}d", errno);
        return false;
    }
    HILOGI("fd for kswapd monitor = %{public}d", kswapdMonitorFd_);
    return true;
}

void KswapdObserver::MainLoop(void)
{
    struct epoll_event *curEpollEvent;

    while (1) {
        HILOGD("waiting for epoll event ...");
        struct epoll_event events[1];
        int i;
        int nevents = epoll_wait(epollfd_, events, 1, -1);
        HILOGD("receive events, num=%{public}d!", nevents);
        if (nevents == -1) {
            if (errno == EINTR) {
                continue;
            }
            HILOGE("failed to wait epoll event(errno=%{public}d)", errno);
            continue;
        }

        for (i = 0, curEpollEvent = &events[0]; i < nevents; ++i, curEpollEvent++) {
            if (curEpollEvent->events & EPOLLHUP) {
                HILOGE("EPOLLHUP in events[%{public}d]", i);
                HandleEventEpollHup(curEpollEvent);
                continue;
            }
            if (curEpollEvent->events & EPOLLERR) {
                HILOGE("epoll err in events[%{public}d]", i);
                continue;
            }
            if ((curEpollEvent->events & EPOLLPRI) && curEpollEvent->data.fd == kswapdMonitorFd_) {
                HandleKswapdReport();
            }
        } // end of for
    } // end of while
}

void KswapdObserver::HandleEventEpollHup(struct epoll_event *curEpollEvent)
{
    if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, curEpollEvent->data.fd, NULL) < 0) {
        HILOGE("Failed to unmonitor for kswapd, errno=%{public}d", errno);
    }
    if (curEpollEvent->data.fd >= 0) {
        close(curEpollEvent->data.fd);
    }
    if (curEpollEvent->data.fd == kswapdMonitorFd_) {
        kswapdMonitorFd_ = -1;
    }
}

void HandleKswapdReport()
{
    HILOGD("called");
#ifdef USE_PURGEABLE_MEMORY
    SystemMemoryInfo info = {MemorySource::KSWAPD, SystemMemoryLevel::MEMORY_LEVEL_LOW};
    PurgeableMemManager::GetInstance().NotifyMemoryLevel(info);
#endif
}

KswapdObserver::~KswapdObserver()
{
    if (kswapdMonitorFd_ >= 0) {
        epoll_ctl(epollfd_, EPOLL_CTL_DEL, kswapdMonitorFd_, NULL);
        close(kswapdMonitorFd_);
    }
    if (epollfd_ >= 0) {
        close(epollfd_);
    }
}
} // namespace Memory
} // namespace OHOS
