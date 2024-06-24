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

#include "window_visibility_observer.h"

#include "event_handler.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "singleton.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "WindowVisibilityObserver";
const std::string WINDOW_OBSERVER_HANDLER = "WindowVisibilityObserverHandler";
constexpr int TIMER_PEROID_MIN = 5;
constexpr int TIMER_PEROID_MS = TIMER_PEROID_MIN * 60 * 1000;
constexpr int TRIGGER_BY_TIME = 1;
constexpr int TRIGGER_BY_SIZE = 2;
}

IMPLEMENT_SINGLE_INSTANCE(WindowVisibilityObserver);

WindowVisibilityObserver::WindowVisibilityObserver()
{
    if (!handler_) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return,
                     AppExecFwk::EventRunner::Create());
    }
    SetTimer();
}

void WindowVisibilityObserver::OnProcessDied(int pid)
{
    HILOGD("called");
    std::lock_guard<std::mutex> lock(mutex_);
    windowVisibleMap_.erase(pid);
    HILOGD("remove pid=%{public}d from WindowVisibilityObserver", pid);
}

void WindowVisibilityObserver::UpdateWindowVisibilityPriority(
    const std::vector<sptr<MemMgrWindowInfo>> &memMgrWindowInfo)
{
    HILOGD("called");
    handler_->PostImmediateTask([this, memMgrWindowInfo] {
        this->UpdateWindowVisibilityPriorityInner(memMgrWindowInfo);
    });
}

void WindowVisibilityObserver::UpdateWindowVisibilityPriorityInner(
    const std::vector<sptr<MemMgrWindowInfo>> &MemMgrWindowInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &info : MemMgrWindowInfo) {
        if (!info) {
            continue;
        }
        auto windowId = info->windowId_;
        auto isVisible = info->isVisible_;
        auto pid = info->pid_;
        auto uid = info->uid_;
        HILOGI("MemMgrWindowInfo[pid=%{public}d, uid=%{public}d, winId=%{public}d, visible=%{public}d]",
            pid, uid, windowId, isVisible);
        if (isVisible) {
            auto windowInfoPtr = windowVisibleMap_.find(pid);
            if (windowInfoPtr != windowVisibleMap_.end()) {
                windowInfoPtr->second.visibleWindowIds.insert(windowId);
            } else {
                ProcessWindowVisibilityInfo info;
                info.uid = uid;
                info.visible = false;
                info.visibleWindowIds.insert(windowId);
                windowVisibleMap_.insert(std::make_pair(pid, info));
            }
        } else {
            auto windowInfoPtr = windowVisibleMap_.find(pid);
            if (windowInfoPtr != windowVisibleMap_.end()) {
                windowInfoPtr->second.visibleWindowIds.erase(windowId);
            }
        }
    }

    HILOGI("windowVisibleMap_size=%{public}zu", windowVisibleMap_.size());
    UpdatePriorityForVisible(windowVisibleMap_);
    if (windowVisibleMap_.size() >= 2048) { //2048: max process num
        if (handler_) {
            handler_->PostImmediateTask([this] { this->CheckMapSize(TRIGGER_BY_SIZE); });
        }
    }
}

void WindowVisibilityObserver::UpdatePriorityForVisible(
    std::map<int32_t, ProcessWindowVisibilityInfo> &windowVisibleMap_)
{
    std::vector<unsigned int> toBeDeleted;
    ReclaimHandleRequest request;
    for (auto &pair : windowVisibleMap_) {
        ProcessWindowVisibilityInfo &info = pair.second;
        if (info.visibleWindowIds.size() > 0 && info.visible == false) {
            info.visible = true;
            ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(
                SingleRequest({pair.first, info.uid, "", ""}, AppStateUpdateReason::VISIBLE));
        } else if (info.visibleWindowIds.size() == 0) {
            if (info.visible == true) {
                info.visible = false;
                ReclaimPriorityManager::GetInstance().UpdateReclaimPriority(
                    SingleRequest({pair.first, info.uid, "", ""}, AppStateUpdateReason::UN_VISIBLE));
            }
            toBeDeleted.push_back(pair.first);
        }
        HILOGD("ProcessWindowVisibilityInfo[pid=%{public}d, uid=%{public}d, vWins=%{public}zu,"
            "visible=%{public}d]", pair.first, info.uid, info.visibleWindowIds.size(), info.visible);
    }
    for (auto pid : toBeDeleted) {
        windowVisibleMap_.erase(pid);
    }
}

void WindowVisibilityObserver::SetTimer()
{
    //set timer and call CheckMapSize each TIMER_PEROID_MIN min.
    handler_->PostTask([this] { this->CheckMapSize(TRIGGER_BY_TIME); },
        TIMER_PEROID_MS, AppExecFwk::EventQueue::Priority::HIGH);
    HILOGD("set timer after %{public}d mins", TIMER_PEROID_MIN);
}

void WindowVisibilityObserver::CheckMapSize(int type)
{
    HILOGD("called");

    std::vector<unsigned int> alivePids;
    if (!KernelInterface::GetInstance().GetAllProcPids(alivePids)) {
        return;
    }

    std::vector<unsigned int> toBeDeleted;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &pair : windowVisibleMap_) {
        auto pid = pair.first;
        if (std::find(alivePids.begin(), alivePids.end(), pid) == alivePids.end()) {
            toBeDeleted.push_back(pid);
        }
    }

    for (auto pid : toBeDeleted) {
        windowVisibleMap_.erase(pid);
    }

    if (type == TRIGGER_BY_TIME) {
        SetTimer();
    }
}
} // namespace Memory
} // namespace OHOS
