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

#include "rdb_security_manager.h"

#include <securec.h>

#include <string>
#include <utility>

#include "directory_ex.h"
#include "file_ex.h"
#include "hks_param.h"
#include "logger.h"
#include "sqlite_database_utils.h"
#include "sqlite_utils.h"

#define HKS_FREE(PTR) \
{ \
    if ((PTR) != nullptr) { \
        free(PTR); \
        (PTR) = nullptr; \
    } \
}

namespace OHOS {
namespace NativeRdb {
using namespace OHOS::Rdb;

RdbPassword::RdbPassword() = default;

RdbPassword::~RdbPassword()
{
    (void)Clear();
}

bool RdbPassword::operator==(const RdbPassword &input) const
{
    if (size_ != input.GetSize()) {
        return false;
    }
    return memcmp(data_, input.GetData(), size_) == 0;
}

bool RdbPassword::operator!=(const RdbPassword &input) const
{
    return !(*this == input);
}

size_t RdbPassword::GetSize() const
{
    return size_;
}

const uint8_t *RdbPassword::GetData() const
{
    return data_;
}

int RdbPassword::SetValue(const uint8_t *inputData, size_t inputSize)
{
    if (inputSize > MAX_PASSWORD_SIZE) {
        return E_ERROR;
    }
    if (inputSize != 0 && inputData == nullptr) {
        return E_ERROR;
    }

    if (inputSize != 0) {
        std::copy(inputData, inputData + inputSize, data_);
    }

    size_t filledSize = std::min(size_, MAX_PASSWORD_SIZE);
    if (inputSize < filledSize) {
        std::fill(data_ + inputSize, data_ + filledSize, UCHAR_MAX);
    }

    size_ = inputSize;
    return E_OK;
}

int RdbPassword::Clear()
{
    return SetValue(nullptr, 0);
}

bool RdbPassword::IsValid() const
{
    return size_ != 0;
}

int32_t RdbSecurityManager::MallocAndCheckBlobData(struct HksBlob *blob, const uint32_t blobSize)
{
    blob->data = (uint8_t *)malloc(blobSize);
    if (blob->data == NULL) {
        LOG_ERROR("Blob data is NULL.");
        return HKS_FAILURE;
    }
    return HKS_SUCCESS;
}

int32_t RdbSecurityManager::HksLoopUpdate(const struct HksBlob *handle, const struct HksParamSet *paramSet,
    const struct HksBlob *inData, struct HksBlob *outData)
{
    struct HksBlob inDataSeg = *inData;
    uint8_t *lastPtr = inData->data + inData->size - 1;
    struct HksBlob outDataSeg = { MAX_OUTDATA_SIZE, NULL };
    uint8_t *cur = outData->data;
    outData->size = 0;

    inDataSeg.size = MAX_UPDATE_SIZE;

    while (inDataSeg.data <= lastPtr) {
        if (inDataSeg.data + MAX_UPDATE_SIZE <= lastPtr) {
            outDataSeg.size = MAX_OUTDATA_SIZE;
        } else {
            inDataSeg.size = lastPtr - inDataSeg.data + 1;
            break;
        }
        if (MallocAndCheckBlobData(&outDataSeg, outDataSeg.size) != HKS_SUCCESS) {
            LOG_ERROR("MallocAndCheckBlobData outDataSeg Failed.");
            return HKS_FAILURE;
        }
        if (HksUpdate(handle, paramSet, &inDataSeg, &outDataSeg) != HKS_SUCCESS) {
            LOG_ERROR("HksUpdate Failed.");
            HKS_FREE(outDataSeg.data);
            return HKS_FAILURE;
        }
        if (memcpy_s(cur, outDataSeg.size, outDataSeg.data, outDataSeg.size) != 0) {
            LOG_ERROR("Method memcpy_s failed");
            HKS_FREE(outDataSeg.data);
            return HKS_FAILURE;
        }
        cur += outDataSeg.size;
        outData->size += outDataSeg.size;
        HKS_FREE(outDataSeg.data);
        if (inDataSeg.data + MAX_UPDATE_SIZE > lastPtr) {
            LOG_ERROR("inDataSeg data Error");
            return HKS_FAILURE;
        }
        inDataSeg.data += MAX_UPDATE_SIZE;
    }

    struct HksBlob outDataFinish = { inDataSeg.size * TIMES, NULL };
    if (MallocAndCheckBlobData(&outDataFinish, outDataFinish.size) != HKS_SUCCESS) {
        LOG_ERROR("MallocAndCheckBlobData outDataFinish Failed.");
        return HKS_FAILURE;
    }
    if (HksFinish(handle, paramSet, &inDataSeg, &outDataFinish) != HKS_SUCCESS) {
        LOG_ERROR("HksFinish Failed.");
        HKS_FREE(outDataFinish.data);
        return HKS_FAILURE;
    }
    if (memcpy_s(cur, outDataFinish.size, outDataFinish.data, outDataFinish.size) != 0) {
        LOG_ERROR("Method memcpy_s failed");
        HKS_FREE(outDataFinish.data);
        return HKS_FAILURE;
    }
    outData->size += outDataFinish.size;
    HKS_FREE(outDataFinish.data);

    return HKS_SUCCESS;
}

int32_t RdbSecurityManager::HksEncryptThreeStage(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    const struct HksBlob *plainText, struct HksBlob *cipherText)
{
    uint8_t handle[sizeof(uint64_t)] = { 0 };
    struct HksBlob handleBlob = { sizeof(uint64_t), handle };
    int32_t result = HksInit(keyAlias, paramSet, &handleBlob, nullptr);
    if (result != HKS_SUCCESS) {
        LOG_ERROR("HksEncrypt failed with error %{public}d", result);
        return result;
    }
    return HksLoopUpdate(&handleBlob, paramSet, plainText, cipherText);
}

int32_t RdbSecurityManager::HksDecryptThreeStage(const struct HksBlob *keyAlias, const struct HksParamSet *paramSet,
    const struct HksBlob *cipherText, struct HksBlob *plainText)
{
    uint8_t handle[sizeof(uint64_t)] = { 0 };
    struct HksBlob handleBlob = { sizeof(uint64_t), handle };
    int32_t result = HksInit(keyAlias, paramSet, &handleBlob, nullptr);
    if (result != HKS_SUCCESS) {
        LOG_ERROR("HksEncrypt failed with error %{public}d", result);
        return result;
    }
    return HksLoopUpdate(&handleBlob, paramSet, cipherText, plainText);
}

RdbSecurityManager::RdbSecurityManager() = default;

RdbSecurityManager::~RdbSecurityManager() = default;

std::vector<uint8_t> RdbSecurityManager::GenerateRandomNum(int32_t len)
{
    std::random_device randomDevice;
    std::uniform_int_distribution<int> distribution(0, std::numeric_limits<uint8_t>::max());
    std::vector<uint8_t> key(len);
    for (int32_t i = 0; i < len; i++) {
        key[i] = static_cast<uint8_t>(distribution(randomDevice));
    }
    return key;
}

bool RdbSecurityManager::SaveSecretKeyToFile(RdbSecurityManager::KeyFileType keyFile)
{
    LOG_INFO("SaveSecretKeyToFile begin.");
    if (!CheckRootKeyExists()) {
        LOG_ERROR("Root key not exists!");
        return false;
    }
    std::vector<uint8_t> key = GenerateRandomNum(RDB_KEY_SIZE);
    RdbSecretKeyData keyData;
    keyData.timeValue = std::chrono::system_clock::to_time_t(std::chrono::system_clock::system_clock::now());
    keyData.distributed = 0;
    keyData.secretKey = EncryptWorkKey(key);

    if (keyData.secretKey.empty()) {
        LOG_ERROR("Key size is 0");
        key.assign(key.size(), 0);
        return false;
    }

    key.assign(key.size(), 0);
    if (!RdbSecurityManager::InitPath(dbKeyDir_)) {
        LOG_ERROR("InitPath err.");
        return false;
    }

    std::string keyPath;
    GetKeyPath(keyFile, keyPath);

    return SaveSecretKeyToDisk(keyPath, keyData);
}

bool RdbSecurityManager::SaveSecretKeyToDisk(const std::string &path, RdbSecretKeyData &keyData)
{
    LOG_INFO("SaveSecretKeyToDisk begin.");
    std::vector<uint8_t> distributedInByte = TransferTypeToByteArray<uint8_t>(keyData.distributed);
    std::vector<uint8_t> timeInByte = TransferTypeToByteArray<time_t>(keyData.timeValue);
    std::vector<char> secretKeyInChar;
    secretKeyInChar.insert(secretKeyInChar.end(), distributedInByte.begin(), distributedInByte.end());
    secretKeyInChar.insert(secretKeyInChar.end(), timeInByte.begin(), timeInByte.end());
    secretKeyInChar.insert(secretKeyInChar.end(), keyData.secretKey.begin(), keyData.secretKey.end());

    std::lock_guard<std::mutex> lock(mutex_);
    bool ret = SaveBufferToFile(path, secretKeyInChar);
    if (!ret) {
        LOG_ERROR("SaveBufferToFile failed!");
        return false;
    }

    return true;
}

int RdbSecurityManager::GenerateRootKey()
{
    LOG_INFO("RDB GenerateRootKey begin.");
    struct HksBlob rootKeyName = { uint32_t(rootKeyAlias_.size()), rootKeyAlias_.data() };
    struct HksParamSet *params = nullptr;
    int32_t ret = HksInitParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksInitParamSet()-client failed with error %{public}d", ret);
        return ret;
    }

    struct HksParam hksParam[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = 0 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
    };

    ret = HksAddParams(params, hksParam, sizeof(hksParam) / sizeof(hksParam[0]));
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksAddParams-client failed with error %{public}d", ret);
        HksFreeParamSet(&params);
        return ret;
    }

    ret = HksBuildParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksBuildParamSet-client failed with error %{public}d", ret);
        HksFreeParamSet(&params);
        return ret;
    }

    ret = HksGenerateKey(&rootKeyName, params, nullptr);
    HksFreeParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksGenerateKey-client failed with error %{public}d", ret);
    }
    LOG_INFO("RDB root key generated successful.");
    return ret;
}

std::vector<uint8_t> RdbSecurityManager::EncryptWorkKey(const std::vector<uint8_t> &key)
{
    struct HksBlob blobAad = { uint32_t(aad_.size()), aad_.data() };
    struct HksBlob blobNonce = { uint32_t(nonce_.size()), nonce_.data() };
    struct HksBlob rootKeyName = { uint32_t(rootKeyAlias_.size()), rootKeyAlias_.data() };
    struct HksBlob plainKey = { uint32_t(key.size()), const_cast<uint8_t *>(key.data()) };
    struct HksParamSet *params = nullptr;
    int32_t ret = HksInitParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksInitParamSet() failed with error %{public}d", ret);
        return {};
    }
    struct HksParam hksParam[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = 0 },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_NONCE, .blob = blobNonce },
        { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = blobAad },
    };
    ret = HksAddParams(params, hksParam, sizeof(hksParam) / sizeof(hksParam[0]));
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksAddParams failed with error %{public}d", ret);
        HksFreeParamSet(&params);
        return {};
    }

    ret = HksBuildParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksBuildParamSet failed with error %{public}d", ret);
        HksFreeParamSet(&params);
        return {};
    }

    uint8_t cipherBuf[256] = { 0 };
    struct HksBlob cipherText = { sizeof(cipherBuf), cipherBuf };
    ret = HksEncryptThreeStage(&rootKeyName, params, &plainKey, &cipherText);
    (void)HksFreeParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksEncrypt failed with error %{public}d", ret);
        return {};
    }

    std::vector<uint8_t> encryptedKey(cipherText.data, cipherText.data + cipherText.size);
    (void)memset_s(cipherBuf, sizeof(cipherBuf), 0, sizeof(cipherBuf));

    return encryptedKey;
}

bool RdbSecurityManager::DecryptWorkKey(std::vector<uint8_t> &source, std::vector<uint8_t> &key)
{
    uint8_t aead_[AEAD_LEN] = { 0 };
    struct HksBlob blobAad = { uint32_t(aad_.size()), &(aad_[0]) };
    struct HksBlob blobNonce = { uint32_t(nonce_.size()), &(nonce_[0]) };
    struct HksBlob rootKeyName = { uint32_t(rootKeyAlias_.size()), &(rootKeyAlias_[0]) };
    struct HksBlob encryptedKeyBlob = { uint32_t(source.size()), source.data() };
    struct HksBlob blobAead = { AEAD_LEN, aead_ };

    struct HksParamSet *params = nullptr;
    int32_t ret = HksInitParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksInitParamSet() failed with error %{public}d", ret);
        return false;
    }
    struct HksParam hksParam[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = 0 },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_NONCE, .blob = blobNonce },
        { .tag = HKS_TAG_ASSOCIATED_DATA, .blob = blobAad },
        { .tag = HKS_TAG_AE_TAG, .blob = blobAead },
    };
    ret = HksAddParams(params, hksParam, sizeof(hksParam) / sizeof(hksParam[0]));
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksAddParams failed with error %{public}d", ret);
        HksFreeParamSet(&params);
        return false;
    }

    ret = HksBuildParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksBuildParamSet failed with error %{public}d", ret);
        HksFreeParamSet(&params);
        return false;
    }

    encryptedKeyBlob.size -= AEAD_LEN;
    for (uint32_t i = 0; i < params->paramsCnt; i++) {
        if (params->params[i].tag == HKS_TAG_AE_TAG) {
            uint8_t *tempPtr = encryptedKeyBlob.data;
            if (memcpy_s(params->params[i].blob.data, AEAD_LEN, tempPtr + encryptedKeyBlob.size, AEAD_LEN) != 0) {
                LOG_ERROR("Method memcpy_s failed");
                HksFreeParamSet(&params);
                return false;
            }
            break;
        }
    }

    uint8_t plainBuf[256] = { 0 };
    struct HksBlob plainKeyBlob = { sizeof(plainBuf), plainBuf };
    ret = HksDecryptThreeStage(&rootKeyName, params, &encryptedKeyBlob, &plainKeyBlob);
    (void)HksFreeParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksDecrypt failed with error %{public}d", ret);
        return false;
    }

    key.assign(plainKeyBlob.data, plainKeyBlob.data + plainKeyBlob.size);
    (void)memset_s(plainBuf, sizeof(plainBuf), 0, sizeof(plainBuf));
    return true;
}

void RdbSecurityManager::Init(const std::string &bundleName, const std::string &path)
{
    ParsePath(path);
    bundleName_ = bundleName;
    rootKeyAlias_ = GenerateRootKeyAlias();
    nonce_ = std::vector<uint8_t>(RDB_HKS_BLOB_TYPE_NONCE, RDB_HKS_BLOB_TYPE_NONCE + strlen(RDB_HKS_BLOB_TYPE_NONCE));
    aad_ = std::vector<uint8_t>(RDB_HKS_BLOB_TYPE_AAD, RDB_HKS_BLOB_TYPE_AAD + strlen(RDB_HKS_BLOB_TYPE_AAD));

    if (CheckRootKeyExists()) {
        return;
    }
    constexpr uint32_t RETRY_MAX_TIMES = 5;
    uint32_t retryCount = 0;
    constexpr int RETRY_TIME_INTERVAL_MILLISECOND = 1 * 1000 * 1000;
    while (retryCount < RETRY_MAX_TIMES) {
        auto ret = GenerateRootKey();
        if (ret == HKS_SUCCESS) {
            break;
        }
        retryCount++;
        LOG_ERROR("RDB generate root key failed, try count:%{public}u", retryCount);
        usleep(RETRY_TIME_INTERVAL_MILLISECOND);
    }
}

bool RdbSecurityManager::CheckRootKeyExists()
{
    LOG_INFO("RDB checkRootKeyExist begin.");
    struct HksBlob rootKeyName = { uint32_t(rootKeyAlias_.size()), rootKeyAlias_.data() };
    struct HksParamSet *params = nullptr;
    int32_t ret = HksInitParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksInitParamSet()-client failed with error %{public}d", ret);
        return ret;
    }

    struct HksParam hksParam[] = {
        { .tag = HKS_TAG_ALGORITHM, .uint32Param = HKS_ALG_AES },
        { .tag = HKS_TAG_KEY_SIZE, .uint32Param = HKS_AES_KEY_SIZE_256 },
        { .tag = HKS_TAG_PURPOSE, .uint32Param = HKS_KEY_PURPOSE_ENCRYPT | HKS_KEY_PURPOSE_DECRYPT },
        { .tag = HKS_TAG_DIGEST, .uint32Param = 0 },
        { .tag = HKS_TAG_PADDING, .uint32Param = HKS_PADDING_NONE },
        { .tag = HKS_TAG_BLOCK_MODE, .uint32Param = HKS_MODE_GCM },
    };

    ret = HksAddParams(params, hksParam, sizeof(hksParam) / sizeof(hksParam[0]));
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksAddParams failed with error %{public}d", ret);
        HksFreeParamSet(&params);
        return ret;
    }

    ret = HksBuildParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksBuildParamSet failed with error %{public}d", ret);
        HksFreeParamSet(&params);
        return ret;
    }

    ret = HksKeyExist(&rootKeyName, params);
    HksFreeParamSet(&params);
    if (ret != HKS_SUCCESS) {
        LOG_ERROR("HksEncrypt failed with error %{public}d", ret);
    }
    return ret == HKS_SUCCESS;
}

bool RdbSecurityManager::InitPath(const std::string &path)
{
    constexpr mode_t DEFAULT_UMASK = 0002;
    if (access(path.c_str(), F_OK) == 0) {
        return true;
    }
    umask(DEFAULT_UMASK);
    if (mkdir(path.c_str(), (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) != 0 && errno != EEXIST) {
        LOG_ERROR("mkdir error:%{public}d, dbDir:%{public}s", errno, SqliteUtils::Anonymous(path).c_str());
        return false;
    }
    return true;
}

RdbPassword RdbSecurityManager::LoadSecretKeyFromFile(KeyFileType keyFile)
{
    std::string keyPath;
    GetKeyPath(keyFile, keyPath);
    if (!FileExists(keyPath)) {
        LOG_ERROR("Key file not exists.");
        return {};
    }

    RdbSecretKeyData keyData;
    if (!LoadSecretKeyFromDisk(keyPath, keyData)) {
        LOG_ERROR("Load key failed.");
        return {};
    }

    std::vector<uint8_t> key;
    if (!DecryptWorkKey(keyData.secretKey, key)) {
        LOG_ERROR("Decrypt key failed!");
        return {};
    }

    RdbPassword rdbPasswd;
    rdbPasswd.isKeyExpired = IsKeyExpired(keyData.timeValue);
    rdbPasswd.SetValue(key.data(), key.size());
    key.assign(key.size(), 0);
    return rdbPasswd;
}

bool RdbSecurityManager::LoadSecretKeyFromDisk(const std::string &keyPath, RdbSecretKeyData &keyData)
{
    LOG_INFO("LoadSecretKeyFromDisk begin.");
    std::vector<char> content;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!LoadBufferFromFile(keyPath, content) || content.empty()) {
            LOG_ERROR("LoadBufferFromFile failed!");
            return false;
        }
    }

    std::vector<uint8_t> distribute;
    auto iter = content.begin();
    distribute.push_back(*iter);
    iter++;
    uint8_t distributeStatus = TransferByteArrayToType<uint8_t>(distribute);

    std::vector<uint8_t> createTime;
    for (int i = 0; i < static_cast<int>(sizeof(time_t) / sizeof(uint8_t)); i++) {
        createTime.push_back(*iter);
        iter++;
    }

    keyData.distributed = distributeStatus;
    keyData.timeValue = TransferByteArrayToType<time_t>(createTime);
    keyData.secretKey.insert(keyData.secretKey.end(), iter, content.end());

    return true;
}

RdbPassword RdbSecurityManager::GetRdbPassword(KeyFileType keyFile)
{
    LOG_INFO("GetRdbPassword Begin.");
    if (!CheckKeyDataFileExists(keyFile)) {
        if (!SaveSecretKeyToFile(keyFile)) {
            LOG_ERROR("Failed to save key.");
            return {};
        }
    }

    return LoadSecretKeyFromFile(keyFile);
}

std::vector<uint8_t> RdbSecurityManager::GenerateRootKeyAlias()
{
    std::vector<uint8_t> rootKeyAlias =
        std::vector<uint8_t>(RDB_ROOT_KEY_ALIAS_PREFIX, RDB_ROOT_KEY_ALIAS_PREFIX + strlen(RDB_ROOT_KEY_ALIAS_PREFIX));
    if (!bundleName_.empty()) {
        rootKeyAlias.insert(rootKeyAlias.end(), bundleName_.begin(), bundleName_.end());
    } else {
        rootKeyAlias.insert(rootKeyAlias.end(), dbDir_.begin(), dbDir_.end());
    }

    return rootKeyAlias;
}

void RdbSecurityManager::DelRdbSecretDataFile(const std::string &path)
{
    LOG_INFO("Delete all key files begin.");
    std::lock_guard<std::mutex> lock(mutex_);
    ParsePath(path);
    SqliteDatabaseUtils::DeleteFile(keyPath_);
    SqliteDatabaseUtils::DeleteFile(newKeyPath_);
}

bool RdbSecurityManager::IsKeyExpired(const time_t &createTime)
{
    auto timePoint = std::chrono::system_clock::from_time_t(createTime);
    return ((timePoint + std::chrono::hours(HOURS_PER_YEAR)) < std::chrono::system_clock::now());
}

RdbSecurityManager &RdbSecurityManager::GetInstance()
{
    static RdbSecurityManager instance;
    return instance;
}

static std::string RemoveSuffix(const std::string &name)
{
    std::string suffix(".db");
    auto pos = name.rfind(suffix);
    if (pos == std::string::npos || pos < name.length() - suffix.length()) {
        return name;
    }
    return { name, 0, pos };
}

void RdbSecurityManager::ParsePath(const std::string &path)
{
    dbDir_ = ExtractFilePath(path);
    const std::string dbName = ExtractFileName(path);
    dbName_ = RemoveSuffix(dbName);
    dbKeyDir_ = dbDir_ + std::string("key/");
    keyPath_ = dbKeyDir_ + dbName_ + std::string(RdbSecurityManager::SUFFIX_PUB_KEY);
    newKeyPath_ = dbKeyDir_ + dbName_ + std::string(RdbSecurityManager::SUFFIX_PUB_KEY_NEW);
}

bool RdbSecurityManager::CheckKeyDataFileExists(RdbSecurityManager::KeyFileType fileType)
{
    if (fileType == KeyFileType::PUB_KEY_FILE) {
        return FileExists(keyPath_);
    } else {
        return FileExists(newKeyPath_);
    }
}

int RdbSecurityManager::GetKeyDistributedStatus(KeyFileType keyFile, bool &status)
{
    LOG_INFO("GetKeyDistributedStatus start.");
    std::string keyPath;
    GetKeyPath(keyFile, keyPath);

    RdbSecretKeyData keyData;
    if (!LoadSecretKeyFromDisk(keyPath, keyData)) {
        return E_ERROR;
    }

    status = (keyData.distributed == DISTRIBUTED);
    return E_OK;
}

int RdbSecurityManager::SetKeyDistributedStatus(KeyFileType keyFile, bool status)
{
    LOG_INFO("SetKeyDistributedStatus start.");
    std::string keyPath;
    GetKeyPath(keyFile, keyPath);
    RdbSecretKeyData keyData;
    if (!LoadSecretKeyFromDisk(keyPath, keyData)) {
        return E_ERROR;
    }

    keyData.distributed = (status ? DISTRIBUTED : UNDISTRIBUTED);
    if (!SaveSecretKeyToDisk(keyPath, keyData)) {
        return E_ERROR;
    }

    return E_OK;
}

void RdbSecurityManager::GetKeyPath(RdbSecurityManager::KeyFileType keyType, std::string &keyPath)
{
    if (keyType == KeyFileType::PUB_KEY_FILE) {
        keyPath = keyPath_;
    } else {
        keyPath = newKeyPath_;
    }
}

void RdbSecurityManager::DelRdbSecretDataFile(RdbSecurityManager::KeyFileType keyFile)
{
    std::string keyPath;
    GetKeyPath(keyFile, keyPath);
    SqliteDatabaseUtils::DeleteFile(keyPath);
}

void RdbSecurityManager::UpdateKeyFile()
{
    if (!SqliteDatabaseUtils::RenameFile(newKeyPath_, keyPath_)) {
        LOG_ERROR("Rename key file failed.");
        return;
    }
}
} // namespace NativeRdb
} // namespace OHOS
