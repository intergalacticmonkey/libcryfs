#include "CryPresetPasswordBasedKeyProvider.h"

using cpputils::unique_ref;
using cpputils::EncryptionKey;
using cpputils::unique_ref;
using cpputils::PasswordBasedKDF;
using cpputils::Data;

namespace cryfs {

CryPresetPasswordBasedKeyProvider::CryPresetPasswordBasedKeyProvider(std::string password, unique_ref<PasswordBasedKDF> kdf, SizedData* returnedHash)
: _password(std::move(password)), _kdf(std::move(kdf)), _returnedHash(returnedHash) {}

void CryPresetPasswordBasedKeyProvider::saveEncryptionKey(EncryptionKey encryptionKey) {
    if (_returnedHash != nullptr) {
        _returnedHash->size = encryptionKey.binaryLength();
        _returnedHash->data = new unsigned char[_returnedHash->size];
        memcpy(_returnedHash->data, encryptionKey.data(), _returnedHash->size);
    }
}

EncryptionKey CryPresetPasswordBasedKeyProvider::requestKeyForExistingFilesystem(size_t keySize, const Data& kdfParameters) {
    EncryptionKey encryptionKey = _kdf->deriveExistingKey(keySize, _password, kdfParameters);
    saveEncryptionKey(encryptionKey);
    return encryptionKey;
}

CryPresetPasswordBasedKeyProvider::KeyResult CryPresetPasswordBasedKeyProvider::requestKeyForNewFilesystem(size_t keySize) {
    auto keyResult = _kdf->deriveNewKey(keySize, _password);
    saveEncryptionKey(keyResult.key);
    return {std::move(keyResult.key), std::move(keyResult.kdfParameters)};
}

}
