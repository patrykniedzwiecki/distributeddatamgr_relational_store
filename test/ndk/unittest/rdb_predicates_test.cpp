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

#include <gtest/gtest.h>

#include <string>
#include "common.h"
#include "relational_store.h"

using namespace testing::ext;
using namespace OHOS::NativeRdb;

class RdbNdkPredicatesTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

std::string predicatesTestPath_ = RDB_TEST_PATH + "rdb_predicates_test.db";
RDB_Store *predicatesTestRdbStore_;

void RdbNdkPredicatesTest::SetUpTestCase(void)
{
    RDB_Config config;
    config.path = predicatesTestPath_.c_str();
    config.securityLevel = SecurityLevel::S1;
    config.isEncrypt = Bool::FALSE;

    int version = 1;
    int errCode = 0;
    char table[] = "test";
    predicatesTestRdbStore_ = OH_Rdb_GetOrOpen(&config, version, NULL, &errCode);
    EXPECT_NE(predicatesTestRdbStore_, NULL);

    char createTableSql[] = "CREATE TABLE test (id INTEGER PRIMARY KEY AUTOINCREMENT, data1 TEXT, data2 INTEGER, "
                            "data3 FLOAT, data4 BLOB, data5 TEXT);";
    errCode = OH_Rdb_Execute(predicatesTestRdbStore_, createTableSql);

    RDB_ValuesBucket* valueBucket = OH_Rdb_CreateValuesBucket();
    OH_VBucket_PutInt64(valueBucket, "id", 1);
    OH_VBucket_PutText(valueBucket, "data1", "zhangSan");
    OH_VBucket_PutInt64(valueBucket, "data2", 12800);
    OH_VBucket_PutReal(valueBucket, "data3", 100.1);
    uint8_t arr[] = {1, 2, 3, 4, 5};
    int len = sizeof(arr) / sizeof(arr[0]);
    OH_VBucket_PutBlob(valueBucket, "data4", arr, len);
    OH_VBucket_PutText(valueBucket, "data5", "ABCDEFG");
    errCode = OH_Rdb_Insert(predicatesTestRdbStore_, table, valueBucket);
    EXPECT_EQ(errCode, 1);

    OH_VBucket_Clear(valueBucket);
    OH_VBucket_PutInt64(valueBucket, "id", 2);
    OH_VBucket_PutText(valueBucket, "data1", "liSi");
    OH_VBucket_PutInt64(valueBucket, "data2", 13800);
    OH_VBucket_PutReal(valueBucket, "data3", 200.1);
    OH_VBucket_PutText(valueBucket, "data5", "ABCDEFGH");
    errCode = OH_Rdb_Insert(predicatesTestRdbStore_, table, valueBucket);
    EXPECT_EQ(errCode, 2);

    OH_VBucket_Clear(valueBucket);
    OH_VBucket_PutInt64(valueBucket, "id", 3);
    OH_VBucket_PutText(valueBucket, "data1", "wangWu");
    OH_VBucket_PutInt64(valueBucket, "data2", 14800);
    OH_VBucket_PutReal(valueBucket, "data3", 300.1);
    OH_VBucket_PutText(valueBucket, "data5", "ABCDEFGHI");
    errCode = OH_Rdb_Insert(predicatesTestRdbStore_, table, valueBucket);
    EXPECT_EQ(errCode, 3);
}

void RdbNdkPredicatesTest::TearDownTestCase(void)
{
    OH_Rdb_DeleteStore(predicatesTestPath_.c_str());
}

void RdbNdkPredicatesTest::SetUp(void)
{
}

void RdbNdkPredicatesTest::TearDown(void)
{
}

/**
 * @tc.name: RDB_NDK_predicates_test_001
 * @tc.desc: Normal testCase of NDK Predicates for EqualTo、AndOR、beginWrap and endWrap
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_001, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    errCode = predicates->OH_Predicates_BeginWrap(predicates);
    errCode = predicates->OH_Predicates_EqualTo(predicates, "data1", "zhangSan");
    errCode = predicates->OH_Predicates_Or(predicates);
    errCode = predicates->OH_Predicates_EqualTo(predicates, "data3", "200.1");
    errCode = predicates->OH_Predicates_EndWrap(predicates);

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 2);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_002
 * @tc.desc: Normal testCase of NDK Predicates for NotEqualTo
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_002, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    errCode = predicates->OH_Predicates_NotEqualTo(predicates, "data1", "zhangSan");
    EXPECT_EQ(errCode, 0);

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 2);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_003
 * @tc.desc: Normal testCase of NDK Predicates for GreaterThan
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_003, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    errCode = predicates->OH_Predicates_GreaterThan(predicates, "data5", "ABCDEFG");
    EXPECT_EQ(errCode, 0);

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 2);

    cursor->OH_Cursor_GoToNextRow(cursor);

    int columnCount = 0;
    cursor->OH_Cursor_GetColumnCount(cursor, &columnCount);
    EXPECT_EQ(columnCount, 6);

    int64_t id;
    cursor->OH_Cursor_GetInt64(cursor, 0, &id);
    EXPECT_EQ(id, 2);

    size_t size = 0;
    cursor->OH_Cursor_GetSize(cursor, 1, &size);
    char data1Value[size + 1];
    cursor->OH_Cursor_GetText(cursor, 1, data1Value, size + 1);
    EXPECT_EQ(strcmp(data1Value, "liSi"), 0);

    int64_t data2Value;
    cursor->OH_Cursor_GetInt64(cursor, 2, &data2Value);
    EXPECT_EQ(data2Value, 13800);

    double data3Value;
    cursor->OH_Cursor_GetReal(cursor, 3, &data3Value);
    EXPECT_EQ(data3Value, 200.1);

    bool isNull = false;
    cursor->OH_Cursor_IsNull(cursor, 4, &isNull);
    EXPECT_EQ(isNull, true);

    cursor->OH_Cursor_GetSize(cursor, 5, &size);
    char data5Value[size + 1];
    cursor->OH_Cursor_GetText(cursor, 5, data5Value, size + 1);
    EXPECT_EQ(strcmp(data5Value, "ABCDEFGH"), 0);

    cursor->OH_Cursor_GoToNextRow(cursor);

    cursor->OH_Cursor_GetInt64(cursor, 0, &id);
    EXPECT_EQ(id, 3);

    cursor->OH_Cursor_GetSize(cursor, 1, &size);
    char data1Value_1[size + 1];
    cursor->OH_Cursor_GetText(cursor, 1, data1Value_1, size + 1);
    EXPECT_EQ(strcmp(data1Value_1, "wangWu"), 0);

    cursor->OH_Cursor_GetInt64(cursor, 2, &data2Value);
    EXPECT_EQ(data2Value, 14800);

    cursor->OH_Cursor_GetReal(cursor, 3, &data3Value);
    EXPECT_EQ(data3Value, 300.1);

    cursor->OH_Cursor_IsNull(cursor, 4, &isNull);
    EXPECT_EQ(isNull, true);

    cursor->OH_Cursor_GetSize(cursor, 5, &size);
    char data5Value_1[size + 1];
    cursor->OH_Cursor_GetText(cursor, 5, data5Value_1, size + 1);
    EXPECT_EQ(strcmp(data5Value_1, "ABCDEFGHI"), 0);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_004
 * @tc.desc: Normal testCase of NDK Predicates for GreaterThanOrEqualTo
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_004, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_GreaterThanOrEqualTo(predicates, "data5", "ABCDEFG");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 3);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_005
 * @tc.desc: Normal testCase of NDK Predicates for LessThan
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_005, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_LessThan(predicates, "data5", "ABCDEFG");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 0);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_006
 * @tc.desc: Normal testCase of NDK Predicates for LessThanOrEqualTo
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_006, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_LessThanOrEqualTo(predicates, "data5", "ABCDEFG");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 1);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_007
 * @tc.desc: Normal testCase of NDK Predicates for IsNull.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_007, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_IsNull(predicates, "data4");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 2);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_008
 * @tc.desc: Normal testCase of NDK Predicates for IsNull.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_008, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_IsNotNull(predicates, "data4");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 1);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_009
 * @tc.desc: Normal testCase of NDK Predicates for Between
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_009, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_Between(predicates, "data2", "12000", "13000");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 1);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_010
 * @tc.desc: Normal testCase of NDK Predicates for NotBetween
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_010, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_NotBetween(predicates, "data2", "12000", "13000");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 2);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_011
 * @tc.desc: Normal testCase of NDK Predicates for OrderBy、Limit、Offset.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_011, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_OrderBy(predicates, "data2", OrderByType::ASC);
    predicates->OH_Predicates_Limit(predicates, 1);
    predicates->OH_Predicates_Offset(predicates, 1);
    predicates->OH_Predicates_Distinct(predicates);

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 1);

    errCode = cursor->OH_Cursor_GoToNextRow(cursor);
    int columnIndex;
    cursor->OH_Cursor_GetColumnIndex(cursor, "data2", &columnIndex);
    EXPECT_EQ(columnIndex, 2);
    int64_t longValue;
    cursor->OH_Cursor_GetInt64(cursor, columnIndex, &longValue);
    EXPECT_EQ(longValue, 13800);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_012
 * @tc.desc: Normal testCase of NDK Predicates for In.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_012, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    const char *names[] = {"zhangSan", "liSi"};
    int len = sizeof(names) / sizeof(names[0]);
    predicates->OH_Predicates_In(predicates, "data1", names, len);

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 2);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_013
 * @tc.desc: Normal testCase of NDK Predicates for NotIn.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_013, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    const char *names[] = {"zhangSan", "liSi"};
    int len = sizeof(names) / sizeof(names[0]);
    predicates->OH_Predicates_NotIn(predicates, "data1", names, len);

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 1);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_014
 * @tc.desc: Normal testCase of NDK Predicates for Like.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_014, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_Like(predicates, "data5", "ABCD%");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 3);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_015
 * @tc.desc: Normal testCase of NDK Predicates for GroupBy.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_015, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    const char *columnNames[] = {"data1", "data2"};
    int len = sizeof(columnNames) / sizeof(columnNames[0]);
    predicates->OH_Predicates_GroupBy(predicates, columnNames, len);

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, nullptr);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 3);
    cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_016
 * @tc.desc: Normal testCase of NDK Predicates for And.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_016, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_EqualTo(predicates, "data1", "zhangSan");
    predicates->OH_Predicates_And(predicates);
    predicates->OH_Predicates_EqualTo(predicates, "data3", "100.1");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 1);
    errCode = cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_017
 * @tc.desc: Normal testCase of NDK Predicates for Clear.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_017, TestSize.Level1)
{
    int errCode = 0;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates("test");
    predicates->OH_Predicates_EqualTo(predicates, "data1", "zhangSan");

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    int rowCount = 0;
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 1);
    errCode = cursor->OH_Cursor_Close(cursor);

    predicates->OH_Predicates_Clear(predicates);
    predicates->OH_Predicates_NotEqualTo(predicates, "data1", "zhangSan");
    cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_NE(cursor, NULL);
    errCode = cursor->OH_Cursor_GetRowCount(cursor, &rowCount);
    EXPECT_EQ(rowCount, 2);
    errCode = cursor->OH_Cursor_Close(cursor);
}

/**
 * @tc.name: RDB_NDK_predicates_test_018
 * @tc.desc: Normal testCase of NDK Predicates for table name is NULL.
 * @tc.type: FUNC
 */
HWTEST_F(RdbNdkPredicatesTest, RDB_NDK_predicates_test_018, TestSize.Level1)
{
    char *table = NULL;
    OH_Predicates *predicates = OH_Rdb_CreatePredicates(table);
    EXPECT_EQ(predicates, NULL);

    OH_Cursor *cursor = OH_Rdb_Query(predicatesTestRdbStore_, predicates, NULL, 0);
    EXPECT_EQ(cursor, NULL);
}