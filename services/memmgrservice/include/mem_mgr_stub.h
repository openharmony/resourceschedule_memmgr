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

#ifndef OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_STUB_H
#define OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_STUB_H

#include <map>

#include "iremote_stub.h"
#include "nocopyable.h"
#include "string_ex.h"
#include "i_mem_mgr.h"

namespace OHOS {
namespace Memory {
class MemMgrStub : public IRemoteStub<IMemMgr> {
public:
    MemMgrStub();
    virtual ~MemMgrStub();

    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleGetBunldePriorityList(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyDistDevStatus(MessageParcel &data, MessageParcel &reply);

    using MemMgrFunc = int32_t (MemMgrStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, MemMgrFunc> memberFuncMap_;

    DISALLOW_COPY_AND_MOVE(MemMgrStub);
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEM_MGR_STUB_H
