/*
 * Copyright (c) 2023 Noah Orensa.
 * Licensed under the MIT license. See LICENSE file in the project root for details.
*/

#pragma once

#include <hash_map.h>
#include <vector>
#include <serialization.h>


using namespace spl;

class PasswordStore
:   public Serializable {

protected:

    std::string _passphrase;
    HashMap<std::string, std::string> _passwords;

public:

    PasswordStore(const std::string &passphrase)
    :   _passphrase(passphrase)
    { }

    void writeObject(OutputStreamSerializer &serializer) const override;

    void readObject(InputStreamSerializer &serializer) override;

    PasswordStore & put(const std::string &name, const std::string &key) {
        _passwords.put(name, key);
        return *this;
    }

    bool remove(const std::string &name) {
        return _passwords.erase(name);
    }

    const std::string & get(const std::string &name) const {
        return _passwords[name];
    }

    std::vector<std::string> list() const;
};
