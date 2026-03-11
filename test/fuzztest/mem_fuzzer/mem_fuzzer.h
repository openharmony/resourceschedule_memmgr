/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_MEMMGR_FUZZTEST_MEM_FUZZER_H
#define OHOS_MEMMGR_FUZZTEST_MEM_FUZZER_H

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>

// IPC related headers
#include "message_parcel.h"
#include "message_option.h"
#include "iremote_stub.h"

// MemMgr service headers
#include "i_mem_mgr.h"
#include "mem_mgr_stub.h"
#include "mem_mgr_service.h"
#include "memmgrservice_ipc_interface_code.h"

// Parcelable types
#include "bundle_priority_list.h"
#include "bundle_priority.h"
#include "mem_mgr_window_info.h"
#include "mem_mgr_process_state_info.h"

// Security
#include "securec.h"

#define FUZZ_PROJECT_NAME "mem_fuzzer"

namespace OHOS {
namespace Memory {

// Fuzzer configuration constants - derived from project definitions
constexpr size_t FUZZ_MIN_DATA_SIZE = sizeof(uint32_t);
constexpr size_t FUZZ_THRESHOLD_FOR_IPC = sizeof(uint32_t) * 2;
constexpr size_t FUZZ_THRESHOLD_FOR_PARCELABLE = sizeof(int32_t) * 4;

// Window info fuzzer limits - derived from mem_mgr_stub.cpp MAX_PARCEL_SIZE
constexpr uint32_t FUZZ_MAX_WINDOW_INFO_COUNT = 100;

// Bundle priority list limits - derived from bundle_priority_list.cpp MAX_PARCEL_SIZE
constexpr int32_t FUZZ_MAX_BUNDLE_COUNT = 1000;

// IPC code range - derived from MemMgrInterfaceCode enum
constexpr uint32_t FUZZ_IPC_CODE_MIN = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_GET_BUNDLE_PRIORITY_LIST);
constexpr uint32_t FUZZ_IPC_CODE_MAX = static_cast<uint32_t>(MemMgrInterfaceCode::MEM_MGR_SET_CRITICAL);

// Fuzz test path selectors
enum class FuzzTestPath : uint8_t {
    FUZZ_IPC_STUB = 0,
    FUZZ_PARCELABLE_WINDOW_INFO,
    FUZZ_PARCELABLE_PROCESS_STATE_INFO,
    FUZZ_PARCELABLE_BUNDLE_PRIORITY_LIST,
    FUZZ_PATH_COUNT
};

/**
 * @brief Data provider for structured fuzzing
 * Extracts typed data from raw fuzz input bytes
 */
class FuzzDataProvider {
public:
    FuzzDataProvider(const uint8_t* data, size_t size)
        : data_(data), size_(size), pos_(0) {}

    template<typename T>
    T ConsumeIntegral()
    {
        T value{};
        size_t typeSize = sizeof(T);
        if (data_ == nullptr || pos_ + typeSize > size_) {
            return value;
        }
        errno_t ret = memcpy_s(&value, typeSize, data_ + pos_, typeSize);
        if (ret != EOK) {
            return T{};
        }
        pos_ += typeSize;
        return value;
    }

    template<typename T>
    T ConsumeIntegralInRange(T min, T max)
    {
        if (min >= max) {
            return min;
        }
        T value = ConsumeIntegral<T>();
        return min + (value % (max - min + 1));
    }

    bool ConsumeBool()
    {
        return ConsumeIntegral<uint8_t>() & 1;
    }

    std::string ConsumeString(size_t maxLength)
    {
        size_t length = ConsumeIntegralInRange<size_t>(0, maxLength);
        if (length > RemainingBytes()) {
            length = RemainingBytes();
        }
        std::string result;
        if (length > 0 && data_ != nullptr) {
            result.assign(reinterpret_cast<const char*>(data_ + pos_), length);
            pos_ += length;
        }
        return result;
    }

    std::vector<uint8_t> ConsumeBytes(size_t count)
    {
        if (count > RemainingBytes()) {
            count = RemainingBytes();
        }
        std::vector<uint8_t> result;
        if (count > 0 && data_ != nullptr) {
            result.assign(data_ + pos_, data_ + pos_ + count);
            pos_ += count;
        }
        return result;
    }

    size_t RemainingBytes() const
    {
        return (pos_ < size_) ? (size_ - pos_) : 0;
    }

    bool HasEnoughData(size_t required) const
    {
        return RemainingBytes() >= required;
    }

private:
    const uint8_t* data_;
    size_t size_;
    size_t pos_;
};

// Fuzzer function declarations
bool FuzzIPCStub(FuzzDataProvider& provider);
bool FuzzParcelableWindowInfo(FuzzDataProvider& provider);
bool FuzzParcelableProcessStateInfo(FuzzDataProvider& provider);
bool FuzzParcelableBundlePriorityList(FuzzDataProvider& provider);

} // namespace Memory
} // namespace OHOS

#endif // OHOS_MEMMGR_FUZZTEST_MEM_FUZZER_H