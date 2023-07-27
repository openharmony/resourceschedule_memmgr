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
#include <cstddef>
#include <cstdint>
#include "mem_mgr_service.h"
#include "memmgrstub_fuzzer.h"
#include "securec.h"

namespace OHOS {
namespace Memory {
#define MEMMGRSTUB_CODE_MIN 1
#define MEMMGRSTUB_CODE_MAX 10
#define MEMMGRSTUB_CODE_MIN_PURGEABLE 4
#define MEMMGRSTUB_CODE_MAX_PURGEABLE 9

const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;

template <class T> T GetData()
{
    T object{};
    size_t objectSize = sizeof(object);
    if (g_baseFuzzData == nullptr || g_baseFuzzSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += objectSize;
    return object;
}

bool HandleGetBunldePriorityListFuzzTest(const uint8_t *data, size_t size)
{
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;
    if (size > sizeof(int) + sizeof(int)) {
        MessageParcel data1 = MessageParcel();
        Parcel parcel;
        sptr<IRemoteObject> iremoteobject = IRemoteObject::Unmarshalling(parcel);
        int intdata = GetData<int>();
        void *voiddata = &intdata;
        size_t size1 = sizeof(int);
        data1.WriteRemoteObject(iremoteobject);
        data1.WriteRawData(voiddata, size1);
        data1.ReadRawData(size1);
        MessageParcel reply = MessageParcel();
        MessageOption option;
        uint32_t code = GetData<int>() % (MEMMGRSTUB_CODE_MAX - MEMMGRSTUB_CODE_MIN + 1) + MEMMGRSTUB_CODE_MIN;
#ifndef USE_PURGEABLE_MEMORY
        if (code <= MEMMGRSTUB_CODE_MAX_PURGEABLE || code >= MEMMGRSTUB_CODE_MIN_PURGEABLE) {
            return false;
        }
#endif
        MemMgrService::GetInstance().OnRemoteRequest(code, data1, reply, option);
    }
    return true;
}
} // namespace Memory
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::Memory::HandleGetBunldePriorityListFuzzTest(data, size);
    return 0;
}
