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

#ifndef OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEMMGRSERVICE_IPC_INTERFACE_CODE_H
#define OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEMMGRSERVICE_IPC_INTERFACE_CODE_H

/* SAID: 1909*/
namespace OHOS {
namespace Memory {
enum class MemMgrInterfaceCode {
        MEM_MGR_GET_BUNDLE_PRIORITY_LIST = 1,
        MEM_MGR_NOTIFY_DIST_DEV_STATUS = 2,
        MEM_MGR_GET_KILL_LEVEL_OF_LMKD = 3,
#ifdef USE_PURGEABLE_MEMORY
        MEM_MGR_REGISTER_ACTIVE_APPS = 4,
        MEM_MGR_DEREGISTER_ACTIVE_APPS = 5,
        MEM_MGR_SUBSCRIBE_APP_STATE = 6,
        MEM_MGR_UNSUBSCRIBE_APP_STATE = 7,
        MEM_MGR_GET_AVAILABLE_MEMORY = 8,       
        MEM_MGR_GET_TOTAL_MEMORY = 9,
#endif
};

enum class AppStateSubscriberInterfaceCode {
    ON_CONNECTED = FIRST_CALL_TRANSACTION,
    ON_DISCONNECTED,
    ON_APP_STATE_CHANGED,
    FORCE_RECLAIM,
    ON_TRIM,
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_INTERFACES_INNERKITS_INCLUDE_MEMMGRSERVICE_IPC_INTERFACE_CODE_H