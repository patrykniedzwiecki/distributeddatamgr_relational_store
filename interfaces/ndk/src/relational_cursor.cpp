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

#include <iostream>
#include <sstream>
#include <string>

#include "logger.h"
#include "oh_cursor.h"
#include "relational_cursor_impl.h"
#include "relational_error_code.h"
#include "rdb_errno.h"
#include "securec.h"

using namespace OHOS::RdbNdk;

int Rdb_GetColumnCount(OH_Cursor *cursor, int *count)
{
    if (cursor == nullptr || count == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, count is NULL ? %{public}d", (cursor == nullptr),
            (count == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    OHOS::RdbNdk::CursorImpl *tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    return tempCursor->GetResultSet()->GetColumnCount(*count);
}

int Rdb_GetColumnType(OH_Cursor *cursor, int32_t columnIndex, OH_ColumnType *columnType)
{
    if (cursor == nullptr || columnType == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, columnType is NULL ? %{public}d",
            (cursor == nullptr), (columnType == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    OHOS::NativeRdb::ColumnType type;
    int error = tempCursor->GetResultSet()->GetColumnType(columnIndex, type);
    *columnType = static_cast<OH_ColumnType>(static_cast<int>(type));
    return error;
}

int Rdb_GetColumnIndex(OH_Cursor *cursor, const char *name, int *columnIndex)
{
    if (cursor == nullptr || name == nullptr || columnIndex == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, name is NULL ? %{public}d,"
                  "columnIndex is NULL ? %{public}d", (cursor == nullptr), (name == nullptr), columnIndex == nullptr);
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    return tempCursor->GetResultSet()->GetColumnIndex(name, *columnIndex);
}

int Rdb_GetColumnName(OH_Cursor *cursor, int32_t columnIndex, char *name, int length)
{
    if (cursor == nullptr || name == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, name is NULL ? %{public}d", (cursor == nullptr),
            (name == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    std::string str;
    int errCode = tempCursor->GetResultSet()->GetColumnName(columnIndex, str);
    if (errCode != OHOS::NativeRdb::E_OK) {
        return errCode;
    }
    errno_t result = memcpy_s(name, length, str.c_str(), str.length());
    if (result != EOK) {
        LOG_ERROR("memcpy_s failed, result is %{public}d", result);
        return OH_Rdb_ErrCode::RDB_ERR;
    }
    return errCode;
}

int Rdb_GetRowCount(OH_Cursor *cursor, int *count)
{
    if (cursor == nullptr || count == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, count is NULL ? %{public}d", (cursor == nullptr),
            (count == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    return tempCursor->GetResultSet()->GetRowCount(*count);
}

int Rdb_GoToNextRow(OH_Cursor *cursor)
{
    if (cursor == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d", (cursor == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    return tempCursor->GetResultSet()->GoToNextRow();
}

int Rdb_GetSize(OH_Cursor *cursor, int32_t columnIndex, size_t *size)
{
    if (cursor == nullptr || size == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, size is NULL ? %{public}d", (cursor == nullptr),
            (size == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    return tempCursor->GetResultSet()->GetSize(columnIndex, *size);
}

int Rdb_GetText(OH_Cursor *cursor, int32_t columnIndex, char *value, int length)
{
    if (cursor == nullptr || value == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, value is NULL ? %{public}d", (cursor == nullptr),
            (value == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    std::string str;
    int errCode = tempCursor->GetResultSet()->GetString(columnIndex, str);
    if (errCode != OHOS::NativeRdb::E_OK) {
        return errCode;
    }
    errno_t result = memcpy_s(value, length, str.c_str(), str.length());
    if (result != EOK) {
        LOG_ERROR("memcpy_s failed, result is %{public}d", result);
        return OH_Rdb_ErrCode::RDB_ERR;
    }
    return errCode;
}

int Rdb_GetInt64(OH_Cursor *cursor, int32_t columnIndex, int64_t *value)
{
    if (cursor == nullptr || value == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, value is NULL ? %{public}d", (cursor == nullptr),
            (value == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    return tempCursor->GetResultSet()->GetLong(columnIndex, *value);
}

int Rdb_GetReal(OH_Cursor *cursor, int32_t columnIndex, double *value)
{
    if (cursor == nullptr || value == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, value is NULL ? %{public}d", (cursor == nullptr),
            (value == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    return tempCursor->GetResultSet()->GetDouble(columnIndex, *value);
}

int Rdb_GetBlob(OH_Cursor *cursor, int32_t columnIndex, unsigned char *value, int length)
{
    if (cursor == nullptr || value == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, value is NULL ? %{public}d", (cursor == nullptr),
            (value == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    std::vector<uint8_t> vec;
    int errCode = tempCursor->GetResultSet()->GetBlob(columnIndex, vec);
    if (errCode != OHOS::NativeRdb::E_OK) {
        return errCode;
    }
    errno_t result = memcpy_s(value, length, vec.data(), vec.size());
    if (result != EOK) {
        LOG_ERROR("memcpy_s failed, result is %{public}d", result);
        return OH_Rdb_ErrCode::RDB_ERR;
    }
    return errCode;
}

int Rdb_IsNull(OH_Cursor *cursor, int32_t columnIndex, bool *isNull)
{
    if (cursor == nullptr || isNull == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d, value is NULL ? %{public}d", (cursor == nullptr),
            (isNull == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    bool isNULLTemp = false;
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    int ret = tempCursor->GetResultSet()->IsColumnNull(columnIndex, isNULLTemp);
    isNULLTemp == true ? *isNull = true : *isNull = false;
    return ret;
}

int Rdb_Close(OH_Cursor *cursor)
{
    if (cursor == nullptr || cursor->id != OHOS::RdbNdk::RDB_CURSOR_CID) {
        LOG_ERROR("Parameters set error:cursor is NULL ? %{public}d", (cursor == nullptr));
        return OH_Rdb_ErrCode::RDB_E_INVALID_ARGS;
    }
    auto tempCursor = static_cast<OHOS::RdbNdk::CursorImpl *>(cursor);
    int errCode = tempCursor->GetResultSet()->Close();
    if (errCode != OHOS::NativeRdb::E_OK) {
        return errCode;
    }
    delete tempCursor;
    tempCursor = nullptr;
    return errCode;
}

OHOS::RdbNdk::CursorImpl::CursorImpl(std::shared_ptr<OHOS::NativeRdb::ResultSet> resultSet) : resultSet_(resultSet)
{
    id = RDB_CURSOR_CID;

    getColumnCount = Rdb_GetColumnCount;
    getColumnType = Rdb_GetColumnType;
    getColumnIndex = Rdb_GetColumnIndex;
    getColumnName = Rdb_GetColumnName;
    getRowCount = Rdb_GetRowCount;
    goToNextRow = Rdb_GoToNextRow;
    getSize = Rdb_GetSize;
    getText = Rdb_GetText;
    getInt64 = Rdb_GetInt64;
    getReal = Rdb_GetReal;
    getBlob = Rdb_GetBlob;
    isNull = Rdb_IsNull;
    close = Rdb_Close;
}

std::shared_ptr<OHOS::NativeRdb::ResultSet> OHOS::RdbNdk::CursorImpl::GetResultSet()
{
    return resultSet_;
}
