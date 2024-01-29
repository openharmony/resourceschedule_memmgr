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

#ifndef OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEM_MGR_LOG_H
#define OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEM_MGR_LOG_H

#include "hilog/log.h"

namespace OHOS {
namespace Memory {

#undef LOG_TAG
#define LOG_TAG "MemMgr"
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD001799

#ifdef HILOGF
#undef HILOGF
#endif
#ifdef HILOGE
#undef HILOGE
#endif
#ifdef HILOGW
#undef HILOGW
#endif
#ifdef HILOGI
#undef HILOGI
#endif
#ifdef HILOGD
#undef HILOGD
#endif

#define HILOGF(fmt, ...) HILOG_FATAL(LOG_CORE, fmt, ##__VA_ARGS__)
#define HILOGE(fmt, ...) HILOG_ERROR(LOG_CORE, fmt, ##__VA_ARGS__)
#define HILOGW(fmt, ...) HILOG_WARN(LOG_CORE,  fmt, ##__VA_ARGS__)
#define HILOGI(fmt, ...) HILOG_INFO(LOG_CORE,  fmt, ##__VA_ARGS__)
#define HILOGD(fmt, ...) HILOG_DEBUG(LOG_CORE, fmt, ##__VA_ARGS__)
} // namespace MemMgr
} // namespace OHOS
#endif // OHOS_MEMORY_MEMMGR_COMMON_INCLUDE_MEM_MGR_LOG_H
