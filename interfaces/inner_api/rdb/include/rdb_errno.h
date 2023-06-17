/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef NATIVE_RDB_RDB_ERRNO_H
#define NATIVE_RDB_RDB_ERRNO_H

namespace OHOS {
namespace NativeRdb {

/**
* @brief The error code in the correct case.
*/
constexpr int E_OK = 0;

/**
* @brief The base code of the exception error code.
*/
constexpr int E_BASE = 14800000;

/**
* @brief The error when the capability not supported.
*/
constexpr int E_NOT_SUPPORTED = 801;

/**
* @brief The error code for common exceptions.
*/
constexpr int E_ERROR = E_BASE;

/**
* @brief The error code for common invalid args.
*/
constexpr int E_INVALID_ARGS = (E_BASE + 1);

/**
* @brief The error code for upgrade the read-only store.
*/
constexpr int E_CANNOT_UPDATE_READONLY = (E_BASE + 2);

/**
* @brief The error code when deleting a file fails.
*/
constexpr int E_REMOVE_FILE = (E_BASE + 3);

/**
* @brief The error code when deleting a file fails for rdb store in using.
*/
constexpr int E_RDB_STORE_IN_USING = (E_BASE + 4);

/**
* @brief The error code for a table name is empty.
*/
constexpr int E_EMPTY_TABLE_NAME = (E_BASE + 5);

/**
* @brief The error code for a values bucket is empty.
*/
constexpr int E_EMPTY_VALUES_BUCKET = (E_BASE + 6);

/**
* @brief The error code when the sql is not select.
*/
constexpr int E_EXECUTE_IN_STEP_QUERY = (E_BASE + 7);

/**
* @brief The error code for the column index is invalid.
*/
constexpr int E_INVALID_COLUMN_INDEX = (E_BASE + 8);

/**
* @brief The error code for the column type is invalid.
*/
constexpr int E_INVALID_COLUMN_TYPE = (E_BASE + 9);

/**
* @brief The error code for a file name is empty.
*/
constexpr int E_EMPTY_FILE_NAME = (E_BASE + 10);

/**
* @brief The error for the current file path is invalid.
*/
constexpr int E_INVALID_FILE_PATH = (E_BASE + 11);

/**
* @brief The error code when using transactions.
*/
constexpr int E_TRANSACTION_IN_EXECUTE = (E_BASE + 12);

/**
* @brief The error code for the current status is invalid.
*/
constexpr int E_INVALID_STATEMENT = (E_BASE + 13);

/**
* @brief The error code when execute write operation in read connection.
*/
constexpr int E_EXECUTE_WRITE_IN_READ_CONNECTION = (E_BASE + 14);

/**
* @brief The error code for execute begin transaction operation in read connection.
*/
constexpr int E_BEGIN_TRANSACTION_IN_READ_CONNECTION = (E_BASE + 15);

/**
* @brief The error code for there are no transactions in this connection.
*/
constexpr int E_NO_TRANSACTION_IN_SESSION = (E_BASE + 16);

/**
* @brief The error code when begin more step query in one session.
*/
constexpr int E_MORE_STEP_QUERY_IN_ONE_SESSION = (E_BASE + 17);

/**
* @brief The error code when the current statement doesn't contains one row result data.
*/
constexpr int E_NO_ROW_IN_QUERY = (E_BASE + 18);

/**
* @brief The error code for the bind arguments count is invalid.
*/
constexpr int E_INVALID_BIND_ARGS_COUNT = (E_BASE + 19);

/**
* @brief The error code for the object type is invalid.
*/
constexpr int E_INVALID_OBJECT_TYPE = (E_BASE + 20);

/**
* @brief The error code for the conflict flag is invalid.
*/
constexpr int E_INVALID_CONFLICT_FLAG = (E_BASE + 21);

/**
* @brief The error code for having clause not in group.
*/
constexpr int E_HAVING_CLAUSE_NOT_IN_GROUP_BY = (E_BASE + 22);

/**
* @brief The error code for not supported by step result set.
*/
constexpr int E_NOT_SUPPORTED_BY_STEP_RESULT_SET = (E_BASE + 23);

/**
* @brief The error code for step result current tid not equal to object's tid.
*/
constexpr int E_STEP_RESULT_SET_CROSS_THREADS = (E_BASE + 24);

/**
* @brief The error code when the result query was not executed.
*/
constexpr int E_STEP_RESULT_QUERY_NOT_EXECUTED = (E_BASE + 25);

/**
* @brief The error code for the result set cursor is after the last row.
*/
constexpr int E_STEP_RESULT_IS_AFTER_LAST = (E_BASE + 26);

/**
* @brief The error code for the result set query exceeded.
*/
constexpr int E_STEP_RESULT_QUERY_EXCEEDED = (E_BASE + 27);

/**
* @brief The error code for the statement not prepared.
*/
constexpr int E_STATEMENT_NOT_PREPARED = (E_BASE + 28);

/**
* @brief The error code for the result set is incorrect.
*/
constexpr int E_EXECUTE_RESULT_INCORRECT = (E_BASE + 29);

/**
* @brief The error code when the result set is closed.
*/
constexpr int E_STEP_RESULT_CLOSED = (E_BASE + 30);

/**
* @brief The error code when input relative path.
*/
constexpr int E_RELATIVE_PATH = (E_BASE + 31);

/**
* @brief The error code for the new encrypt key is empty.
*/
constexpr int E_EMPTY_NEW_ENCRYPT_KEY = (E_BASE + 32);

/**
* @brief The error code for change unencrypted to encrypted.
*/
constexpr int E_CHANGE_UNENCRYPTED_TO_ENCRYPTED = (E_BASE + 33);

/**
* @brief The error code for change encrypt in busy.
*/
constexpr int E_CHANGE_ENCRYPT_KEY_IN_BUSY = (E_BASE + 34);

/**
* @brief The error code when the statement not initialized.
*/
constexpr int E_STEP_STATEMENT_NOT_INIT = (E_BASE + 35);

/**
* @brief The error code for the attach is not supported in WAL journal mode.
*/
constexpr int E_NOT_SUPPORTED_ATTACH_IN_WAL_MODE = (E_BASE + 36);

/**
* @brief The error code when create folder failed.
*/
constexpr int E_CREATE_FOLDER_FAIL = (E_BASE + 37);

/**
* @brief The error for SQL builder normalize failed.
*/
constexpr int E_SQLITE_SQL_BUILDER_NORMALIZE_FAIL = (E_BASE + 38);

/**
* @brief The error for store session not give connection temporarily.
*/
constexpr int E_STORE_SESSION_NOT_GIVE_CONNECTION_TEMPORARILY = (E_BASE + 39);

/**
* @brief The error for store session not current transaction.
*/
constexpr int E_STORE_SESSION_NO_CURRENT_TRANSACTION = (E_BASE + 40);

/**
* @brief The error for not supported the current operation.
*/
constexpr int E_NOT_SUPPORT = (E_BASE + 41);

/**
* @brief The error for the current parcel is invalid.
*/
constexpr int E_INVALID_PARCEL = (E_BASE + 42);

/**
* @brief The error code when using sqlite3_step function failed.
*/
constexpr int E_QUERY_IN_EXECUTE = (E_BASE + 43);

/**
* @brief The error for set persist WAL.
*/
constexpr int E_SET_PERSIST_WAL = (E_BASE + 44);

/**
* @brief The error when the database does not exist.
*/
constexpr int E_DB_NOT_EXIST = (E_BASE + 45);

/**
* @brief The error when the read connection count is overload.
*/
constexpr int E_ARGS_READ_CON_OVERLOAD = (E_BASE + 46);

/**
* @brief The error when the wal file size over default limit.
*/
static constexpr int E_WAL_SIZE_OVER_LIMIT = (E_BASE + 47);

/**
* @brief The error when the connection count is used up.
*/
static constexpr int E_CON_OVER_LIMIT = (E_BASE + 48);

constexpr int E_INVALID_BUNDLE_NAME = (E_BASE + 49);
} // namespace NativeRdb
} // namespace OHOS

#endif
