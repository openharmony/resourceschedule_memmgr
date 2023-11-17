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

#include "mem_mgr_proxy.h"

#include "mem_mgr_constant.h"
#include "memmgr_log.h"
#include "parcel.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "MemMgrProxy";
}

int32_t MemMgrProxy::GetBundlePriorityList(BundlePriorityList &bundlePrioList)
{
    HILOGE("called");
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteParcelable(&bundlePrioList)) {
        HILOGE("write bundlePrioList failed");
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_BUNDLE_PRIORITY_LIST), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    std::shared_ptr<BundlePriorityList> list
        = std::shared_ptr<BundlePriorityList>(reply.ReadParcelable<BundlePriorityList>());
    if (list == nullptr) {
        return -1;
    }
    bundlePrioList = *list;
    return ERR_OK;
}

int32_t MemMgrProxy::NotifyDistDevStatus(int32_t pid, int32_t uid, const std::string &name, bool connected)
{
    HILOGI("called, pid=%{public}d, uid=%{public}d, name=%{public}s, connected=%{public}d", pid, uid, name.c_str(),
        connected);
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(pid) || !data.WriteInt32(uid) || !data.WriteString(name) || !data.WriteBool(connected)) {
        HILOGE("write params failed");
        return ERR_INVALID_DATA;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_NOTIFY_DIST_DEV_STATUS), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    return ret;
}

int32_t MemMgrProxy::GetKillLevelOfLmkd(int32_t &killLevel)
{
    HILOGI("called");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return ERR_NULL_OBJECT;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_KILL_LEVEL_OF_LMKD), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }

    int32_t curKillLevel = 0;
    if (!reply.ReadInt32(curKillLevel)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    killLevel = curKillLevel;
    return ERR_OK;
}

#ifdef USE_PURGEABLE_MEMORY
int32_t MemMgrProxy::RegisterActiveApps(int32_t pid, int32_t uid)
{
    HILOGI("called, pid=%{public}d, uid=%{public}d", pid, uid);
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(pid) || !data.WriteInt32(uid)) {
        HILOGE("write params failed");
        return ERR_INVALID_DATA;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_REGISTER_ACTIVE_APPS), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    return ret;
}

int32_t MemMgrProxy::DeregisterActiveApps(int32_t pid, int32_t uid)
{
    HILOGI("called, pid=%{public}d, uid=%{public}d", pid, uid);
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(pid) || !data.WriteInt32(uid)) {
        HILOGE("write params failed");
        return ERR_INVALID_DATA;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_DEREGISTER_ACTIVE_APPS), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    return ret;
}

int32_t MemMgrProxy::SubscribeAppState(const sptr<IAppStateSubscriber> &subscriber)
{
    HILOGI("called");
    if (subscriber == nullptr) {
        HILOGE("subscriber is null");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(subscriber->AsObject())) {
        HILOGE("write subscriber failed");
        return ERR_INVALID_DATA;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_SUBSCRIBE_APP_STATE), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    return ret;
}

int32_t MemMgrProxy::UnsubscribeAppState(const sptr<IAppStateSubscriber> &subscriber)
{
    HILOGI("called");
    if (subscriber == nullptr) {
        HILOGE("subscriber is null");
        return ERR_NULL_OBJECT;
    }
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteRemoteObject(subscriber->AsObject())) {
        HILOGE("write subscriber failed");
        return ERR_INVALID_DATA;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_UNSUBSCRIBE_APP_STATE), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    return ret;
}

int32_t MemMgrProxy::GetAvailableMemory(int32_t &memSize)
{
    HILOGI("called");
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_AVAILABLE_MEMORY), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    if (!reply.ReadInt32(memSize)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    return ERR_OK;
}

int32_t MemMgrProxy::GetTotalMemory(int32_t &memSize)
{
    HILOGI("called");
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_TOTAL_MEMORY), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    if (!reply.ReadInt32(memSize)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    return ERR_OK;
}
#endif // USE_PURGEABLE_MEMORY

int32_t MemMgrProxy::OnWindowVisibilityChanged(const std::vector<sptr<MemMgrWindowInfo>> &MemMgrWindowInfo)
{
    HILOGD("called");
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(MemMgrWindowInfo.size()))) {
        HILOGE("write MemMgrWindowInfo size failed");
        return ERR_INVALID_DATA;
    }
    for (auto &info : MemMgrWindowInfo) {
        if (!data.WriteParcelable(info)) {
            HILOGE("write MemMgrWindowInfo failed");
            return ERR_INVALID_DATA;
        }
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_ON_WINDOW_VISIBILITY_CHANGED), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }
    int32_t ret;
    if (!reply.ReadInt32(ret)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    return ret;
}

int32_t MemMgrProxy::GetReclaimPriorityByPid(int32_t pid, int32_t &priority)
{
    HILOGD("called");
    sptr<IRemoteObject> remote = Remote();
    MessageParcel data;
    if (!data.WriteInterfaceToken(IMemMgr::GetDescriptor())) {
        HILOGE("write interface token failed");
        return ERR_FLATTEN_OBJECT;
    }
    if (!data.WriteInt32(pid)) {
        HILOGE("write pid failed");
        return ERR_INVALID_DATA;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t error = remote->SendRequest(
        static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_PRIORITY_BY_PID), data, reply, option);
    if (error != ERR_NONE) {
        HILOGE("transact failed, error: %{public}d", error);
        return error;
    }

    int32_t curPriority = RECLAIM_PRIORITY_UNKNOWN + 1;
    if (!reply.ReadInt32(curPriority)) {
        HILOGE("read result failed");
        return IPC_PROXY_ERR;
    }
    priority = curPriority;
    return ERR_OK;
}
} // namespace Memory
} // namespace OHOS
