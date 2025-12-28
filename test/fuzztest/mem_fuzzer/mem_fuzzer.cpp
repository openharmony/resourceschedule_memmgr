/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "mem_fuzzer.h"

namespace OHOS {
namespace Memory {

namespace {
// String length limits for fuzzing - avoid excessive memory allocation
constexpr size_t FUZZ_MAX_NAME_LENGTH = 128;
} // namespace

/**
 * @brief Write interface token to MessageParcel
 * Uses IMemMgr::GetDescriptor() to avoid hardcoding
 */
static bool WriteInterfaceToken(MessageParcel& data)
{
    return data.WriteInterfaceToken(IMemMgr::GetDescriptor());
}

/**
 * @brief Fuzz GetBundlePriorityList IPC handler
 * Tests: HandleGetBunldePriorityList in mem_mgr_stub.cpp
 */
static bool FuzzGetBundlePriorityList(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    // Construct BundlePriorityList parcel data
    int32_t count = provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_MAX_BUNDLE_COUNT);
    data.WriteInt32(count);

    for (int32_t i = 0; i < count && provider.HasEnoughData(sizeof(int32_t)); ++i) {
        data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // uid
        data.WriteString(provider.ConsumeString(FUZZ_MAX_NAME_LENGTH));  // name
        data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // priority
        data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // accountId
    }

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_BUNDLE_PRIORITY_LIST);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz NotifyDistDevStatus IPC handler
 * Tests: HandleNotifyDistDevStatus in mem_mgr_stub.cpp
 */
static bool FuzzNotifyDistDevStatus(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // pid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // uid
    data.WriteString(provider.ConsumeString(FUZZ_MAX_NAME_LENGTH));  // name
    data.WriteBool(provider.ConsumeBool());  // connected

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_NOTIFY_DIST_DEV_STATUS);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz GetKillLevelOfLmkd IPC handler
 * Tests: HandleGetKillLevelOfLmkd in mem_mgr_stub.cpp
 */
static bool FuzzGetKillLevelOfLmkd(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_KILL_LEVEL_OF_LMKD);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

#ifdef USE_PURGEABLE_MEMORY
/**
 * @brief Fuzz RegisterActiveApps IPC handler
 * Tests: HandleRegisterActiveApps in mem_mgr_stub.cpp
 */
static bool FuzzRegisterActiveApps(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // pid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // uid

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_REGISTER_ACTIVE_APPS);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz DeregisterActiveApps IPC handler
 * Tests: HandleDeregisterActiveApps in mem_mgr_stub.cpp
 */
static bool FuzzDeregisterActiveApps(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // pid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // uid

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_DEREGISTER_ACTIVE_APPS);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz GetAvailableMemory IPC handler
 * Tests: HandleGetAvailableMemory in mem_mgr_stub.cpp
 */
static bool FuzzGetAvailableMemory(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_AVAILABLE_MEMORY);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz GetTotalMemory IPC handler
 * Tests: HandleGetTotalMemory in mem_mgr_stub.cpp
 */
static bool FuzzGetTotalMemory(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_TOTAL_MEMORY);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}
#endif // USE_PURGEABLE_MEMORY

/**
 * @brief Fuzz OnWindowVisibilityChanged IPC handler
 * Tests: HandleOnWindowVisibilityChanged in mem_mgr_stub.cpp
 */
static bool FuzzOnWindowVisibilityChanged(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    uint32_t count = provider.ConsumeIntegralInRange<uint32_t>(0, FUZZ_MAX_WINDOW_INFO_COUNT);
    data.WriteUint32(count);

    for (uint32_t i = 0; i < count && provider.HasEnoughData(sizeof(uint32_t)); ++i) {
        // Write MemMgrWindowInfo parcel format: windowId, pid, uid, isVisible
        data.WriteUint32(provider.ConsumeIntegral<uint32_t>());  // windowId
        data.WriteInt32(provider.ConsumeIntegral<int32_t>());    // pid
        data.WriteInt32(provider.ConsumeIntegral<int32_t>());    // uid
        data.WriteBool(provider.ConsumeBool());                   // isVisible
    }

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_ON_WINDOW_VISIBILITY_CHANGED);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz GetReclaimPriorityByPid IPC handler
 * Tests: HandleGetReclaimPriorityByPid in mem_mgr_stub.cpp
 */
static bool FuzzGetReclaimPriorityByPid(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // pid

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_PRIORITY_BY_PID);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz NotifyProcessStateChangedSync IPC handler
 * Tests: HandleNotifyProcessStateChangedSync in mem_mgr_stub.cpp
 */
static bool FuzzNotifyProcessStateChangedSync(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    // Write MemMgrProcessStateInfo parcel format
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // callerPid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // callerUid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // pid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // uid
    data.WriteUint32(provider.ConsumeIntegral<uint32_t>());  // reason

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_NOTIFY_PROCESS_STATE_CHANGED_SYNC);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz NotifyProcessStateChangedAsync IPC handler
 * Tests: HandleNotifyProcessStateChangedAsync in mem_mgr_stub.cpp
 */
static bool FuzzNotifyProcessStateChangedAsync(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    // Write MemMgrProcessStateInfo parcel format
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // callerPid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // callerUid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // pid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // uid
    data.WriteUint32(provider.ConsumeIntegral<uint32_t>());  // reason

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_NOTIFY_PROCESS_STATE_CHANGED_ASYNC);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz NotifyProcessStatus IPC handler
 * Tests: HandleNotifyProcessStatus in mem_mgr_stub.cpp
 */
static bool FuzzNotifyProcessStatus(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // pid
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // type
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // status
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // saId

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_NOTIFY_PROCESS_STATUS);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz SetCritical IPC handler
 * Tests: HandleSetCritical in mem_mgr_stub.cpp
 */
static bool FuzzSetCritical(FuzzDataProvider& provider)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return false;
    }

    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // pid
    data.WriteBool(provider.ConsumeBool());                 // critical
    data.WriteInt32(provider.ConsumeIntegral<int32_t>());  // saId

    uint32_t code = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_SET_CRITICAL);
    MemMgrService::GetInstance().OnRemoteRequest(code, data, reply, option);
    return true;
}

/**
 * @brief Fuzz IPC stub with random code selection
 * Routes to appropriate handler based on fuzz data
 */
bool FuzzIPCStub(FuzzDataProvider& provider)
{
    if (!provider.HasEnoughData(FUZZ_THRESHOLD_FOR_IPC)) {
        return false;
    }

    // Select IPC code from valid range using enum values
    uint32_t code = provider.ConsumeIntegralInRange<uint32_t>(FUZZ_IPC_CODE_MIN, FUZZ_IPC_CODE_MAX);

    switch (static_cast<MemMgrInterfaceCode>(code)) {
        case MemMgrInterfaceCode::MEM_MGR_GET_BUNDLE_PRIORITY_LIST:
            return FuzzGetBundlePriorityList(provider);
        case MemMgrInterfaceCode::MEM_MGR_NOTIFY_DIST_DEV_STATUS:
            return FuzzNotifyDistDevStatus(provider);
        case MemMgrInterfaceCode::MEM_MGR_GET_KILL_LEVEL_OF_LMKD:
            return FuzzGetKillLevelOfLmkd(provider);
#ifdef USE_PURGEABLE_MEMORY
        case MemMgrInterfaceCode::MEM_MGR_REGISTER_ACTIVE_APPS:
            return FuzzRegisterActiveApps(provider);
        case MemMgrInterfaceCode::MEM_MGR_DEREGISTER_ACTIVE_APPS:
            return FuzzDeregisterActiveApps(provider);
        case MemMgrInterfaceCode::MEM_MGR_GET_AVAILABLE_MEMORY:
            return FuzzGetAvailableMemory(provider);
        case MemMgrInterfaceCode::MEM_MGR_GET_TOTAL_MEMORY:
            return FuzzGetTotalMemory(provider);
#endif
        case MemMgrInterfaceCode::MEM_MGR_ON_WINDOW_VISIBILITY_CHANGED:
            return FuzzOnWindowVisibilityChanged(provider);
        case MemMgrInterfaceCode::MEM_MGR_GET_PRIORITY_BY_PID:
            return FuzzGetReclaimPriorityByPid(provider);
        case MemMgrInterfaceCode::MEM_MGR_NOTIFY_PROCESS_STATE_CHANGED_SYNC:
            return FuzzNotifyProcessStateChangedSync(provider);
        case MemMgrInterfaceCode::MEM_MGR_NOTIFY_PROCESS_STATE_CHANGED_ASYNC:
            return FuzzNotifyProcessStateChangedAsync(provider);
        case MemMgrInterfaceCode::MEM_MGR_NOTIFY_PROCESS_STATUS:
            return FuzzNotifyProcessStatus(provider);
        case MemMgrInterfaceCode::MEM_MGR_SET_CRITICAL:
            return FuzzSetCritical(provider);
        default:
            return false;
    }
}

/**
 * @brief Fuzz MemMgrWindowInfo Unmarshalling
 * Tests: MemMgrWindowInfo::Unmarshalling in mem_mgr_window_info.cpp
 */
bool FuzzParcelableWindowInfo(FuzzDataProvider& provider)
{
    if (!provider.HasEnoughData(FUZZ_THRESHOLD_FOR_PARCELABLE)) {
        return false;
    }

    MessageParcel parcel;
    // Write raw fuzz data in expected format
    parcel.WriteUint32(provider.ConsumeIntegral<uint32_t>());  // windowId
    parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());    // pid
    parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());    // uid
    parcel.WriteBool(provider.ConsumeBool());                   // isVisible

    // Attempt to unmarshall
    MemMgrWindowInfo* info = MemMgrWindowInfo::Unmarshalling(parcel);
    if (info != nullptr) {
        delete info;
    }
    return true;
}

/**
 * @brief Fuzz MemMgrProcessStateInfo Unmarshalling
 * Tests: MemMgrProcessStateInfo::Unmarshalling in mem_mgr_process_state_info.cpp
 */
bool FuzzParcelableProcessStateInfo(FuzzDataProvider& provider)
{
    if (!provider.HasEnoughData(FUZZ_THRESHOLD_FOR_PARCELABLE)) {
        return false;
    }

    MessageParcel parcel;
    // Write raw fuzz data in expected format
    parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());    // callerPid
    parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());    // callerUid
    parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());    // pid
    parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());    // uid
    parcel.WriteUint32(provider.ConsumeIntegral<uint32_t>());  // reason

    // Attempt to unmarshall
    MemMgrProcessStateInfo* info = MemMgrProcessStateInfo::Unmarshalling(parcel);
    if (info != nullptr) {
        delete info;
    }
    return true;
}

/**
 * @brief Fuzz BundlePriorityList Unmarshalling
 * Tests: BundlePriorityList::Unmarshalling in bundle_priority_list.cpp
 */
bool FuzzParcelableBundlePriorityList(FuzzDataProvider& provider)
{
    if (!provider.HasEnoughData(sizeof(int32_t))) {
        return false;
    }

    MessageParcel parcel;
    
    // Write count - use smaller range to avoid excessive loop iterations
    int32_t count = provider.ConsumeIntegralInRange<int32_t>(0, FUZZ_MAX_BUNDLE_COUNT);
    parcel.WriteInt32(count);

    // Write bundle entries
    for (int32_t i = 0; i < count && provider.HasEnoughData(sizeof(int32_t)); ++i) {
        parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());  // uid
        parcel.WriteString(provider.ConsumeString(FUZZ_MAX_NAME_LENGTH));  // name
        parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());  // priority
        parcel.WriteInt32(provider.ConsumeIntegral<int32_t>());  // accountId
    }

    // Attempt to unmarshall
    BundlePriorityList* list = BundlePriorityList::Unmarshalling(parcel);
    if (list != nullptr) {
        delete list;
    }
    return true;
}

/**
 * @brief Main fuzzer routing function
 * Selects fuzz path based on input data and routes accordingly
 */
static bool DoFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < FUZZ_MIN_DATA_SIZE) {
        return false;
    }

    FuzzDataProvider provider(data, size);

    // Select fuzz path based on first byte
    uint8_t pathSelector = provider.ConsumeIntegral<uint8_t>();
    FuzzTestPath path = static_cast<FuzzTestPath>(
        pathSelector % static_cast<uint8_t>(FuzzTestPath::FUZZ_PATH_COUNT));

    switch (path) {
        case FuzzTestPath::FUZZ_IPC_STUB:
            return FuzzIPCStub(provider);
        case FuzzTestPath::FUZZ_PARCELABLE_WINDOW_INFO:
            return FuzzParcelableWindowInfo(provider);
        case FuzzTestPath::FUZZ_PARCELABLE_PROCESS_STATE_INFO:
            return FuzzParcelableProcessStateInfo(provider);
        case FuzzTestPath::FUZZ_PARCELABLE_BUNDLE_PRIORITY_LIST:
            return FuzzParcelableBundlePriorityList(provider);
        default:
            return FuzzIPCStub(provider);
    }
}

} // namespace Memory
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run fuzz tests */
    OHOS::Memory::DoFuzzTest(data, size);
    return 0;
}

