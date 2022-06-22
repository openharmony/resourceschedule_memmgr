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

#include "memmgr_log.h"
#include "memmgr_ptr_util.h"
#include "parameters.h"
#include "kernel_interface.h"
#include "nandlife_controller.h"

namespace OHOS {
namespace Memory {
namespace {
const std::string TAG = "NandLifeController";

constexpr int TIMER_PEROID_MIN = 15;
constexpr int TIMER_PEROID_MS = TIMER_PEROID_MIN * 60 * 1000;

const std::string PARAM_VALUE_ZERO = "0";
const std::string PARAM_VALUE_ONE = "1";
const std::string PARAM_VALUE_UNKOWN = "-1";

const std::string PERMANENTLY_CLOSED_STATUS_PARAM = "persist.sys.eswap.permanently.closed";
const std::string PERMANENTLY_CLOSED = PARAM_VALUE_ONE;
const std::string NOT_PERMANENTLY_CLOSED = PARAM_VALUE_ZERO;

const std::string MINS_TODAY_PARAM = "persist.sys.eswap.minsToday";

const std::string SWAP_OUT_KB_TODAY_PARAM = "persist.sys.eswap.swapOutKBToday";

const std::string MINS_FROM_BIRTH_PARAM = "persist.sys.eswap.minsFromBirth";

const std::string SWAP_OUT_KB_FROM_BIRTH_PARAM = "persist.sys.eswap.swapOutKBFromBirth";

const std::string params[] = {
    PERMANENTLY_CLOSED_STATUS_PARAM,
    MINS_TODAY_PARAM,
    SWAP_OUT_KB_TODAY_PARAM,
    MINS_FROM_BIRTH_PARAM,
    SWAP_OUT_KB_FROM_BIRTH_PARAM,
};

const std::string PSI_HEALTH_INFO_PATH = "/dev/memcg/memory.eswap_info";
const std::string SWAP_OUT_SIZE_TAG = "Total Swapout Size";

const std::string ESWAP_ENABLE_PATH = "/proc/sys/kernel/hyperhold/enable";
const std::string ENABLE_ESWAP = "enable";
const std::string DISABLE_ESWAP = "disable";

constexpr int RETRY_TIMES = 3;
}

IMPLEMENT_SINGLE_INSTANCE(NandLifeController);

NandLifeController::NandLifeController()
{
    timerFunc_ = std::bind(&NandLifeController::CheckSwapOut, this);
}

bool NandLifeController::Init()
{
    if (!GetEventHandler()) {
        CloseSwapOutTemporarily("init handler failed, nandlife controller cannot set timmer");
        return false;
    }
    HILOGI("init handler successed");

    // read nandlife config from xml, then check and set it.
    // if the config does not meet the requirements, eswap will be closed temporarily.
    if (!GetAndValidateNandLifeConfig()) {
        CloseSwapOutTemporarily("get or validate nandlife config failed, controller will not work properly.");
        return false;
    }
    HILOGI("get and validate nandlife config success. dailyQuotaMB=%{public}llu, totalQuotaMB=%{public}llu",
        config_.daily_swap_out_quota_mb, config_.total_swap_out_quota_mb);
    if (config_.daily_swap_out_quota_mb == 0 && config_.total_swap_out_quota_mb == 0) {
        HILOGE("will not limit swap-out!");
        OpenSwapOutPermanently();
        OpenSwapOutTemporarily("not limit swap-out in xml");
        return true;
    } else {
        DAILY_SWAP_OUT_QUOTA_KB = config_.daily_swap_out_quota_mb * 1024; // 1024: MB to KB
        TOTAL_SWAP_OUT_QUOTA_KB = config_.total_swap_out_quota_mb * 1024; // 1024: MB to KB
    }

    if (!LoadNandLifeParam()) {
        CloseSwapOutTemporarily("load nandlife info file failed, controller will not work properly.");
        return false;
    }
    HILOGI("load nandlife sys param success");

    PrintNandLifeParam();

    if (IsSwapOutClosedPermently()) {
        CloseSwapOutTemporarily("swap-out has benn closed permently, nandlife controller no need work!");
        return false;
    }

    unsigned long long swapOutKBSinceKernelBoot = 0;
    if (GetSwapOutKBSinceKernelBoot(swapOutKBSinceKernelBoot)) {
        HILOGI("swapOutKBSinceKernelBoot=%{public}llu KB", swapOutKBSinceKernelBoot);
        lastSwapOutKB_ = swapOutKBSinceKernelBoot;
        nowSwapOutKB_ = swapOutKBSinceKernelBoot;
    } else {
        CloseSwapOutTemporarily("invalid swapOutKBSinceKernelBoot");
        return false;
    }

    // check total limit
    if (CheckReachedTotalLimit()) {
        SetTimer();
        return false;
    }

    // check daily limit
    if (CheckReachedDailyLimit()) {
        SetTimer();
        return false;
    }

    OpenSwapOutTemporarily("pass all check when init");
    SetTimer();
    return true;
}

// may throw exception due to number format
unsigned long long ReadUnsignedLongLongParam(const std::string &paramName)
{
    std::string value = system::GetParameter(paramName, PARAM_VALUE_UNKOWN);
    if (value == PARAM_VALUE_UNKOWN) {
        HILOGI("param <%{public}s> not set", paramName.c_str());
    }
    return std::strtoull(value.c_str(), nullptr, 10); // 10:Decimal
}

bool NandLifeController::LoadNandLifeParam()
{
    minsToday_ = ReadUnsignedLongLongParam(MINS_TODAY_PARAM);
    if (errno == ERANGE || minsToday_ == ULLONG_MAX) {
        HILOGI("[%{public}llu] invalid value of minsToday_", iter);
        return false;
    } else {
        HILOGI("[%{public}llu] minsToday_=%{public}llu", iter, minsToday_);
    }

    swapOutKBToday_ = ReadUnsignedLongLongParam(SWAP_OUT_KB_TODAY_PARAM);
    if (errno == ERANGE || swapOutKBToday_ == ULLONG_MAX) {
        HILOGI("[%{public}llu] invalid value of swapOutKBToday_", iter);
        return false;
    } else {
        HILOGI("[%{public}llu] swapOutKBToday_=%{public}llu", iter, swapOutKBToday_);
    }

    minsSinceBirth_ = ReadUnsignedLongLongParam(MINS_FROM_BIRTH_PARAM);
    if (errno == ERANGE || minsSinceBirth_ == ULLONG_MAX) {
        HILOGI("[%{public}llu] invalid value of minsSinceBirth_", iter);
        return false;
    } else {
        HILOGI("[%{public}llu] minsSinceBirth_=%{public}llu", iter, minsSinceBirth_);
    }

    swapOutKBSinceBirth_ = ReadUnsignedLongLongParam(SWAP_OUT_KB_FROM_BIRTH_PARAM);
    if (errno == ERANGE || swapOutKBSinceBirth_ == ULLONG_MAX) {
        HILOGI("[%{public}llu] invalid value of swapOutKBSinceBirth_", iter);
        return false;
    } else {
        HILOGI("[%{public}llu] swapOutKBSinceBirth_=%{public}llu", iter, swapOutKBSinceBirth_);
    }

    return true;
}

void NandLifeController::PrintNandLifeParam()
{
    HILOGI("[%{public}llu] begin print nandlife param-------------", iter);
    for (auto param : params) {
        HILOGI("[%{public}llu] %{public}s=%{public}s", iter, param.c_str(),
            system::GetParameter(param, PARAM_VALUE_UNKOWN).c_str());
    }
    HILOGI("[%{public}llu] end print nandlife param --------------", iter);
}

bool NandLifeController::IsSwapOutClosedPermently()
{
    return system::GetParameter(PERMANENTLY_CLOSED_STATUS_PARAM, PARAM_VALUE_UNKOWN) == PERMANENTLY_CLOSED;
}

bool NandLifeController::GetAndValidateNandLifeConfig()
{
    config_ = MemmgrConfigManager::GetInstance().GetNandLifeConfig();
    return config_.daily_swap_out_quota_mb >= 0 && config_.total_swap_out_quota_mb >=0;
}

bool NandLifeController::GetEventHandler()
{
    if (handler_ == nullptr) {
        MAKE_POINTER(handler_, shared, AppExecFwk::EventHandler, "failed to create event handler", return false,
            AppExecFwk::EventRunner::Create());
    }
    return true;
}

bool NandLifeController::GetSwapOutKBSinceKernelBoot(unsigned long long &ret)
{
    for (auto i = 0; i < RETRY_TIMES; i ++) {
        if (KernelInterface::GetInstance().ReadSwapOutKBSinceKernelBoot(PSI_HEALTH_INFO_PATH, SWAP_OUT_SIZE_TAG, ret)) {
            return true;
        }
    }
    return false;
}

void NandLifeController::SetTimer()
{
    // set timer and call CheckSwapOut each TIMER_PEROID_MIN min.
    handler_->PostTask(timerFunc_, TIMER_PEROID_MS, AppExecFwk::EventQueue::Priority::HIGH);
    HILOGI("[%{public}llu] set timer after %{public}d mins", iter, TIMER_PEROID_MIN);
}

bool NandLifeController::CheckReachedDailyLimit()
{
    bool reachedDailyLimit = swapOutKBToday_ >= DAILY_SWAP_OUT_QUOTA_KB;
    HILOGI("[%{public}llu] swapOutKBToday_(%{public}llu) %{public}s DAILY_SWAP_OUT_QUOTA_KB(%{public}llu)",
        iter, swapOutKBToday_, (reachedDailyLimit ? ">=" : "<"), DAILY_SWAP_OUT_QUOTA_KB);
    if (reachedDailyLimit) {
        CloseSwapOutTemporarily("reach daily limit, close swap-out temporarily!");
    } else {
        HILOGI("[%{public}llu] unreach daily limit, swap-out is still opened!", iter);
    }
    return reachedDailyLimit;
}

bool NandLifeController::CheckReachedTotalLimit()
{
    bool reachedTotalLimit = swapOutKBSinceBirth_ >= TOTAL_SWAP_OUT_QUOTA_KB;
    HILOGI("[%{public}llu] swapOutKBSinceBirth_(%{public}llu) %{public}s TOTAL_SWAP_OUT_QUOTA_KB(%{public}llu)",
        iter, swapOutKBSinceBirth_, (reachedTotalLimit ? ">=" : "<"), TOTAL_SWAP_OUT_QUOTA_KB);
    if (reachedTotalLimit) {
        HILOGE("[%{public}llu] reached total limit, close swap-out forever!", iter);
        CloseSwapOutPermanently();
    } else {
        HILOGI("[%{public}llu] unreach total limit!", iter);
    }
    return reachedTotalLimit;
}

void NandLifeController::CheckSwapOut()
{
    ++iter;

    HILOGE("[%{public}llu] called", iter);

    if (IsSwapOutClosedPermently()) {
        CloseSwapOutTemporarily("swap-out has benn closed permently!");
        SetTimer();
        return;
    }

    PrintNandLifeParam();

    minsToday_ += TIMER_PEROID_MIN;
    minsSinceBirth_ += TIMER_PEROID_MIN;

    if (GetSwapOutKBSinceKernelBoot(nowSwapOutKB_)) {
        HILOGI("[%{public}llu] swapOutKBSinceKernelBoot=%{public}llu KB", iter, nowSwapOutKB_);
    } else {
        CloseSwapOutTemporarily("invalid swapOutKBSinceKernelBoot");
        SetTimer();
        return;
    }
    if (nowSwapOutKB_ < lastSwapOutKB_) {
        CloseSwapOutTemporarily("deltaSwapOutMB < 0");
        SetTimer();
        return;
    }
    unsigned long long increasedSwapOutKB = nowSwapOutKB_ - lastSwapOutKB_;
    HILOGE("[%{public}llu] lastSwapOutKB_=%{public}llu, nowSwapOutKB_=%{public}llu, increasedSwapOutKB=%{public}llu",
        iter, lastSwapOutKB_, nowSwapOutKB_, increasedSwapOutKB);
    lastSwapOutKB_ = nowSwapOutKB_;
    swapOutKBToday_ += increasedSwapOutKB;
    swapOutKBSinceBirth_ += increasedSwapOutKB;

    CheckReachedDailyLimit();

    if (minsToday_ >= 24 * 60) { // 24: a day has 24 hours, 60: one hour has 60 min
        HILOGI("[%{public}llu] enter a new day", iter);
        minsToday_ = 0;
        swapOutKBToday_ = 0;
        if (swapOutKBSinceBirth_ < TOTAL_SWAP_OUT_QUOTA_KB) { // swap-out is allowed
            HILOGI("[%{public}llu] open swap-out since a new day", iter);
            OpenSwapOutTemporarily("enter a new day");
        }
    }

    if (!UpdateNandLifeParam()) {
        CloseSwapOutTemporarily("UpdateNandLifeParam failed!");
    }

    PrintNandLifeParam();

    CheckReachedTotalLimit();

    // set next timer
    SetTimer();
}

bool NandLifeController::SetParameterRetry(const std::string &paramName, const std::string &paramValue, int retryTimes)
{
    for (auto i = 0; i < retryTimes; i++) {
        if (system::SetParameter(paramName, paramValue)) {
            return true;
        }
    }
    HILOGW("[%{public}llu] set [%{public}s] to [%{public}s] failed!", iter, paramName.c_str(), paramValue.c_str());
    return false;
}

bool NandLifeController::UpdateNandLifeParam()
{
    if (!SetParameterRetry(MINS_TODAY_PARAM, std::to_string(minsToday_), RETRY_TIMES)) {
        return false;
    }
    if (!SetParameterRetry(SWAP_OUT_KB_TODAY_PARAM, std::to_string(swapOutKBToday_), RETRY_TIMES)) {
        return false;
    }
    if (!SetParameterRetry(MINS_FROM_BIRTH_PARAM, std::to_string(minsSinceBirth_), RETRY_TIMES)) {
        return false;
    }
    if (!SetParameterRetry(SWAP_OUT_KB_FROM_BIRTH_PARAM, std::to_string(swapOutKBSinceBirth_), RETRY_TIMES)) {
        return false;
    }
    HILOGW("[%{public}llu] all success!", iter);
    return true;
}

void NandLifeController::OpenSwapOutTemporarily(const std::string &reason)
{
    HILOGW("[%{public}llu] %{public}s", iter, reason.c_str());
    for (auto  i = 0; i < RETRY_TIMES; i++) {
        if (KernelInterface::GetInstance().EchoToPath(ESWAP_ENABLE_PATH.c_str(), ENABLE_ESWAP.c_str())) {
            HILOGI("[%{public}llu] open eswap temporarily success!", iter);
            return;
        }
    }
    HILOGW("[%{public}llu] open eswap temporarily failed!", iter);
}

void NandLifeController::CloseSwapOutTemporarily(const std::string &reason)
{
    HILOGW("[%{public}llu] %{public}s", iter, reason.c_str());
    for (auto  i = 0; i < RETRY_TIMES; i++) {
        if (KernelInterface::GetInstance().EchoToPath(ESWAP_ENABLE_PATH.c_str(), DISABLE_ESWAP.c_str())) {
            HILOGW("[%{public}llu] clsoe eswap temporarily success!", iter);
            return;
        }
    }
    HILOGW("[%{public}llu] close eswap temporarily failed!", iter);
}

void NandLifeController::OpenSwapOutPermanently()
{
    bool ret = SetParameterRetry(PERMANENTLY_CLOSED_STATUS_PARAM, NOT_PERMANENTLY_CLOSED, RETRY_TIMES);
    HILOGW("[%{public}llu] open eswap permanently %{public}s!", iter, ret ? "success" : "failed");
}

void NandLifeController::CloseSwapOutPermanently()
{
    CloseSwapOutTemporarily("CloseSwapOutPermanently close eswap temporarily first!");
    bool ret = SetParameterRetry(PERMANENTLY_CLOSED_STATUS_PARAM, PERMANENTLY_CLOSED, RETRY_TIMES);
    HILOGW("[%{public}llu] close eswap permanently %{public}s!", iter, ret ? "success" : "failed");
}
} // namespace Memory
} // namespace OHOS
