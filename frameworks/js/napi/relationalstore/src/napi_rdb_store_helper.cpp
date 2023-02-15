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
#include <functional>
#include <string>
#include <vector>

#include "js_ability.h"
#include "js_logger.h"
#include "js_utils.h"
#include "napi_async_call.h"
#include "napi_rdb_error.h"
#include "napi_rdb_store.h"
#include "napi_rdb_store_helper.h"
#include "napi_rdb_trace.h"
#include "rdb_errno.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"
#include "sqlite_database_utils.h"
#include "unistd.h"

using namespace OHOS::NativeRdb;
using namespace OHOS::AppDataMgrJsKit;

namespace OHOS {
namespace RelationalStoreJsKit {
struct HelperRdbContext : public Context {
    RdbStoreConfig config;
    std::shared_ptr<RdbStore> proxy;
    std::shared_ptr<OHOS::AppDataMgrJsKit::Context> abilitycontext;
    bool isSystemAppCalled;

    HelperRdbContext() : config(""), proxy(nullptr), isSystemAppCalled(false)
    {
    }
    virtual ~HelperRdbContext(){};
};

int ParseContext(const napi_env &env, const napi_value &object, std::shared_ptr<HelperRdbContext> context)
{
    auto abilityContext = JSAbility::GetContext(env, object);
    CHECK_RETURN_SET(abilityContext != nullptr, std::make_shared<ParamError>("context", "a Context."));
    context->abilitycontext = abilityContext;
    return OK;
}

int ParseDatabaseName(const napi_env &env, const napi_value &object, std::shared_ptr<HelperRdbContext> context)
{
    napi_value value;
    napi_get_named_property(env, object, "name", &value);
    CHECK_RETURN_SET(value != nullptr, std::make_shared<ParamError>("config", "a StoreConfig."));

    std::string name = JSUtils::Convert2String(env, value);
    CHECK_RETURN_SET(!name.empty(), std::make_shared<ParamError>("config", "a StoreConfig."));
    if (name.find("/") != std::string::npos) {
        CHECK_RETURN_SET(false, std::make_shared<ParamError>("StoreConfig.name", "a file name without path"));
    }

    context->config.SetName(std::move(name));
    return OK;
}

int ParseIsEncrypt(const napi_env &env, const napi_value &object, std::shared_ptr<HelperRdbContext> context)
{
    napi_value value = nullptr;
    napi_status status = napi_get_named_property(env, object, "encrypt", &value);
    if (status == napi_ok && value != nullptr) {
        bool isEncrypt = false;
        JSUtils::Convert2Bool(env, value, isEncrypt);
        context->config.SetEncryptStatus(isEncrypt);
    }
    return OK;
}

int ParseContextProperty(const napi_env &env, std::shared_ptr<HelperRdbContext> context)
{
    if (context->abilitycontext == nullptr) {
        int status = ParseContext(env, nullptr, context); // when no context as arg got from application.
        CHECK_RETURN_SET(status == OK, std::make_shared<ParamError>("context", "a Context."));
    }
    context->config.SetModuleName(context->abilitycontext->GetModuleName());
    context->config.SetArea(context->abilitycontext->GetArea());
    context->config.SetBundleName(context->abilitycontext->GetBundleName());
    context->config.SetUri(context->abilitycontext->GetUri());
    context->config.SetReadPermission(context->abilitycontext->GetReadPermission());
    context->config.SetWritePermission(context->abilitycontext->GetWritePermission());
    context->isSystemAppCalled = context->abilitycontext->IsSystemAppCalled();
    return OK;
}

int ParseDatabaseDir(const napi_env &env, std::shared_ptr<HelperRdbContext> context)
{
    if (context->abilitycontext == nullptr) {
        int status = ParseContext(env, nullptr, context); // when no context as arg got from application.
        CHECK_RETURN_SET(status == OK, std::make_shared<ParamError>("context", "a Context."));
    }
    CHECK_RETURN_SET(context->abilitycontext != nullptr, std::make_shared<ParamError>("context", "a Context."));
    int errorCode = E_OK;
    std::string databaseName = context->config.GetName();
    std::string databaseDir = context->abilitycontext->GetDatabaseDir();
    std::string realPath = SqliteDatabaseUtils::GetDefaultDatabasePath(databaseDir, databaseName, errorCode);
    CHECK_RETURN_SET(errorCode == E_OK, std::make_shared<ParamError>("config", "a StoreConfig."));
    context->config.SetPath(std::move(realPath));
    return OK;
}

int ParseSecurityLevel(const napi_env &env, const napi_value &object, std::shared_ptr<HelperRdbContext> context)
{
    napi_value value = nullptr;
    bool hasProp = false;
    napi_status status = napi_has_named_property(env, object, "securityLevel", &hasProp);
    CHECK_RETURN_SET(status == napi_ok && hasProp, std::make_shared<ParamError>("config", "with securityLevel."));

    status = napi_get_named_property(env, object, "securityLevel", &value);
    CHECK_RETURN_SET(status == napi_ok, std::make_shared<ParamError>("config", "with securityLevel."));

    int32_t securityLevel;
    napi_get_value_int32(env, value, &securityLevel);
    SecurityLevel sl = static_cast<SecurityLevel>(securityLevel);
    LOG_DEBUG("Get sl:%{public}d", securityLevel);

    bool isValidSecurityLevel = sl >= SecurityLevel::S1 && sl < SecurityLevel::LAST;
    CHECK_RETURN_SET(isValidSecurityLevel, std::make_shared<ParamError>("config", "with correct securityLevel."));

    context->config.SetSecurityLevel(sl);

    LOG_DEBUG("ParseSecurityLevel end");
    return OK;
}

int ParseStoreConfig(const napi_env &env, const napi_value &object, std::shared_ptr<HelperRdbContext> context)
{
    CHECK_RETURN_CORE(OK == ParseDatabaseName(env, object, context), RDB_REVT_NOTHING, ERR);
    CHECK_RETURN_CORE(OK == ParseIsEncrypt(env, object, context), RDB_REVT_NOTHING, ERR);
    CHECK_RETURN_CORE(OK == ParseSecurityLevel(env, object, context), RDB_REVT_NOTHING, ERR);
    CHECK_RETURN_CORE(OK == ParseContextProperty(env, context), RDB_REVT_NOTHING, ERR);
    CHECK_RETURN_CORE(OK == ParseDatabaseDir(env, context), RDB_REVT_NOTHING, ERR);
    return OK;
}

int ParsePath(const napi_env &env, const napi_value &arg, std::shared_ptr<HelperRdbContext> context)
{
    std::string path = JSUtils::Convert2String(env, arg);
    CHECK_RETURN_SET(!path.empty(), std::make_shared<ParamError>("name", "a without path non empty string."));

    size_t pos = path.find_first_of('/');
    CHECK_RETURN_SET(pos == std::string::npos, std::make_shared<ParamError>("name", "a without path without /."));

    std::string databaseDir = context->abilitycontext->GetDatabaseDir();
    int errorCode = E_OK;
    std::string realPath = SqliteDatabaseUtils::GetDefaultDatabasePath(databaseDir, path, errorCode);
    CHECK_RETURN_SET(errorCode == E_OK, std::make_shared<ParamError>("path", "access"));

    context->config.SetPath(realPath);
    return OK;
}

class DefaultOpenCallback : public RdbOpenCallback {
public:
    int OnCreate(RdbStore &rdbStore) override
    {
        return E_OK;
    }
    int OnUpgrade(RdbStore &rdbStore, int oldVersion, int newVersion) override
    {
        return E_OK;
    }
};

napi_value GetRdbStore(napi_env env, napi_callback_info info)
{
    DISTRIBUTED_DATA_HITRACE(std::string(__FUNCTION__));
    LOG_DEBUG("RelationalStoreJsKit::GetRdbStore start");
    auto context = std::make_shared<HelperRdbContext>();
    auto input = [context, info](napi_env env, size_t argc, napi_value *argv, napi_value self) {
        bool checked = JSAbility::CheckContext(env, info);
        CHECK_RETURN_SET_E(checked, std::make_shared<ParamError>("context", "a valid Context."));
        CHECK_RETURN_SET_E(argc == 2 || argc == 3, std::make_shared<ParamNumError>("2 or 3"));
        CHECK_RETURN(OK == ParseContext(env, argv[0], context));
        CHECK_RETURN(OK == ParseStoreConfig(env, argv[1], context));
    };
    auto exec = [context]() -> int {
        LOG_DEBUG("RelationalStoreJsKit::GetRdbStore Async");
        int errCode = OK;
        DefaultOpenCallback callback;
        context->proxy = RdbHelper::GetRdbStore(context->config, -1, callback, errCode);
        return errCode;
    };
    auto output = [context](napi_env env, napi_value &result) {
        result = RdbStoreProxy::NewInstance(env, context->proxy, context->isSystemAppCalled);
        CHECK_RETURN_SET_E(result != nullptr, std::make_shared<InnerError>(E_ERROR));
        LOG_DEBUG("RelationalStoreJsKit::GetRdbStore end");
    };
    context->SetAction(env, info, input, exec, output);

    CHECK_RETURN_NULL(context->error == nullptr || context->error->GetCode() == OK);
    return AsyncCall::Call(env, context);
}

napi_value DeleteRdbStore(napi_env env, napi_callback_info info)
{
    DISTRIBUTED_DATA_HITRACE(std::string(__FUNCTION__));
    LOG_DEBUG("RelationalStoreJsKit::DeleteRdbStore start");
    auto context = std::make_shared<HelperRdbContext>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) {
        CHECK_RETURN_SET_E(argc == 2 || argc == 3, std::make_shared<ParamNumError>("2 or 3"));
        CHECK_RETURN(OK == ParseContext(env, argv[0], context));
        CHECK_RETURN(OK == ParsePath(env, argv[1], context));
    };
    auto exec = [context]() -> int {
        return RdbHelper::DeleteRdbStore(context->config.GetPath());
    };
    auto output = [context](napi_env env, napi_value &result) {
        napi_status status = napi_create_int64(env, OK, &result);
        CHECK_RETURN_SET_E(status == napi_ok, std::make_shared<InnerError>(E_ERROR));
        LOG_DEBUG("RelationalStoreJsKit::DeleteRdbStore end");
    };
    context->SetAction(env, info, input, exec, output);

    CHECK_RETURN_NULL(context->error == nullptr || context->error->GetCode() == OK);
    return AsyncCall::Call(env, context);
}

napi_value InitRdbHelper(napi_env env, napi_value exports)
{
    LOG_INFO("RelationalStoreJsKit::InitRdbHelper begin");
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("getRdbStore", GetRdbStore),
        DECLARE_NAPI_FUNCTION("deleteRdbStore", DeleteRdbStore),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(properties) / sizeof(*properties), properties));
    LOG_INFO("RelationalStoreJsKit::InitRdbHelper end");
    return exports;
}
} // namespace RelationalStoreJsKit
} // namespace OHOS
