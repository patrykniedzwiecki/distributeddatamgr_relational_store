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

#ifndef NATIVE_RDB_ABS_RESULT_SET_H
#define NATIVE_RDB_ABS_RESULT_SET_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>

#include "result_set.h"
#include "value_object.h"

namespace OHOS {
namespace NativeRdb {
/**
 * The AbsResultSet class of RDB.
 * Provides methods for accessing a database result set generated by querying the database.
 */
class API_EXPORT AbsResultSet : public ResultSet {
public:
    /**
     * @brief Constructor.
     */
    API_EXPORT AbsResultSet();

    /**
     * @brief Destructor.
     */
    API_EXPORT virtual ~AbsResultSet();

    /**
     * @brief Obtains the number of rows in the result set.
     */
    API_EXPORT int GetRowCount(int &count) override;

    /**
     * @brief Obtains the names of all columns in a result set.
     */
    API_EXPORT int GetAllColumnNames(std::vector<std::string> &columnNames) override;

    /**
     * @brief Obtains the value of the specified column in the current row as a byte array.
     *
     * The implementation class determines whether to throw an exception if the value of the specified column
     * in the current row is null or the specified column is not of the Blob type.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns the value of the specified column as a byte array.
     */
    API_EXPORT int GetBlob(int columnIndex, std::vector<uint8_t> &blob) override;

    /**
     * @brief Obtains the value of the specified column in the current row as string.
     *
     * The implementation class determines whether to throw an exception if the value of the specified column
     * in the current row is null or the specified column is not of the string type.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns the value of the specified column as a string.
     */
    API_EXPORT int GetString(int columnIndex, std::string &value) override;

    /**
     * @brief Obtains the value of the specified column in the current row as int.
     *
     * The implementation class determines whether to throw an exception if the value of the specified column
     * in the current row is null or the specified column is not of the integer type.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns the value of the specified column as a int.
     */
    API_EXPORT int GetInt(int columnIndex, int &value) override;

    /**
     * @brief Obtains the value of the specified column in the current row as long.
     *
     * The implementation class determines whether to throw an exception if the value of the specified column
     * in the current row is null or the specified column is not of the long type.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns the value of the specified column as a long.
     */
    API_EXPORT int GetLong(int columnIndex, int64_t &value) override;

    /**
     * @brief Obtains the value of the specified column in the current row as double.
     *
     * The implementation class determines whether to throw an exception if the value of the specified column
     * in the current row is null or the specified column is not of the double type.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns the value of the specified column as a double.
     */
    API_EXPORT int GetDouble(int columnIndex, double &value) override;

    /**
     * @brief Obtains the value of the specified column in the current row as asset.
     *
     * The implementation class determines whether to throw an exception if the value of the specified column
     * in the current row is null or the specified column is not of the Asset type.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns the value of the specified column as a double.
     */
    API_EXPORT int GetAsset(int32_t col, ValueObject::Asset &value) override;

    /**
     * @brief Obtains the value of the specified column in the current row as assets.
     *
     * The implementation class determines whether to throw an exception if the value of the specified column
     * in the current row is null or the specified column is not of the Assets type.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns the value of the specified column as a double.
     */
    API_EXPORT int GetAssets(int32_t col, ValueObject::Assets &value) override;

    /**
     * @brief Obtains the value of the specified column in the current row as ValueObject.
     *
     * The implementation class determines whether to throw an exception if the value of the specified column
     * in the current row is null or the specified column is not of the double type.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns the value of the specified column as a double.
     */
    API_EXPORT int Get(int32_t col, ValueObject &value) override;

    /**
     * @brief Get the modify time of the cloud data.
     *
     * @param modifyTime Indicates the data modify utc time.
     *
     * @return Returns true if the value of the specified column in the current row is null;
     * returns false otherwise.
     */
    API_EXPORT int GetModifyTime(std::string &modifyTime) override;

    /**
     * @brief Checks whether the value of the specified column in the current row is null.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns true if the value of the specified column in the current row is null;
     * returns false otherwise.
     */
    API_EXPORT int IsColumnNull(int columnIndex, bool &isNull) override;

    /**
     * @brief Gets the entire row of data for the current row from the result set.
     */
    API_EXPORT int GetRow(RowEntity &rowEntity) override;

    /**
     * @brief Move the cursor to an absolute position.
     *
     * @param position Indicates the specified column index, which starts from 0.
     *
     * @return Returns whether the requested move succeeded.
     */
    API_EXPORT int GoToRow(int position) override;

    /**
     * @brief Obtains data type of the given column's value.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns column value type.
     */
    API_EXPORT int GetColumnType(int columnIndex, ColumnType &columnType) override;

    /**
     * @brief Returns the current position of the cursor in the result set.
     *
     * The value is zero-based. When the result set is first returned the cursor
     * will be at position -1, which is before the first row.
     * After the last row is returned another call to next() will leave the cursor past
     * the last entry, at a position of count().
     *
     * @return Returns the current cursor position.
     */
    API_EXPORT int GetRowIndex(int &position) const override;

    /**
     * @brief Go to the specified row of the result set forwards or backwards by an offset
     * relative to its current position.
     *
     * A positive offset indicates moving backwards, and a negative offset indicates moving forwards.
     *
     * @param offset Indicates the offset relative to the current position.
     *
     * @return Returns whether true if the result set is moved successfully and does not go beyond the range;
     * returns false otherwise.
     */
    API_EXPORT int GoTo(int offset) override;

    /**
     * @brief Go to the first row of the result set.
     *
     * @return Returns if the result set is moved successfully;
     * returns false otherwise, for example, if the result set is empty.
     */
    API_EXPORT int GoToFirstRow() override;

    /**
     * @brief Go to the last row of the result set.
     *
     * @return Returns if the result set is moved successfully;
     * returns false otherwise, for example, if the result set is empty.
     */
    API_EXPORT int GoToLastRow() override;

    /**
     * @brief Go to the next row of the result set.
     *
     * @return Returns if the result set is moved successfully;
     * returns false otherwise, for example, if the result set is already in the last row.
     */
    API_EXPORT int GoToNextRow() override;

    /**
     * @brief Go to the previous row of the result set.
     *
     * @return Returns if the result set is moved successfully;
     * returns false otherwise, for example, if the result set is already in the first row.
     */
    API_EXPORT int GoToPreviousRow() override;

    /**
     * @brief Checks whether the result set is positioned at the first row.
     */
    API_EXPORT int IsAtFirstRow(bool &result) const override;

    /**
     * @brief Checks whether the result set is positioned at the last row.
     */
    API_EXPORT int IsAtLastRow(bool &result) override;

    /**
     * @brief Returns whether the cursor is pointing to the position before the first row.
     */
    API_EXPORT int IsStarted(bool &result) const override;

    /**
     * @brief Checks whether the result set is positioned after the last row.
     */
    API_EXPORT int IsEnded(bool &result) override;

    /**
     * @brief Obtains the number of columns in the result set.
     */
    API_EXPORT int GetColumnCount(int &count) override;

    /**
     * @brief Returns the zero-based index for the given column name.
     *
     * @param columnName Indicates the specified name of the column.
     *
     * @return Returns the column index for the given column, or -1 if the column does not exist.
     */
    API_EXPORT int GetColumnIndex(const std::string &columnName, int &columnIndex) override;

    /**
     * @brief Returns the column name at the given column index.
     *
     * @param columnIndex Indicates the specified column index, which starts from 0.
     *
     * @return Returns the column name for the given index.
     */
    API_EXPORT int GetColumnName(int columnIndex, std::string &columnName) override;

    /**
     * @brief Checks whether the current result set is closed.
     *
     * @return Returns the true if the result set is closed by calling the close method.
     */
    API_EXPORT bool IsClosed() const override;

    /**
     * @brief Closes the result set.
     *
     * Calling this method on the result set will release all of its resources and makes it ineffective.
     */
    API_EXPORT int Close() override;

protected:
    std::map<std::string , int> columnMap_;
    int columnCount_ = -1;
    // The default position of the result set
    static const int INIT_POS = -1;
    /*
     * The value can be in the range [-1 ~ n], where -1 represents the start flag position and N represents the data end
     * flag position, and [0, n-1] represents the real data index.
     */
    int rowPos_;

    // Indicates whether the result set is closed
    bool isClosed;
    std::mutex columnMapLock_;
};
} // namespace NativeRdb
} // namespace OHOS

#endif