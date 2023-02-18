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
    HashMap<std::string, HashMap<std::string, std::string>> _passwords;

public:

    PasswordStore(const std::string &passphrase)
    :   _passphrase(passphrase)
    { }

    void writeObject(OutputStreamSerializer &serializer) const override;

    void readObject(InputStreamSerializer &serializer) override;

    HashMap<std::string, HashMap<std::string, std::string>> & passwords() {
        return _passwords;
    }

    const HashMap<std::string, HashMap<std::string, std::string>> & passwords() const {
        return _passwords;
    }

    std::vector<std::string> list() const;
};
