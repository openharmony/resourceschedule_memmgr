/*
 * Copyright (c) 2021 XXXX.
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

#include "kernel_interface.h"
#include "memmgr_log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "KernelInterface";
}

IMPLEMENT_SINGLE_INSTANCE(KernelInterface);

bool KernelInterface::EchoToPath(const char* path, const char* content)
{
    uint32_t fd = open(path, O_RDWR, 0666);
    if (fd == -1) {
        HILOGE("echo %{public}s > %{public}s failed: file is not open", content, path);
        return false;
    }
    if (write(fd, content, strlen(content)) < 0) {
        HILOGE("echo %{public}s > %{public}s failed: write failed", content, path);
        close(fd);
        return false;
    }
    close(fd);
    HILOGI("echo %{public}s > %{public}s", content, path);
    return true;
}
} // namespace Memory
} // namespace OHOS
