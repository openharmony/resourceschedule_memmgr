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

#include "app_state_callback_mem_host.h"
#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "reclaim_priority_manager.h"

#include "app_mgr_interface.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "AppStateCallbackMemHost";
}

AppStateCallbackMemHost::AppStateCallbackMemHost()
{
    MEMMGR_MAKE_UNIQUE(appMgrClient_ = std::make_unique<AppExecFwk::AppMgrClient>());
    appStateObserver_ = new (std::nothrow) AppStateObserver();
    if (appStateObserver_ == NULL) {
        HILOGE("appStateObserver is NULL");
    }
}

AppStateCallbackMemHost::~AppStateCallbackMemHost()
{
    if (appStateObserver_ != nullptr) {
        delete appStateObserver_;
        appStateObserver_ = nullptr;
    }
}

bool AppStateCallbackMemHost::ConnectAppMgrService()
{
    if (appMgrClient_ == nullptr) {
        return false;
    }
    int result = static_cast<int>(appMgrClient_->ConnectAppMgrService());
    connected_ = result == ERR_OK;
    return connected_;
}

bool AppStateCallbackMemHost::Connected()
{
    return connected_;
}

sptr<AppExecFwk::IAppMgr> GetAppManangerInstance()
{
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    auto appObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    return iface_cast<AppExecFwk::IAppMgr>(appObject);
}

bool AppStateCallbackMemHost::Register()
{
    int result = GetAppManangerInstance()->RegisterApplicationStateObserver(appStateObserver_);
    return result == ERR_OK;
}
} // namespace Memory
} // namespace OHOS
