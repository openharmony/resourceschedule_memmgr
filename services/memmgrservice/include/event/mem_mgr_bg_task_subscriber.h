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

#ifndef OHOS_MEMORY_MEMMGR_BACKGROUND_TASK_SUBSRIBER_H
#define OHOS_MEMORY_MEMMGR_BACKGROUND_TASK_SUBSRIBER_H

#include "background_task_subscriber.h"

namespace OHOS {
namespace Memory {
class MemMgrBgTaskSubscriber : public BackgroundTaskMgr::BackgroundTaskSubscriber {
public:
    virtual void OnConnected() override;
    virtual void OnDisconnected() override;
    virtual void OnTransientTaskStart(const std::shared_ptr<BackgroundTaskMgr::TransientTaskAppInfo>& ttInfo) override;
    virtual void OnTransientTaskEnd(const std::shared_ptr<BackgroundTaskMgr::TransientTaskAppInfo>& ttInfo) override;
    virtual void OnContinuousTaskStart(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo> &ctInfo) override;
    virtual void OnContinuousTaskStop(const std::shared_ptr<BackgroundTaskMgr::ContinuousTaskCallbackInfo> &ctInfo) override;
    virtual void OnRemoteDied(const wptr<IRemoteObject> &object) override;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_BACKGROUND_TASK_SUBSRIBER_H
