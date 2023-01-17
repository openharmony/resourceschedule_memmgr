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

#include "mem_mgr_stub.h"
#include "memmgr_log.h"
#include "memmgr_config_manager.h"
#include "kernel_interface.h"
#include "low_memory_killer.h"

namespace OHOS {
namespace Memory {
namespace {
    const std::string TAG = "MemMgrStub";
}

MemMgrStub::MemMgrStub()
{
    memberFuncMap_[static_cast<uint32_t>(IMemMgr::MEM_MGR_GET_BUNDLE_PRIORITY_LIST)] =
        &MemMgrStub::HandleGetBunldePriorityList;
    memberFuncMap_[static_cast<uint32_t>(IMemMgr::MEM_MGR_NOTIFY_DIST_DEV_STATUS)] =
        &MemMgrStub::HandleNotifyDistDevStatus;
    memberFuncMap_[static_cast<uint32_t>(IMemMgr::MEM_MGR_GET_KILL_LEVEL_OF_LMKD)] =
        &MemMgrStub::HandleGetKillLevelOfLmkd;
#ifdef USE_PURGEABLE_MEMORY
    memberFuncMap_[static_cast<uint32_t>(IMemMgr::MEM_MGR_REGISTER_ACTIVE_APPS)] =
        &MemMgrStub::HandleRegisterActiveApps;
    memberFuncMap_[static_cast<uint32_t>(IMemMgr::MEM_MGR_DEREGISTER_ACTIVE_APPS)] =
        &MemMgrStub::HandleDeregisterActiveApps;
    memberFuncMap_[static_cast<uint32_t>(IMemMgr::MEM_MGR_SUBSCRIBE_APP_STATE)] =
        &MemMgrStub::HandleSubscribeAppState;
    memberFuncMap_[static_cast<uint32_t>(IMemMgr::MEM_MGR_UNSUBSCRIBE_APP_STATE)] =
        &MemMgrStub::HandleUnsubscribeAppState;
    memberFuncMap_[static_cast<uint32_t>(IMemMgr::MEM_MGR_GET_AVAILABLE_MEMORY)] =
        &MemMgrStub::HandleGetAvailableMemory;
#endif
}

MemMgrStub::~MemMgrStub()
{
    memberFuncMap_.clear();
}

int MemMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    HILOGI("MemMgrStub::OnReceived, code = %{public}d, flags= %{public}d.", code, option.GetFlags());
    std::u16string descriptor = MemMgrStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOGE("local descriptor is not equal to remote");
        return ERR_INVALID_STATE;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t MemMgrStub::HandleGetBunldePriorityList(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("called");
    std::shared_ptr<BundlePriorityList> list
        = std::shared_ptr<BundlePriorityList>(data.ReadParcelable<BundlePriorityList>());

    if (!list) {
        HILOGE("BundlePriorityList ReadParcelable failed");
        return -1;
    }
    int32_t ret = GetBundlePriorityList(*list);
    reply.WriteParcelable(list.get());
    return ret;
}

int32_t MemMgrStub::HandleNotifyDistDevStatus(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("called");
    int32_t pid = 0;
    int32_t uid = 0;
    std::string name;
    bool connected;
    if (!data.ReadInt32(pid) || !data.ReadInt32(uid) || !data.ReadString(name) || !data.ReadBool(connected)) {
        HILOGE("read params failed");
        return IPC_STUB_ERR;
    }
    HILOGI("called, pid=%{public}d, uid=%{public}d, name=%{public}s, connected=%{public}d", pid, uid, name.c_str(),
        connected);

    int32_t ret = NotifyDistDevStatus(pid, uid, name, connected);
    if (!reply.WriteInt32(ret)) {
        return IPC_STUB_ERR;
    }
    return ret;
}

int32_t MemMgrStub::HandleGetKillLevelOfLmkd(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("called");
    int32_t killLevel = LowMemoryKiller::GetInstance().GetKillLevel();
    if (!reply.WriteInt32(killLevel)) {
        return IPC_STUB_ERR;
    }
    return 0;
}

#ifdef USE_PURGEABLE_MEMORY
int32_t MemMgrStub::HandleRegisterActiveApps(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = 0;
    int32_t uid = 0;
    if (!data.ReadInt32(pid) || !data.ReadInt32(uid)) {
        HILOGE("read params failed");
        return IPC_STUB_ERR;
    }
    HILOGI("called, pid=%{public}d, uid=%{public}d", pid, uid);
    int32_t ret = RegisterActiveApps(pid, uid);
    if (!reply.WriteInt32(ret)) {
        return IPC_STUB_ERR;
    }
    return ret;
}

int32_t MemMgrStub::HandleDeregisterActiveApps(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = 0;
    int32_t uid = 0;
    if (!data.ReadInt32(pid) || !data.ReadInt32(uid)) {
        HILOGE("read params failed");
        return IPC_STUB_ERR;
    }
    HILOGI("called, pid=%{public}d, uid=%{public}d", pid, uid);
    int32_t ret = DeregisterActiveApps(pid, uid);
    if (!reply.WriteInt32(ret)) {
        return IPC_STUB_ERR;
    }
    return ret;
}

int32_t MemMgrStub::HandleSubscribeAppState(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("called");
    int32_t pid = 0;
    int32_t uid = 0;
    sptr<IRemoteObject> subscriber = data.ReadRemoteObject();
    if (!data.ReadInt32(pid) || !data.ReadInt32(uid)) {
        HILOGE("read params failed");
        return IPC_STUB_ERR;
    }
    int32_t ret = SubscribeAppState(iface_cast<IAppStateSubscriber>(subscriber));
    if (!reply.WriteInt32(ret)) {
        return IPC_STUB_ERR;
    }
    return ret;
}

int32_t MemMgrStub::HandleUnsubscribeAppState(MessageParcel &data, MessageParcel &reply)
{
    HILOGI("called");
    sptr<IRemoteObject> subscriber = data.ReadRemoteObject();
    if (subscriber == nullptr) {
        HILOGE("read params failed");
        return IPC_STUB_ERR;
    }

    int32_t ret = UnsubscribeAppState(iface_cast<IAppStateSubscriber>(subscriber));
    if (!reply.WriteInt32(ret)) {
        return IPC_STUB_ERR;
    }
    return ret;
}

int32_t MemMgrStub::HandleGetAvailableMemory(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = GetAvailableMemory();
    if (!reply.WriteInt32(ret)) {
        return IPC_STUB_ERR;
    }
    return ret;
}
#endif // USE_PURGEABLE_MEMORY
} // namespace Memory
} // namespace OHOS
