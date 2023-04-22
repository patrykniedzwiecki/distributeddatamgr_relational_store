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

#ifndef RELATIONAL_STORE_H
#define RELATIONAL_STORE_H

#include "relational_cursor.h"
#include "relational_predicates.h"
#include "relational_values_bucket.h"

#ifdef __cplusplus
extern "C" {
#endif

enum StorageMode {
    RDB_MODE_MEMORY = 101,
    RDB_MODE_DISK,
};

enum JournalMode {
    MODE_DELETE,
    MODE_TRUNCATE,
    MODE_PERSIST,
    MODE_MEMORY,
    MODE_WAL,
    MODE_OFF,
};

enum StoreType {
    NORMAL,
    BACKUP,
    CORRUPT,
};

enum DistributedType {
    RDB_DEVICE_COLLABORATION = 10,
    RDB_DISTRIBUTED_TYPE_MAX
};

enum SecurityLevel {
    S1 = 1,
    S2,
    S3,
    S4,
    LAST
};

struct RDB_Config {
    const char *name;
    const char *path;
    StorageMode storageMode;
    JournalMode journalMode;

    bool readOnly;
    StoreType storeType;

    DistributedType distributedType;
    int area;
    const char *bundleName;
    const char *moduleName;

    bool isEncrypt;
    SecurityLevel securityLevel;
    bool isCreateNecessary;

    bool autoCheck;
    int journalSize;
    int pageSize;
    int readConSize;
    const char *encryptAlgo;
};

struct RDB_Store {
    int id;
};

RDB_Store *OH_Rdb_GetOrOpen(RDB_Config const *config, int version, int *errCode);
int OH_Rdb_CloseStore(RDB_Store *store);
int OH_Rdb_DeleteStore(const char *path);

int OH_Rdb_Insert(RDB_Store *store, char const *table, RDB_ValuesBucket *valuesBucket);
int OH_Rdb_Update(RDB_Store *store, RDB_ValuesBucket *valuesBucket, OH_Predicates *predicates);
int OH_Rdb_Delete(RDB_Store *store, OH_Predicates *predicate);
OH_Cursor *OH_Rdb_Query(RDB_Store *store, OH_Predicates *predicate, char const *const *columnNames, int length);
int OH_Rdb_Execute(RDB_Store *store, char const *sql);
OH_Cursor *OH_Rdb_ExecuteQuery(RDB_Store *store, char const *sql);
int OH_Rdb_Transaction(RDB_Store *store);
int OH_Rdb_RollBack(RDB_Store *store);
int OH_Rdb_Commit(RDB_Store *store);
int OH_Rdb_Backup(RDB_Store *store, const char *databasePath, const unsigned char *destEncryptKey);
int OH_Rdb_Restore(RDB_Store *store, const char *databasePath, const unsigned char *destEncryptKey);
int OH_Rdb_GetVersion(RDB_Store *store, int *version);
int OH_Rdb_SetVersion(RDB_Store *store, const int version);

#ifdef __cplusplus
};
#endif

#endif // RELATIONAL_STORE_H
