/*
 * Copyright (c) 2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <password_store.h>
#include <libcryptopp/default.h>
#include <libcryptopp/filters.h>
#include <libcryptopp/hex.h>
#include <std_serialization.h>
#include <json.h>
#include <error.h>
#include <algorithm>
#include <functional>

using DataParameters = CryptoPP::DataParametersInfo<
    CryptoPP::AES::BLOCKSIZE,
    CryptoPP::AES::DEFAULT_KEYLENGTH,
    CryptoPP::SHA256::DIGESTSIZE,
    8,
    2500
>;

using Encryptor = CryptoPP::DataEncryptorWithMAC<
    CryptoPP::AES,
    CryptoPP::SHA256,
    CryptoPP::HMAC<CryptoPP::SHA256>,
    DataParameters
>;

using Decryptor = CryptoPP::DataDecryptorWithMAC<
    CryptoPP::AES,
    CryptoPP::SHA256,
    CryptoPP::HMAC<CryptoPP::SHA256>,
    DataParameters
>;

static const uint64_t MAGIC = 0x5555555555551234;

static const uint32_t VERSION = 1;

static const std::function<HashMap<std::string, HashMap<std::string, std::string>>(InputStreamSerializer &, const char *)> reader[] = {
    // 0
    [] (InputStreamSerializer &serializer, const char *passphrase) -> HashMap<std::string, HashMap<std::string, std::string>> {
        std::string encrypted, decrypted;

        serializer >> encrypted;

        try {
            CryptoPP::StringSource ss2(encrypted, true,
                new CryptoPP::HexDecoder(
                    new Decryptor(
                        passphrase,
                        new CryptoPP::StringSink(decrypted)
                    )
                )
            );
        }
        catch (const CryptoPP::DataDecryptorErr &e) {
            throw Error("Invalid password");
        }
        catch (...) {
            throw RuntimeError("Unexpected exception occurred");
        }

        auto m = JSON::decode<HashMap<std::string, std::string>>(decrypted);

        return m.map<HashMap<std::string, HashMap<std::string, std::string>>>([] (const MapNode<std::string, std::string> &n) {
            return MapNode<std::string, HashMap<std::string, std::string>>(
                n.k,
                HashMap<std::string, std::string>({ {"default", n.v} })
            );
        });
    },

    // 1
    [] (InputStreamSerializer &serializer, const char *passphrase) -> HashMap<std::string, HashMap<std::string, std::string>> {
        std::string encrypted, decrypted;

        serializer >> encrypted;

        try {
            CryptoPP::StringSource ss2(encrypted, true,
                new CryptoPP::HexDecoder(
                    new Decryptor(
                        passphrase,
                        new CryptoPP::StringSink(decrypted)
                    )
                )
            );
        }
        catch (const CryptoPP::DataDecryptorErr &e) {
            throw Error("Invalid password");
        }
        catch (...) {
            throw RuntimeError("Unexpected exception occurred");
        }

        return JSON::decode<HashMap<std::string, HashMap<std::string, std::string>>>(decrypted);
    }
};

void PasswordStore::writeObject(OutputStreamSerializer &serializer) const {
    std::string encrypted;

    CryptoPP::StringSource ss1(JSON::encode(_passwords), true,
        new Encryptor(
            _passphrase.c_str(),
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(encrypted)
            )
        )
    );

    serializer << MAGIC << VERSION << encrypted;
}

void PasswordStore::readObject(InputStreamSerializer &serializer) {
    uint64_t magic;
    uint32_t version;

    if (! serializer.peek(&magic, sizeof(magic))) {
        throw RuntimeError("An unexpected error occurred while attempting to read password file");
    }

    if (magic == MAGIC) {
        serializer >> magic >> version;
        _passwords = reader[version](serializer, _passphrase.c_str());
    }
    else {
        _passwords = reader[0](serializer, _passphrase.c_str());
    }
}

std::vector<std::string> PasswordStore::list() const {
    auto l =  _passwords.map<List<std::string>>([] (const MapNode<std::string, HashMap<std::string, std::string>> &p) {
        return p.k;
    });

    std::vector<std::string> v(l.begin(), l.end());
    std::sort(v.begin(), v.end());
    return v;
}
