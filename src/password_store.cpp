/*
 * Copyright (c) 2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#include <password_store.h>
#include <cryptopp/default.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <std_serialization.h>
#include <json.h>
#include <error.h>
#include <algorithm>

void PasswordStore::writeObject(OutputStreamSerializer &serializer) const {
    std::string encrypted;

    CryptoPP::StringSource ss1(JSON::encode(_passwords), true,
        new CryptoPP::DefaultEncryptorWithMAC(
            _passphrase.c_str(),
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(encrypted)
            )
        )
    );

    serializer << encrypted;
}

void PasswordStore::readObject(InputStreamSerializer &serializer) {
    std::string encrypted, decrypted;

    serializer >> encrypted;

    try {
        CryptoPP::StringSource ss2(encrypted, true,
            new CryptoPP::HexDecoder(
                new CryptoPP::DefaultDecryptorWithMAC(
                    _passphrase.c_str(),
                    new CryptoPP::StringSink(decrypted)
                )
            )
        );
    }
    catch (const CryptoPP::KeyBadErr &e) {
        throw Error("Invalid password");
    }
    catch (...) {
        throw RuntimeError("Unexpected exception occurred");
    }

    _passwords = JSON::decode<HashMap<std::string, std::string>>(decrypted);
}

std::vector<std::string> PasswordStore::list() const {
    auto l =  _passwords.map<List<std::string>>([] (const MapNode<std::string, std::string> &p) {
        return p.k;
    });

    std::vector<std::string> v(l.begin(), l.end());
    std::sort(v.begin(), v.end());
    return v;
}
