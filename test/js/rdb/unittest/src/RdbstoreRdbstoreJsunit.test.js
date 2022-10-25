/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import dataRdb from '@ohos.data.rdb';

const TAG = "[RDB_JSKITS_TEST]"
const CREATE_TABLE_TEST = "CREATE TABLE IF NOT EXISTS test (" + "id INTEGER PRIMARY KEY AUTOINCREMENT, " + "name TEXT NOT NULL, " + "age INTEGER, " + "salary REAL, " + "blobType BLOB)";

const STORE_CONFIG = {
    name: "rdbstore.db",
}
describe('rdbStoreTest', function () {
    beforeAll(async function () {
        console.info(TAG + 'beforeAll')
    })

    beforeEach(function () {
        console.info(TAG + 'beforeEach')
    })

    afterEach(function () {
        console.info(TAG + 'afterEach')
    })

    afterAll(async function () {
        console.info(TAG + 'afterAll')
    })

    console.log(TAG + "*************Unit Test Begin*************");

    /**
     * @tc.name rdb store getRdbStore test
     * @tc.number SUB_DDM_AppDataFWK_JSRDB_RdbStore_0010
     * @tc.desc rdb store getRdbStore test
     */
    it('testRdbStore0001', 0, async function (done) {
        console.log(TAG + "************* testRdbStore0001 start *************");
        let storePromise = dataRdb.getRdbStore(STORE_CONFIG, 1);
        storePromise.then(async (store) => {
            try {
                await console.log(TAG + "getRdbStore done: " + store);
            } catch (e) {
                expect(null).assertFail();
            }
        }).catch((err) => {
            expect(null).assertFail();
        })
        await storePromise
        storePromise = null
        await dataRdb.deleteRdbStore("rdbstore.db");
        done();
        console.log(TAG + "************* testRdbStore0001 end   *************");
    })

    /**
     * @tc.name rdb store getRdbStore and create table
     * @tc.number SUB_DDM_AppDataFWK_JSRDB_RdbStore_0020
     * @tc.desc rdb store getRdbStore and create table
     */
    it('testRdbStore0002', 0, async function (done) {
        console.log(TAG + "************* testRdbStore0002 start *************");
        let storePromise = dataRdb.getRdbStore(STORE_CONFIG, 2);
        storePromise.then(async (store) => {
            try {
                await console.log(TAG + "getRdbStore done: " + store);
                await store.executeSql(CREATE_TABLE_TEST);
            } catch (e) {
                expect(null).assertFail();
            }
        }).catch((err) => {
            expect(null).assertFail();
        })
        await storePromise
        storePromise = null
        await dataRdb.deleteRdbStore("rdbstore.db");
        done();
        console.log(TAG + "************* testRdbStore0002 end   *************");
    })

    /**
     * @tc.name rdb storegetRdbStore with wrong path
     * @tc.number SUB_DDM_AppDataFWK_JSRDB_RdbStore_0030
     * @tc.desc rdb store getRdbStore with wrong path
     */
    it('testRdbStore0003', 0, async function (done) {
        console.log(TAG + "************* testRdbStore0003 start *************");

        let storeConfig = {
            name: "/wrong/rdbstore.db",
        }
        let storePromise = dataRdb.getRdbStore(storeConfig, 4);
        storePromise.then(async (ret) => {
            await console.log(TAG + "getRdbStore done" + ret);
            expect(null).assertFail();
        }).catch((err) => {
            console.log(TAG + "getRdbStore with wrong path");
        })
        storePromise = null
        done();
        console.log(TAG + "************* testRdbStore0003 end   *************");
    })

    /**
     * @tc.name rdb store deleteRdbStore
     * @tc.number SUB_DDM_AppDataFWK_JSRDB_RdbStore_0040
     * @tc.desc rdb store deleteRdbStore
     */
    it('testRdbStore0004', 0, async function (done) {
        console.log(TAG + "************* testRdbStore0004 start *************");

        let storePromise = dataRdb.getRdbStore(STORE_CONFIG, 5);
        storePromise.then(async (store) => {
            try {
                await store.executeSql(CREATE_TABLE_TEST);
            } catch (e) {
                expect(null).assertFail();
            }
        }).catch((err) => {
            expect(null).assertFail();
        })
        await storePromise
        storePromise = null
        await dataRdb.deleteRdbStore("rdbstore.db");
        done();
        console.log(TAG + "************* testRdbStore0004 end   *************");
    })

    /**
     * @tc.name rdb store setVersion & getVersion
     * @tc.number SUB_DDM_AppDataFWK_JSRDB_RdbStore_0050
     * @tc.desc rdb store setVersion & getVersion
     */
    it('testRdbStore0005', 0, async function (done) {
        console.log(TAG + "************* testRdbStore0005 start *************");

        let storePromise = dataRdb.getRdbStore(STORE_CONFIG, 2);
        storePromise.then(async (store) => {
            try {
                expect(2).assertEqual(store.getVersion())
                store.setVersion(5)
                expect(5).assertEqual(store.getVersion())
                store.setVersion(2147483647)
                expect(2147483647).assertEqual(store.getVersion())
                store.setVersion(-2147483648)
                expect(-2147483648).assertEqual(store.getVersion())

                // Exceeds the range of 32-bit signed integer value.
                store.setVersion(2147483647000)
                expect(-1000).assertEqual(store.getVersion())
                store.setVersion(-2147483648100)
                expect(-100).assertEqual(store.getVersion())
            } catch (e) {
                expect(null).assertFail();
            }
        }).catch((err) => {
            expect(null).assertFail();
        })
        await storePromise
        storePromise = null
        await dataRdb.deleteRdbStore("rdbstore.db");
        done();
        console.log(TAG + "************* testRdbStore0005 end   *************");
    })

    /**
     * @tc.name rdb store getRdbStoreV9 with securityLevel
     * @tc.number SUB_DDM_AppDataFWK_JSRDB_RdbStore_0060
     * @tc.desc rdb store getRdbStoreV9 with securityLevel
     * @tc.require: I5PIL6
     */
    it('testRdbStore0006', 0, async function (done) {
        console.log(TAG + "************* testRdbStore0006 start *************");
        let config = {
            name: "secure.db",
            securityLevel: dataRdb.SecurityLevel.S3
        }
        let storePromise = dataRdb.getRdbStoreV9(config, 1);
        storePromise.then(async (store) => {
            try {
                await store.executeSql(CREATE_TABLE_TEST);
            } catch (e) {
                expect(null).assertFail();
            }
        }).catch((err) => {
            expect(null).assertFail();
        })
        await storePromise
        storePromise = null
        await dataRdb.deleteRdbStore("secure.db");
        done();
        console.log(TAG + "************* testRdbStore0006 end   *************");
    })

    /**
     * @tc.name rdb store getRdbStoreV9 with invalid securityLevel
     * @tc.number SUB_DDM_AppDataFWK_JSRDB_RdbStore_0070
     * @tc.desc rdb store getRdbStoreV9 with invalid securityLevel
     * @tc.require: I5PIL6
     */
    it('testRdbStore0007', 0, async function (done) {
        console.log(TAG + "************* testRdbStore0007 start *************");
        let config = {
            name: "secure.db",
            securityLevel: 8
        }
        let storePromise = dataRdb.getRdbStoreV9(config, 1);
        storePromise.then(async (ret) => {
            expect(null).assertFail();
        }).catch((err) => {
            console.log(TAG + "getRdbStoreV9 with invalid securityLevel");
        })
        storePromise = null
        done();
        console.log(TAG + "************* testRdbStore0007 end   *************");
    })
    console.log(TAG + "*************Unit Test End*************");
})